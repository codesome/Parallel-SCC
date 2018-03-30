#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
struct vertex {
    std::vector<int> succs;
    int index;
    int lowlink;
    bool onStack;
    vertex(std::vector<int>&& _succs) : succs(std::forward<std::vector<int>>(_succs)),index(-1),lowlink(-1),onStack(false) {}
};
struct graph {
    std::vector<vertex> verts;
};
std::vector<std::vector<int>> tarjan(graph& g) {
    int index=0;
    std::stack<int> S;
    std::vector<std::vector<int>> retval;
    std::stack<std::tuple<int,int,int>> callstack;
    int v;
    unsigned widx;
    int w;
    auto strongconnect=[&index,&g,&S,&retval,&v,&widx,&w,&callstack]() -> void {
        startlabel: g.verts[v].index=index;
        g.verts[v].lowlink=index;
        ++index;
        S.push(v);
        g.verts[v].onStack=true;
        for (widx=0u;widx!=g.verts[v].succs.size();++widx) {
            w=g.verts[v].succs[widx];
            if (g.verts[w].index==-1) {
                callstack.push(std::make_tuple(v,widx,w));
                v=w;
                //strongconnect();
                goto startlabel;
                endlabel: auto temp=callstack.top();
                callstack.pop();
                v=std::get<0>(temp);
                widx=std::get<1>(temp);
                w=std::get<2>(temp);
                g.verts[v].lowlink=std::min(g.verts[v].lowlink,g.verts[w].lowlink);
            }
            else if (g.verts[w].onStack) {
                g.verts[v].lowlink=std::min(g.verts[v].lowlink,g.verts[w].index);
            }
        }
        if (g.verts[v].lowlink == g.verts[v].index) {
            std::vector<int> scc;
            int w;
            do {
                w=S.top();
                S.pop();
                g.verts[w].onStack=false;
                scc.emplace_back(w);
            } while (w!=v);
            std::sort(std::begin(scc),std::end(scc));
            retval.emplace_back(std::move(scc));
        }
        if (!callstack.empty()) {
            goto endlabel;
        }
    };
    for (auto i=0u;i!=g.verts.size();++i) {
        if (g.verts[i].index==-1) {
            v=i;
            strongconnect();
        }
    }
    return retval;
}
int main(int argc, char** argv) {
    if (argc!=2) {
        std::cout<<"Usage: "<<argv[0]<<" num_of_vertices";
        return -1;
    }
    graph G;
    {
        std::ifstream reader(argv[1]);
        int V,edgecount;
        reader>>V;
        for (auto i=0;i!=V;++i) {
            reader>>edgecount;
            std::vector<int> dests;
            int temp;
            for (auto j=0;j!=edgecount;++j) {
                reader>>temp;
                dests.emplace_back(temp-1);
            }
            G.verts.emplace_back(std::move(dests));
        }
    }
    auto sccs=tarjan(G);
    std::cout<<sccs.size()<<"\n";
    std::sort(std::begin(sccs),std::end(sccs),[](const auto& vec1, const auto& vec2){ return vec1[0] < vec2[0];});
    for (auto ind=0u;ind!=sccs.size();++ind) {
        for (auto elem : sccs[ind]) {
            std::cout<<elem+1<<" ";
        }
        std::cout<<"\n";
    }
}
