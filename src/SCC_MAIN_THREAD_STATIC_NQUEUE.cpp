/*
* Using semaphore in task pool instead of locks
* Wait free queue in task pool
* No deletion of nodes
*/
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <tuple>
#include <vector>
#include <semaphore.h>
#include "AtomicQueue.h"
#include "concurrentqueue.h"

struct node { //used for each node of the graph
    std::set<int> preds; //set of all predecessors
    std::set<int> succs; //set of all successors
};

class task_queue { //a thread pool implementation - is necessary to get good performance
    std::vector<std::thread> workers; //actual worker threads
    std::atomic<bool> stopflag; //used to signal workers to stop
    sem_t cv_sem;
    int numthreads;

public:
    moodycamel::ConcurrentQueue<std::function<void()>> *tasks; //queue of tasks to be executed
    
    task_queue(int max_tasks, int numthreads=8) : stopflag(false) { //by default, we have 8 worker threads
        tasks = new moodycamel::ConcurrentQueue<std::function<void()>>(max_tasks);
        this->numthreads = numthreads;
        sem_init(&cv_sem, 0, 0);
        auto thread_task=[this]() { //logic for thread to dequeue and execute tasks
            bool empty;
            while(true) {
                sem_wait(&cv_sem);
                if (stopflag.load()) { //stop signal
                    break;
                }
                else {
                    std::function<void()> task;
                    empty = false;
                    while(!tasks->try_dequeue(task)) {
                        if(tasks->size_approx()==0) {
                            empty = true;
                            break;
                        }
                    }
                    if(!empty)
                        task(); //execute task
                }
            }
        };
        for (int i=0;i!=numthreads;++i) { //create worker threads
            workers.emplace_back(thread_task);
        }
    }

    void add_task(std::function<void()> task) {
        tasks->enqueue(task);
        sem_post(&cv_sem);
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
    
    moodycamel::ConcurrentQueue<std::function<void()>>* getTaskQueuePointer() {
        return tasks;
    }
};

int main(int argc, char const *argv[]) {
    const char *file_name = argc>1 ? argv[1]: "graph.txt";

    std::ifstream reader(file_name); //graph reader
    int n; //number of nodes
    reader>>n;
    std::map<int,node> graph;

    for (int i=0;i!=n;++i) { //construct the edge lists with empty initialization
        graph.emplace(i+1,node());
    }
    
    for (int i=0;i!=n;++i) {
        int ne; // number of edges
        reader>>ne;
        for(int j=0; j<ne; j++) {
            int local;
            reader>>local; //other end of edge
            if (local!=i+1) { //insert into the graph
                graph[i+1].succs.emplace(local);
                graph[local].preds.emplace(i+1);
            }
        }
    }

    int n_threads = argc>2? atoi(argv[2]): 8;
    int task_pool_step = argc>3? atoi(argv[3]):10000;
    task_queue tq(n/task_pool_step, n_threads);

    bool empty;
    moodycamel::ConcurrentQueue<std::function<void()>>* tasks = tq.getTaskQueuePointer();

    std::atomic<bool> changeflag(false); //used to track if any colors changed
    std::map<int,std::unique_ptr<std::atomic<int>>> registers; //registers used for propagation
    
    for (auto i=1;i!=n+1;++i) { //zero-initialize the registers
        registers.emplace(i,std::make_unique<std::atomic<int>>(0));
    }
    
    std::vector<std::vector<int>> sccs; //output list
    std::mutex out_lk; //used to control access to the output list
    
    auto phase_one_single_iter=[&changeflag,&registers](int node_num,node& selfref,int own_val){
        for (int i : selfref.succs) {
            auto& succreg=registers[i]; //get reference to child's register
            
            while (true) {
                auto val = succreg->load(); //read value from child's register
                if (val<own_val) { //own color is greater than child's
                    auto res= succreg->compare_exchange_strong(val,own_val); //attempt propagation
                    if (res) { //succeeded in propagating
                        changeflag.store(true); //notify change
                        break;
                    } //else retry, as some other thread managed to alter register
                }
                else { //child's color is greater than own
                    break;
                }
            }
        }
    };
    
    auto phase_two=[&registers,&graph,&sccs,&out_lk](int node_num,node& selfref){
        if (registers[node_num]->load()==node_num) { //is a root of an SCC
            std::set<int> visited; //track nodes visited in reversed BFS
            std::queue<int> to_visit; //queue for BFS
            std::vector<int> scc; //list of nodes in the SCC
            visited.insert(node_num); //mark self as visited
            scc.push_back(node_num); //add self to SCC
            
            for (int i : selfref.preds) {
                to_visit.push(i);
                visited.insert(i); //mark reverse children as visited
            }
            
            while (!to_visit.empty()) { //there are still nodes to visit
                int x = to_visit.front(); //remove node from queue
                to_visit.pop();
                if (registers[x]->load()==node_num) { //if visited node has root's color
                    scc.push_back(x); //add to scc
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
    
    std::set<int> active_workers; //nodes that are still in graph
    for (int i=1;i!=n+1;++i) { //initialize with all nodes
        active_workers.insert(i);
    }
    
    unsigned found_sccs=0;
    auto start_time = std::chrono::high_resolution_clock::now(); //start timing
    
    while (active_workers.size()) { //while graph is non-empty
        int aw_size = active_workers.size();
        int tasks_created;
        for (auto& pair : registers) { //initialize registers with node's colors
            pair.second->store(pair.first);
        }
    
        std::atomic<int> finished(0); //used like a barrier
        do {
            changeflag.store(false);
            finished.store(0);
    
            tasks_created = 0;
            for (int i=0; i<aw_size; i+=task_pool_step) {
                auto task=[&,i,phase_one_single_iter](){ //one propagation from one node
                    std::set<int>::iterator it = std::next(active_workers.begin(), i);
                    int trip_count = task_pool_step<(aw_size-i)? task_pool_step:(aw_size-i);
                    for(int j=0; j<trip_count; j++) {
                        int node_num=*it;
                        node& selfref=graph[node_num];
                        int own_val=registers[node_num]->load();
                        phase_one_single_iter(node_num,selfref,own_val);
                        ++it;
                    }
                    ++finished;
                };
                tq.add_task(task); //schedule task
                tasks_created++;
            }
    
            while(true) {
                std::function<void()> task;
                empty = false;
                while(!tasks->try_dequeue(task)) {
                    if(tasks->size_approx()==0) {
                        empty = true;
                        break;
                    }
                }
                if(empty) {
                    break;
                } else {
                    task();
                }
            }

            while (finished.load()!=tasks_created) {} //wait for all threads to complete the iteration
    
        } while(changeflag.load()); //until graph reaches stable state
    
        finished.store(0); //reset barrier
    
        tasks_created = 0;
        for (int i=0; i<aw_size; i+=task_pool_step) {

            auto task=[&,i,phase_two](){ //initiate root check at all nodes

                std::set<int>::iterator it = std::next(active_workers.begin(), i);
                int trip_count = task_pool_step<(aw_size-i)? task_pool_step:(aw_size-i);
                for(int j=0; j<trip_count; j++) {
                    int node_num=*it;
                    node& selfref = graph[i];
                    phase_two(node_num,selfref);
                    ++it;
                }
                ++finished;
            
            };
            tq.add_task(task);

            tasks_created++;
        }

        while(true) {
            std::function<void()> task;
            empty = false;
            while(!tasks->try_dequeue(task)) {
                if(tasks->size_approx()==0) {
                    empty = true;
                    break;
                }
            }
            if(empty) {
                break;
            } else {
                task();
            }
        }
    
        while(finished.load()!=tasks_created) {} //wait for all threads to finish
    
        finished.store(0); //reset barrier
        for (auto i=found_sccs;i!=sccs.size();++i) { //loop over new SCCs added
            for (auto elem : sccs[i]) {
                active_workers.erase(elem); //remove node from active ids
                registers[elem]->store(n+1);
            }
        }
    
        found_sccs=sccs.size();
    }
    
    auto stop_time = std::chrono::high_resolution_clock::now(); //stop timing
    double micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(stop_time-start_time).count();
    std::cout<<"Time: "<< micro_sec/1e6 <<"\n";
    tq.stop(); //stop the thread pool's execution
    
    // for (const auto& elem : sccs) { //print found sccs
    //     for (const auto& inner_elem : elem) {
    //         std::cout<<inner_elem<<" ";
    //     }
    //     std::cout<<"\n";
    // }
    
}
