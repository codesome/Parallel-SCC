#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
int main(int argc, char** argv) {
    if (argc!=6) {
        std::cout<<"Usage: "<<argv[0]<<" number_of_sccs dag_density dag_strength (mean_scc_size-1) outputfilename\n";
        return 1;
    }
    auto scc_num=std::atoi(argv[1]);
    auto dag_density=std::atof(argv[2]);
    auto dag_strength=std::atof(argv[3]);
    auto mean_scc_size=std::atof(argv[4]);
    if (scc_num<=0 || dag_density < 0 || dag_density > 1 || mean_scc_size < 0) {
        std::cout<<"Error: Invalid parameters\n";
        return 1;
    }
    std::random_device rd;
    std::mt19937 engine(rd());
    //Phase One: Generate DAG
    std::vector<std::vector<int>> dag_adj_list;
    dag_adj_list.reserve(scc_num);
    for (auto i=0;i!=scc_num-1;++i) {
        std::binomial_distribution<> edge_distro(scc_num-i-1,dag_density);
        std::vector<int> choices;
        auto num_edges=edge_distro(engine);
        if (num_edges) {
            choices.reserve(num_edges);
            int gen_seed=i+1;
            for (;gen_seed!=i+num_edges+1;++gen_seed) {
                choices.emplace_back(gen_seed);
            }
            for (;gen_seed!=scc_num;++gen_seed) {
                std::uniform_int_distribution<> locator(0,gen_seed-i-1);
                auto choice=locator(engine);
                if (choice < num_edges) {
                    choices[choice]=gen_seed;
                }
            }
        }
        dag_adj_list.emplace_back(std::move(choices));
    }
    dag_adj_list.emplace_back();
    /*for (auto i=0;i!=scc_num;++i) {
        std::cout<<i<<" - ";
        for (auto j=0u;j!=dag_adj_list[i].size();++j) {
            std::cout<<dag_adj_list[i][j]<<" ";
        }
        std::cout<<"\n";
    }*/
    std::vector<std::vector<std::vector<int>>> dag_nodes;
    std::poisson_distribution<> size_distro(mean_scc_size);
    for (auto i=0;i!=scc_num;++i) {
        auto scc_size=size_distro(engine)+1;
        std::vector<std::vector<int>> scc;
        scc.resize(scc_size);
        for (auto j=0;j!=scc_size;++j) {
            std::vector<int> in;
            in.reserve(scc_size);
            int count=0;
            std::generate_n(std::back_inserter(in),scc_size,[&count](){return count++;});
            std::swap(in[0],in[j]);
            std::shuffle(std::begin(in)+1,std::end(in),engine);
            for (auto k=1;k!=scc_size;++k) {
                std::uniform_int_distribution<> srcpicker(0,k-1);
                auto src=srcpicker(engine);
                scc[in[src]].emplace_back(in[k]);
            }
        }
        for (auto j=0;j!=scc_size;++j) {
            std::sort(std::begin(scc[j]),std::end(scc[j]));
            auto last=std::unique(std::begin(scc[j]),std::end(scc[j]));
            scc[j].erase(last,std::end(scc[j]));
        }
        dag_nodes.emplace_back(std::move(scc));
    }
    /*for (auto i=0;i!=scc_num;++i) {
        std::cout<<i<<":\n";
        for (auto j=0u;j!=dag_nodes[i].size();++j) {
            std::cout<<"\t"<<j<<" - ";
            for (auto k=0u;k!=dag_nodes[i][j].size();++k) {
                std::cout<<dag_nodes[i][j][k]<<" ";
            }
            std::cout<<"\n";
        }
    }*/
    std::map<std::tuple<int,int>,int> resolver;
    int counter=0;
    for (auto i=0;i!=scc_num;++i) {
        for (auto j=0;j!=dag_nodes[i].size();++j) {
            resolver.emplace(std::make_pair(std::make_tuple(i,j),counter));
            ++counter;
        }
    }
    std::vector<std::vector<int>> adj_list;
    adj_list.resize(counter);
    for (auto i=0;i!=scc_num;++i) {
        for (auto j=0;j!=dag_nodes[i].size();++j) {
            for (auto k=0;k!=dag_nodes[i][j].size();++k) {
                adj_list[resolver[std::make_tuple(i,j)]].emplace_back(resolver[std::make_tuple(i,dag_nodes[i][j][k])]);
            }
        }
    }
    for (auto i=0;i!=scc_num;++i) {
        for (auto j=0;j!=dag_adj_list[i].size();++j) {
            std::vector<std::tuple<int,int>> edges;
            auto sz1=dag_nodes[i].size();
            auto sz2=dag_nodes[dag_adj_list[i][j]].size();
            std::uniform_int_distribution<> srcpicker(0,sz1-1);
            std::uniform_int_distribution<> destpicker(0,sz2-1);
            std::poisson_distribution<> cross_edge_distro(sz1*sz2*dag_strength);
            auto tries=cross_edge_distro(engine);
            for (auto k=0;k!=tries;++k) {
                auto left=srcpicker(engine);
                auto right=destpicker(engine);
                edges.emplace_back(std::make_tuple(resolver[std::make_tuple(i,left)],resolver[std::make_tuple(dag_adj_list[i][j],right)]));
            }
            std::sort(std::begin(edges),std::end(edges));
            auto last=std::unique(std::begin(edges),std::end(edges));
            edges.erase(last,std::end(edges));
            for (auto k=0;k!=edges.size();++k) {
                adj_list[std::get<0>(edges[k])].emplace_back(std::get<1>(edges[k]));
            }
        }
    }
    //finally construct the real graph
    std::ofstream writer(argv[5]);
    writer<<adj_list.size()<<"\n";
    for (auto i=0;i!=adj_list.size();++i) {
        std::sort(std::begin(adj_list[i]),std::end(adj_list[i]));
        auto last=std::unique(std::begin(adj_list[i]),std::end(adj_list[i]));
        adj_list[i].erase(last,std::end(adj_list[i]));
        writer<<adj_list[i].size()<<" ";
        for (auto j=0;j!=adj_list[i].size();++j) {
            writer<<adj_list[i][j]+1<<" ";
        }
        writer<<"\n";
    }
}
