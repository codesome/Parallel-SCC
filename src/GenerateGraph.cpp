#include <iostream>
#include <fstream>
#include <cassert>
#include <random>
using namespace std;

int main(int argc, char const *argv[]) {
    if(argc < 4) {
        cout << argv[0] << " {num_vertices} {num_edges} {output_file_name}\n";
        return 1;
    }

    int NOV = atoi(argv[1])
      , NOE = atoi(argv[2]);

    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> gen(0,NOV-1);
    ofstream file;
    file.open(argv[3]);
    file << NOV << " " << NOE << "\n"; 
    
    int i = 0, j, u, v, edge[NOE][2], count, duplicate;

    while(i < NOE) {
        duplicate = false;
        u = gen(engine);
        v = gen(engine);
        //u = rand()%NOV;
        //v = rand()%NOV;
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
            file << u+1 << " " << v+1 << "\n";
        }
        i++;
    }
    file.close();

    return 0;
}