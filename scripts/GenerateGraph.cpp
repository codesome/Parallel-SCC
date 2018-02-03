#include <iostream>
#include <fstream>
#include <cassert>
#include <random>
using namespace std;

int main(int argc, char const *argv[]) {
    if(argc < 4) {
        cout << argv[0] << " {num_vertices} {num_edges} {output_file_name2}\n";
        return 1;
    }

    int NOV = atoi(argv[1])
      , NOE = atoi(argv[2]);

    vector< vector<int> > vec(NOV);
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> gen(0,NOV-1);
    
    int i = 0, j, u, v, edge[NOE][2], count, duplicate;

    while(i < NOE) {
        duplicate = false;
        u = gen(engine);
        v = gen(engine);
        if(u == v)
            continue;
        else {
            for(j = 0; j < i; j++) {
                if(u == edge[j][0] && v == edge[j][1]) {
                    i--;
                    duplicate = true;
                    break;
                }
            }
        }
        if(!duplicate) {
            edge[i][0] = u;
            edge[i][1] = v;
            vec.at(u).push_back(v);
        }
        i++;
    }

    ofstream file;
    file.open(argv[3]);
    file << NOV << "\n";
    u = 1;
    for(vector<int> vv: vec) {
        file << vv.size();
        for(int i: vv) {
            file << " " << (i+1);
        }
        file << "\n";
        u++;
    }

    return 0;
}