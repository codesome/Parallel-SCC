#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <chrono>
struct vertex {
    std::vector<int> succs;
    int index;
    int lowlink;
    bool onStack;
    vertex(std::vector<int>&& _succs) : succs(std::forward<std::vector<int>>(_succs)), index(-1), lowlink(-1), onStack(false) {}
};
struct graph {
    std::vector<vertex> verts;
};
std::vector<std::vector<int>> tarjan(graph& g) {
    int index = 0;
    std::stack<int> S;
    std::vector<std::vector<int>> retval;
    std::stack<std::tuple<int, int, int>> callstack;
    int v;
    unsigned widx;
    int w;
    auto strongconnect = [&index, &g, &S, &retval, &v, &widx, &w, &callstack]() -> void {
startlabel: g.verts[v].index = index;
        g.verts[v].lowlink = index;
        ++index;
        S.push(v);
        g.verts[v].onStack = true;
        for (widx = 0u; widx != g.verts[v].succs.size(); ++widx) {
            w = g.verts[v].succs[widx];
            if (g.verts[w].index == -1) {
                callstack.push(std::make_tuple(v, widx, w));
                v = w;
                //strongconnect();
                goto startlabel;
endlabel: auto temp = callstack.top();
                callstack.pop();
                v = std::get<0>(temp);
                widx = std::get<1>(temp);
                w = std::get<2>(temp);
                g.verts[v].lowlink = std::min(g.verts[v].lowlink, g.verts[w].lowlink);
            }
            else if (g.verts[w].onStack) {
                g.verts[v].lowlink = std::min(g.verts[v].lowlink, g.verts[w].index);
            }
        }
        if (g.verts[v].lowlink == g.verts[v].index) {
            std::vector<int> scc;
            int w;
            do {
                w = S.top();
                S.pop();
                g.verts[w].onStack = false;
                scc.emplace_back(w);
            } while (w != v);
            retval.emplace_back(std::move(scc));
        }
        if (!callstack.empty()) {
            goto endlabel;
        }
    };
    for (auto i = 0u; i != g.verts.size(); ++i) {
        if (g.verts[i].index == -1) {
            v = i;
            strongconnect();
        }
    }
    return retval;
}
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " num_of_vertices";
        return -1;
    }
    graph G;
    {
        std::ifstream reader(argv[1]);
        int V, edgecount;
        reader >> V;
        int start = 0;
        if (argc > 3) {
            start = atoi(argv[2]);
            V = atoi(argv[3]);
            if (V < start) return 0;
        }
        for(int i=0; i<start; i++) {
            int temp;
            reader >> edgecount;
            for (auto j = 0; j != edgecount; ++j) {
                reader >> temp;
            }
        }
        for (auto i = start; i != V; ++i) {
            reader >> edgecount;
            std::vector<int> dests;
            int temp;
            for (auto j = 0; j != edgecount; ++j) {
                reader >> temp;
                if((temp-1) < V && (temp-1) >= start) {
                    dests.emplace_back(temp - 1 - start);
                }
            }
            G.verts.emplace_back(std::move(dests));
        }
    }
    auto start_time = std::chrono::high_resolution_clock::now(); //start timing
    auto sccs = tarjan(G);
    auto stop_time = std::chrono::high_resolution_clock::now(); //start timing


    double micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
    std::cout << "Time: " << micro_sec / 1e6 << "\n";
    
    // for (int i = 0; i < sccs.size(); i++) {
    //     sort(sccs[i].begin(), sccs[i].end());
    // }
    // sort(sccs.begin(), sccs.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
    //     return a[0] < b[0];
    // });
    // for (const auto& elem : sccs) { //print found sccs
    //     for (const auto& inner_elem : elem) {
    //         std::cout << inner_elem << " ";
    //     }
    //     std::cout << "\n";
    // }

    std::cout << "Size: " <<  sccs.size() << std::endl;
}
