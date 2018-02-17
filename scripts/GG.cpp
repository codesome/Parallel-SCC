#include <iostream>
#include <fstream>
#include <cassert>
#include <random>
#include <unordered_set>
using namespace std;


int main(int argc, char const *argv[]) {
    if(argc < 4) {
        cout << argv[0] << " {num_vertices} {num_edges} {probablity} {output_file_name2}\n";
        return 1;
    }

    int NOV = atoi(argv[1])
      , NOE = atoi(argv[2]);
    float probablity = atof(argv[3]);

    random_device rd, rd_p;
    mt19937 engine(rd()), engine_p(rd_p());
    uniform_int_distribution<int> gen(0,NOV-1);
    uniform_real_distribution<> dis(0, 1);
    int u, v, count;
    unordered_set<int> edges;

    ofstream file;
    file.open(argv[4]);
    file << NOV << "\n";
    for(u=0; u<NOV; u++) {
        count = 0;
        edges.clear();
        for(int i=0; i<NOE; i++) {
            v = gen(engine);
            if(u == v || edges.find(v)!=edges.end()) {
                i--;
                continue;
            } else if(dis(engine_p) <= probablity) {
                count++;
                edges.insert(v);
            }
        }
        file << count;
        for(int e : edges) {
            file << " " << e+1;
        }
        file << "\n";
    }

    return 0;
}