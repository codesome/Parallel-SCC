struct node { //used for each node of the graph
    std::vector<int> succs; //set of all successors
};

int main(int argc, char const *argv[]) {

    /*
        n = number of nodes
    */

    std::vector<node> graph;
    // 'graph' is updated with the nodes
    // index in vector denotes the graph id

    int n_cuts = <integer>; // same as number of threads
    int step = n / n_cuts; // used to give nodes to a thread

    // union find for storing scc id for a node
    std::vector<int> scc_uf(n);

    // used to give unique id to scc in parallel
    std::atomic_int scc_ids(0);


    cross_edges; // all the edges from one graph to another graph cut (vector of vector)
    new_nodes; // new nodes that will be formed after running Tarjan on graph cuts (vector of vector)

    // for all n_cut number of threads thread
    (int thread_id) {

        /*===============================PHASE 1===============================*/
        std::map<int, node> &local_nodes = new_graph_global[thread_id];

        int start_index = thread_id*step;
        int end_index = (thread_id==n_cuts-1)? n: (thread_id+1)*step;
        
        local_cross_edges; // same as cross_edges but local to thread (vector in the (vector of vector))
        local_new_nodes; // same as new_nodes but local to thread (vector in the (vector of vector))

        { // Run Tarjan on the 'graph'
            #Some functions that happen inside Tarjan:
                1. For and edge u->v
                    If (v < start_index || v >= end_index) // outside the cut
                    And (start_index <= u < end_index) // inside the cut
                    Then the edge u->v is said to be cross edge

                    local_cross_edges.add(edge(u,v))

                2. When we find a scc

                    my_scc_id = scc_ids++;
                    for all w in scc:
                        scc_uf[w] = my_scc_id // marking the node's scc_id

                    local_new_nodes.add(node(my_scc_id)) // new node with id=my_scc_id
        }


        #BARRIER#


        /*===============================PHASE 2===============================*/

        for (edge e in local_cross_edges) {
            scc_id = scc_uf[e.source];

            // adding successor for the new graph using cross edge
            local_new_nodes[scc_id].succs.add(scc_uf[e.destination]);
        }

        // by using vector in (vector of vector), 
        // the local_cross_edges and local_new_nodes are inside
        // cross_edges and new_graph directly

    }; /* end of thread function */

    #join all threads here#
    /*===============================PHASE 3=============================================*/
    // Sequential code starts

    std::vector<node> new_graph;
    // gather all the nodes from new_nodes into new_graph
    // nodes in new_graph are sorted based on their scc_id, which acts like node id here

    { // Run Tarjan on new_graph

        Let 'temp_sccs' be the result of this Tarjan

        Hence temp_sccs contains the scc_id (from phase 1 and 2) in its strongly connected components
    }

    * For all the nodes v in 'graph'
        check for scc_uf[v] in temp_sccs // its scc_id in temp_sccs
        and add it in that scc 

    * After adding all nodes, sccs=temp_sccs, which is the result
}
