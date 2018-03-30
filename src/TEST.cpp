#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <thread>
#include <tuple>
#include <vector>
#include <semaphore.h>
#include "AtomicQueue.h"
#include "concurrentqueue.h"

struct node { //used for each node of the graph
    std::vector<int> preds; //set of all predecessors
    std::vector<int> succs; //set of all successors
};

class task_queue { //a thread pool implementation - is necessary to get good performance
    std::vector<std::thread> workers; //actual worker threads
    std::atomic<bool> stopflag; //used to signal workers to stop
    sem_t cv_sem;
    int numthreads;
    std::function<void(int)> task1, task2;
    std::atomic<bool> is_task1;
    std::atomic_int barrier;

public:
    task_queue(int max_tasks, std::unordered_set<int> *active_workers, std::atomic<int> *finished, std::function<void(int)>& task1, std::function<void(int)>& task2, int numthreads=8) : stopflag(false) { //by default, we have 8 worker threads
        this->numthreads = numthreads;
        this->task1 = task1;
        this->task2 = task2;
        stopflag.store(false);
        sem_init(&cv_sem, 0, 0);
        barrier.store(numthreads);
        auto thread_task=[this, active_workers, finished](int tid) { //logic for thread to dequeue and execute tasks
            while(!stopflag.load()) {
                sem_wait(&cv_sem);
                if(!stopflag.load()) {
                    int step = (active_workers->size()/(this->numthreads+1));
                    int start_index = tid*step;
                    int end_index = (tid+1)*step;
                    auto it = std::next(active_workers->begin(), start_index);
                    if((this->is_task1).load()) {
                        for(int i=start_index; i<end_index; i++) {
                            this->task1(*it);
                            ++it;
                        }
                    } else {
                        for(int i=start_index; i<end_index; i++) {
                            this->task2(*it);
                            ++it;
                        }
                    }
                    this->barrier++;
                    while(this->barrier.load()!=this->numthreads);
                    (*finished)++;
                }
            }
        };
        for (int i=0;i!=numthreads;++i) { //create worker threads
            workers.emplace_back(thread_task, i);
        }
    }

    void restart(bool is_task1) {
        (this->is_task1).store(is_task1);
        while(barrier.load()!=numthreads);
        barrier.store(0);
        for (int i = 0; i < numthreads; ++i) {
            sem_post(&cv_sem);
        }
    }

    void stop() { //issue signal to stop workers, and join them
        stopflag.store(true);
        for (int i = 0; i < numthreads; ++i) {
            sem_post(&cv_sem);
        }
        for (auto iter = std::begin(workers);iter!= std::end(workers);++iter) {
            iter->join();
        }
    }

};

int main(int argc, char const *argv[]) {
    const char *file_name = argc>1 ? argv[1]: "graph.txt";

    std::ifstream reader(file_name); //graph reader
    int n; //number of nodes
    reader>>n;
    std::vector<node> graph;

    for (int i=0;i!=n;++i) { //construct the edge lists with empty initialization
        graph.emplace_back(node());
    }
    
    for (int i=0;i!=n;++i) {
        int ne; // number of edges
        reader>>ne;
        for(int j=0; j<ne; j++) {
            int local;
            reader>>local; //other end of edge
            if (local!=i+1) { //insert into the graph
                graph[i].succs.emplace_back(local-1);
                graph[local-1].preds.emplace_back(i);
            }
        }
    }

    int n_threads = argc>2? atoi(argv[2]): 8;
    int task_pool_step = argc>3? atoi(argv[3]):10000;

    bool empty;

    std::atomic<bool> changeflag(false); //used to track if any colors changed
    // std::map<int,std::unique_ptr<std::atomic<int>>> registers; //registers used for propagation
    std::unique_ptr<std::atomic<int>[]> registers; //registers used for propagation
    std::unique_ptr<std::atomic<bool>[]> changed_now, changed_prev; //registers used for propagation
    
    registers = std::make_unique<std::atomic<int>[]>(n);
    changed_now = std::make_unique<std::atomic<bool>[]>(n);
    changed_prev = std::make_unique<std::atomic<bool>[]>(n);
    for (auto i=0;i!=n;++i) { //zero-initialize the registers
        changed_now[i] = false;
        changed_prev[i] = true;
    }
    
    std::vector<std::vector<int>> sccs; //output list
    std::mutex out_lk; //used to control access to the output list

    std::unordered_set<int> active_workers; //nodes that are still in graph
    for (int i=1;i!=n+1;++i) { //initialize with all nodes
        active_workers.insert(i-1);
    }
    
    unsigned found_sccs=0;
    auto start_time = std::chrono::high_resolution_clock::now(); //start timing

    std::function<void(int)> task1 = 
    [&graph, &registers, &changed_prev, &changed_now, &changeflag](int node_num){ //one propagation from one node
        if(changed_prev[node_num].load()) {
            node& selfref=graph[node_num];
            int own_val=registers[node_num].load();
            for (int i : selfref.succs) {
                auto& succreg=registers[i]; //get reference to child's register
                
                while (true) {
                    auto val = succreg.load(); //read value from child's register
                    if (val<own_val) { //own color is greater than child's
                        auto res= succreg.compare_exchange_strong(val,own_val); //attempt propagation
                        if (res) { //succeeded in propagating
                            changed_now[i].store(true);
                            changeflag.store(true); //notify change
                            break;
                        } //else retry, as some other thread managed to alter register
                    }
                    else { //child's color is greater than own
                        break;
                    }
                }
            }
        }
    };
    
    std::function<void(int)> task2 =
    [&graph, &registers, n, &sccs, &out_lk](int node_num){ //initiate root check at all nodes
        node& selfref = graph[node_num];
        if (registers[node_num].load()==node_num) { //is a root of an SCC
            std::unordered_set<int> visited; //track nodes visited in reversed BFS
            std::queue<int> to_visit; //queue for BFS
            std::vector<int> scc; //list of nodes in the SCC
            visited.insert(node_num); //mark self as visited
            scc.push_back(node_num); //add self to SCC

            registers[node_num].store(n+1);
            
            for (int i : selfref.preds) {
                to_visit.push(i);
                visited.insert(i); //mark reverse children as visited
            }
            
            while (!to_visit.empty()) { //there are still nodes to visit
                int x = to_visit.front(); //remove node from queue
                to_visit.pop();
                if (registers[x].load()==node_num) { //if visited node has root's color
                    scc.push_back(x); //add to scc
                    registers[x].store(n+1);
                    for (int i : graph[x].preds) { //add its reverse children
                        if (visited.find(i)==std::end(visited)) { //if not already visited
                            visited.insert(i);
                            to_visit.push(i);
                        }
                    }
                }
            }
            
            { //add SCC to output list
                std::lock_guard<std::mutex> lock(out_lk);
                sccs.emplace_back(std::move(scc));
            }
        }
    }; 

    std::atomic<int> finished; //used like a barrier
    task_queue tq(n/task_pool_step, &active_workers, &finished, task1, task2,  n_threads-1);
    while (active_workers.size()) { //while graph is non-empty
        int aw_size = active_workers.size();
        // printf("main %d\n", aw_size);
        int tasks_created;
        for (auto i : active_workers) { //initialize registers with node's colors
            registers[i].store(i);
        }

        finished.store(0);
        do {
            // printf("\n\n");
            changeflag.store(false);
            finished.store(0);

            for (int i = 0; i < n; ++i) {
                changed_now[i].store(false);
            }
            if(aw_size>n_threads)
                tq.restart(true);

            int start_index = aw_size>n_threads? (n_threads-1)*(aw_size/n_threads): 0;
            auto it = std::next(active_workers.begin(), start_index);
            // printf("M %d %d\n", start_index, aw_size-1 );
            for(int i=start_index; i<aw_size; i++) {
                task1(*it);
                ++it;
            }
            finished++;

            if(aw_size>n_threads)
                while (finished.load()!=n_threads) {} //wait for all threads to complete the iteration
    
            std::swap(changed_now, changed_prev);

        } while(changeflag.load()); //until graph reaches stable state
    
        finished.store(0); //reset barrier
    
        if(aw_size>n_threads)
            tq.restart(false);
        
        int start_index = aw_size>n_threads? (n_threads-1)*(aw_size/n_threads): 0;
        auto it = std::next(active_workers.begin(), start_index);
        for(int i=start_index; i<aw_size; i++) {
            task2(*it);
            ++it;
        }
        finished++;

        if(aw_size>n_threads)
            while(finished.load()!=n_threads) {} //wait for all threads to finish
    
        finished.store(0); //reset barrier
        for (auto i=found_sccs;i!=sccs.size();++i) { //loop over new SCCs added
            for (auto elem : sccs[i]) {
                active_workers.erase(elem); //remove node from active ids
            }
        }
    
        found_sccs=sccs.size();
    }
    
    auto stop_time = std::chrono::high_resolution_clock::now(); //stop timing
    double micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(stop_time-start_time).count();
    std::cout<<"Time: "<< micro_sec/1e6 <<"\n";
    tq.stop(); //stop the thread pool's execution

    
    // for(int i=0; i<sccs.size(); i++) {
    //     sort(sccs[i].begin(), sccs[i].end());
    // }
    // sort(sccs.begin(), sccs.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
    //     return a[0] < b[0];
    // });
    // for (const auto& elem : sccs) { //print found sccs
    //     for (const auto& inner_elem : elem) {
    //         std::cout<<inner_elem<<" ";
    //     }
    //     std::cout<<"\n";
    // }
    
    std::cout << "Size: " <<  sccs.size() << std::endl;
}
