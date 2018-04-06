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
#include <stack>
#include <tuple>
#include <vector>
#include <map>

struct node { //used for each node of the graph
    int index;
    int lowlink;
    bool onStack;
    node(): index(-1), lowlink(-1), onStack(false) {}
    node(int index): index(index), lowlink(-1), onStack(false) {}
    std::vector<int> succs; //set of all successors
};

int main(int argc, char const *argv[]) {
    const char *file_name = argc > 1 ? argv[1] : "graph.txt";

    std::ifstream reader(file_name); //graph reader
    int n; //number of nodes
    reader >> n;
    std::vector<node> graph;

    for (int i = 0; i != n; ++i) { //construct the edge lists with empty initialization
        graph.emplace_back();
    }

    for (int i = 0; i != n; ++i) {
        int ne; // number of edges
        reader >> ne;
        for (int j = 0; j < ne; j++) {
            int local;
            reader >> local; //other end of edge
            if (local != i + 1) { //insert into the graph
                graph[i].succs.emplace_back(local - 1);
            }
        }
    }
    
    int n_cuts = argc > 2 ? atoi(argv[2]) : 8;
    int step = n / n_cuts;

    auto start_time = std::chrono::high_resolution_clock::now(); //start timing

    std::vector<int> scc_uf(n);

    std::atomic_int scc_ids(0);
    // std::vector<std::vector<std::pair<int, int>>> cross_edges_global(n_cuts);
    std::vector<std::map<int, node>> new_graph_global(n_cuts);
    std::atomic_int nodes_added(0);

    /*===============================PHASE 1===============================*/
    std::function<void(int)> thread_task =
        [&nodes_added, n, n_cuts, step, &graph, &scc_uf, &scc_ids, &new_graph_global]
    (int thread_id) {
        int sccfound = 0;
        std::map<int, node> &local_nodes = new_graph_global[thread_id];

        int start_index = thread_id * step;
        int end_index;
        if (thread_id == n_cuts - 1) {
            end_index = n;
        } else {
            end_index = (thread_id + 1) * step;
        }

        // std::vector<std::pair<int, int>> &cross_edges = cross_edges_global[thread_id];

        // Tarjan
        {
            int index = 0;
            std::stack<int> S;
            std::vector<std::vector<int>> retval;
            std::stack<std::tuple<int, int, int>> callstack;
            int v;
            unsigned widx;
            int w;
            auto strongconnect =
            [&sccfound, &nodes_added, &index, &graph, &S, &retval, &v, &widx, &w, &callstack, &scc_uf, start_index, end_index, &scc_ids, &local_nodes]() -> void {
startlabel:
                graph[v].index = index;
                graph[v].lowlink = index;
                ++index;
                S.push(v);
                graph[v].onStack = true;
                for (widx = 0u; widx != graph[v].succs.size(); ++widx) {
                    w = graph[v].succs[widx];
                    if (w < start_index || w >= end_index) {
                        // cross_edges.emplace_back(v, w);
                        continue;
                    }
                    if (graph[w].index == -1) {
                        callstack.push(std::make_tuple(v, widx, w));
                        v = w;
                        goto startlabel;
endlabel:
                        auto temp = callstack.top();
                        callstack.pop();
                        v = std::get<0>(temp);
                        widx = std::get<1>(temp);
                        w = std::get<2>(temp);
                        graph[v].lowlink = std::min(graph[v].lowlink, graph[w].lowlink);
                    }
                    else if (graph[w].onStack) {
                        graph[v].lowlink = std::min(graph[v].lowlink, graph[w].index);
                    }
                }
                if (graph[v].lowlink == graph[v].index) {
                    int w;
                    int my_scc_id = scc_ids++;
                    sccfound++;
                    do {
                        w = S.top();
                        S.pop();
                        graph[w].onStack = false;
                        scc_uf[w] = my_scc_id;
                    } while (w != v);
                    nodes_added++;
                    local_nodes.emplace(std::make_pair(my_scc_id, node(my_scc_id)));
                }
                if (!callstack.empty()) {
                    goto endlabel;
                }
            };
            for (auto i = start_index; i != end_index; ++i) {
                if (graph[i].index == -1) {
                    v = i;
                    strongconnect();
                }
            }
        }

        // printf("thread%d: SCC=%d, %d %d\n", thread_id, sccfound, start_index, end_index);

    };

    std::vector<std::thread> workers;

    for (int i = 0; i < n_cuts; ++i) {
        workers.emplace_back([](std::function<void(int)> task, int thread_id) {
            task(thread_id);
        }, thread_task, i);
    }

    for (auto iter = std::begin(workers); iter != std::end(workers); ++iter) {
        iter->join();
    }

    // printf("Before %d\n", scc_ids.load() );
    // TODO: keep phase 1 and 2 in single thread and use barriers
    /*===============================PHASE 2===============================*/
    std::function<void(int)> thread_task2 =
        [&graph, n, n_cuts, step, &scc_uf, &new_graph_global]
    (int thread_id) {

        std::map<int, node> &local_nodes = new_graph_global[thread_id];

        int start_index = thread_id * step;
        int end_index;
        if (thread_id == n_cuts - 1) {
            end_index = n;
        } else {
            end_index = (thread_id + 1) * step;
        }

        // std::vector<std::pair<int, int>> &cross_edges = cross_edges_global[thread_id];

        for(int i=start_index; i!=end_index; i++) {
            int scc_id = scc_uf[i];
            for(auto v: graph[i].succs) {
                int succ_scc_id = scc_uf[v];
                if(scc_id != succ_scc_id) {
                    local_nodes[scc_id].succs.push_back(succ_scc_id);
                }
            }
        }
        // for (auto& p : cross_edges) {
        //     int scc_id = scc_uf[p.first];
        //     local_nodes[scc_id].succs.push_back(scc_uf[p.second]);
        //     printf("%d->%d\n",p.first,p.second);
        // }

    };

    workers.clear();

    for (int i = 0; i < n_cuts; ++i) {
        workers.emplace_back([](std::function<void(int)> task, int thread_id) {
            task(thread_id);
        }, thread_task2, i);
    }

    for (auto iter = std::begin(workers); iter != std::end(workers); ++iter) {
        iter->join();
    }

    /*===============================PHASE 3===============================*/

    int new_n = scc_ids.load();
    std::vector<node> new_graph;
    new_graph.reserve(new_n);
    for (auto& thread_nodes : new_graph_global) {
        for (auto& node_pair : thread_nodes) {
            // printf("%d %d\n", node_pair.first, node_pair.second.index );
            new_graph.emplace_back(std::move(node_pair.second));
        }
    }

    std::sort(new_graph.begin(), new_graph.end(), [](const node & a, const node & b) {
        return a.index < b.index;
    });

    // for(auto nd: new_graph) {
    //     printf("Node=%d, ", nd.index);
    //     for(auto su: nd.succs) {
    //         printf("%d ", su);
    //     }
    //     printf("\n");
    // }

    for (auto& n : new_graph) {
        n.index = -1;
    }

    std::swap(graph, new_graph);


    // Tarjan
    std::vector<std::vector<int>> retval;
    {
        int index = 0;
        std::stack<int> S;
        std::stack<std::tuple<int, int, int>> callstack;
        int v;
        unsigned widx;
        int w;
        auto strongconnect =
        [&index, &graph, &S, &retval, &v, &widx, &w, &callstack]() -> void {
startlabel_last:
            graph[v].index = index;
            graph[v].lowlink = index;
            ++index;
            S.push(v);
            graph[v].onStack = true;
            for (widx = 0u; widx != graph[v].succs.size(); ++widx) {
                w = graph[v].succs[widx];
                if (graph[w].index == -1) {
                    callstack.push(std::make_tuple(v, widx, w));
                    v = w;
                    goto startlabel_last;
endlabel_last:
                    auto temp = callstack.top();
                    callstack.pop();
                    v = std::get<0>(temp);
                    widx = std::get<1>(temp);
                    w = std::get<2>(temp);
                    graph[v].lowlink = std::min(graph[v].lowlink, graph[w].lowlink);
                }
                else if (graph[w].onStack) {
                    graph[v].lowlink = std::min(graph[v].lowlink, graph[w].index);
                }
            }
            if (graph[v].lowlink == graph[v].index) {
                std::vector<int> scc;
                int w;
                do {
                    w = S.top();
                    S.pop();
                    graph[w].onStack = false;
                    scc.emplace_back(w);
                } while (w != v);
                retval.emplace_back(std::move(scc));
            }
            if (!callstack.empty()) {
                goto endlabel_last;
            }
        };
        for (auto i = 0; i != new_n; ++i) {
            if (graph[i].index == -1) {
                v = i;
                strongconnect();
            }
        }
    }




    std::vector<int> scc_index_map(new_n);
    int idx = 0;
    for (auto& v : retval) {
        for (auto& i : v) {
            scc_index_map[i] = idx;
        }
        idx++;
        v.clear();
    }

    idx = 0;
    for (auto& scc_id : scc_uf) {
        auto idx_of_scc = scc_index_map[scc_id];
        retval[idx_of_scc].push_back(idx);
        idx++;
    }

    auto stop_time = std::chrono::high_resolution_clock::now(); //stop timing
    double micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
    std::cout << "Time: " << micro_sec / 1e6 << "\n";

    // for(int i=0; i<retval.size(); i++) {
    //     sort(retval[i].begin(), retval[i].end());
    // }
    // sort(retval.begin(), retval.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
    //     return a[0] < b[0];
    // });
    // for (const auto& elem : retval) { //print found retval
    //     for (const auto& inner_elem : elem) {
    //         std::cout<<inner_elem<<" ";
    //     }
    //     std::cout<<"\n";
    // }

    std::cout << "Size: " <<  retval.size() << std::endl;
}
