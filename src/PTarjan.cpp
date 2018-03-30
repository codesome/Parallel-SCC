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
#include <stack>
#include <vector>

struct node { //used for each node of the graph
    int index;
    int lowlink;
    bool onStack;
    node(): index(-1), lowlink(-1), onStack(false) {}
    std::vector<int> preds; //set of all predecessors
    std::vector<int> succs; //set of all successors
};

int main(int argc, char const *argv[]) {
    const char *file_name = argc>1 ? argv[1]: "graph.txt";

    std::ifstream reader(file_name); //graph reader
    int n; //number of nodes
    reader>>n;
    std::vector<node> graph;

    for (int i=0;i!=n;++i) { //construct the edge lists with empty initialization
        graph.emplace_back();
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

    int n_cuts = argc>2? atoi(argv[2]): 8;
    int step = n/n_cuts;
    
    std::vector<std::vector<int>> sccs; //output list
    auto start_time = std::chrono::high_resolution_clock::now(); //start timing

    std::vector<int> graph_cut_uf(n);
    std::vector<int> scc_uf(n);

    std::atomic_int scc_ids(0);

    std::function<void(int)> thread_task = 
    [n,n_cuts,step,&graph_cut_uf,&graph,&scc_uf,&scc_ids]
    (int thread_id){

        int start_index = thread_id*step;
        int end_index;
        if(thread_id==n_cuts-1) {
            end_index = n;
        } else {
            end_index = (thread_id+1)*step;
        }

        for (int i = start_index; i < end_index; ++i) {
            graph_cut_uf[i] = thread_id;
        }

        std::vector<std::pair<int,int>> cross_edges;

        {
            int index=0;
            std::stack<int> S;
            std::vector<std::vector<int>> retval;
            std::stack<std::tuple<int,int,int>> callstack;
            int v;
            unsigned widx;
            int w;
            auto strongconnect =
            [&index,&graph,&S,&retval,&v,&widx,&w,&callstack,&scc_uf,&cross_edges,start_index,end_index,&scc_ids]() -> void {
                startlabel: graph[v].index=index;
                graph[v].lowlink=index;
                ++index;
                S.push(v);
                graph[v].onStack=true;
                for (widx=0u;widx!=graph[v].succs.size();++widx) {
                    w=graph[v].succs[widx];
                    if(w < start_index || w >= end_index) {
                        cross_edges.emplace_back(v,w);
                        continue;
                    }
                    if (graph[w].index==-1) {
                        callstack.push(std::make_tuple(v,widx,w));
                        v=w;
                        goto startlabel;
                        endlabel: auto temp=callstack.top();
                        callstack.pop();
                        v=std::get<0>(temp);
                        widx=std::get<1>(temp);
                        w=std::get<2>(temp);
                        graph[v].lowlink=std::min(graph[v].lowlink,graph[w].lowlink);
                    }
                    else if (graph[w].onStack) {
                        graph[v].lowlink=std::min(graph[v].lowlink,graph[w].index);
                    }
                }
                if (graph[v].lowlink == graph[v].index) {
                    // std::vector<int> scc;
                    int w;
                    int my_scc_id = scc_ids++;
                    do {
                        w=S.top();
                        S.pop();
                        graph[w].onStack=false;
                        scc_uf[w] = my_scc_id;
                        // scc.emplace_back(w);
                        scc_uf[w] = v;
                    } while (w!=v);
                    // std::sort(std::begin(scc),std::end(scc));
                    // retval.emplace_back(std::move(scc));
                }
                if (!callstack.empty()) {
                    goto endlabel;
                }
            };
            for (auto i=start_index;i!=end_index;++i) {
                if (graph[i].index==-1) {
                    v=i;
                    strongconnect();
                }
            }
        }

        // for(auto p: cross_edges) {
        //     printf("%d %d %d\n", thread_id, p.first, p.second );
        // }




    };

    std::vector<std::thread> workers;

    for (int i = 0; i < n_cuts; ++i) {
        workers.emplace_back([](std::function<void(int)> task, int thread_id){
            task(thread_id);
        }, thread_task, i);
    }

    for (auto iter = std::begin(workers);iter!= std::end(workers);++iter) {
        iter->join();
    }


    printf("SCC %d\n", scc_ids.load() );

    // int new_n = scc_ids.load();
    // std::vector<node> condensed_graph;
    // for (int i=0; i!=new_n; ++i) {
    //     condensed_graph.emplace_back();
    // }

    // for(auto p: cross_edges) {
        
    // }





    auto stop_time = std::chrono::high_resolution_clock::now(); //stop timing
    double micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(stop_time-start_time).count();
    std::cout<<"Time: "<< micro_sec/1e6 <<"\n";

    
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
