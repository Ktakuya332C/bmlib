#include "bm.h"
#include <cstring>
#include "math_utils.h"

BM::BM(int max_nodes, int max_edges) {
    pparams = new Graph(max_nodes, max_edges);
    pacts = new Graph(max_nodes, max_edges);
}

BM::~BM(void) {
    delete pparams;
    delete pacts;
}

Status BM::add_vis_node(std::string name, short input_idx, float bias) {
    return (*pparams).add_node(name, input_idx, bias);
}

Status BM::add_hid_node(std::string name, float bias) {
    return (*pparams).add_node(name, -1, bias);
}

Status BM::add_bm_edge(std::string name, std::string nd_name1, std::string nd_name2, float weight) {
    return (*pparams).add_edge(name, nd_name1, nd_name2, weight);
}

Status BM::add_bm_edge_fast(std::string name, std::string nd_name1, std::string nd_name2, float weight) {
    return (*pparams).add_edge_fast(name, nd_name1, nd_name2, weight, true);
}

Status BM::learn(Graph &updates, float learning_rate) {
    if (pparams->n_nodes != updates.n_nodes)
        return Status(false, "Update does not have same number of nodes");
    if (pparams->n_edges != updates.n_edges)
        return Status(false, "Update does not have same number of edges");
    
    for (int i=0; i<pparams->n_nodes; i++) {
        if (strcmp(updates.nodes[i].name, pparams->nodes[i].name) != 0)
            return Status(false, "Update does not have same name node at the same index");
        pparams->nodes[i].value += learning_rate * updates.nodes[i].value;
    }
    for (int i=0; i<pparams->n_edges; i++) {
        if (strcmp(updates.edges[i].name, pparams->edges[i].name) != 0)
            return Status(false, "Update does not have same name edge at the same index");
        pparams->edges[i].value += learning_rate * updates.edges[i].value;
    }
    
    return Status(true);
}

void BM::cor_cdby_data_stoc(std::vector<std::vector<float> > &data, Graph *pg, int n_iter) {
    pacts->copy_from(*pparams);
    pg->copy_from(*pparams);
    
    // Initialization
    for (int i=0; i<pg->n_nodes; i++) {
        pg->nodes[i].value = 0.0;
    }
    for (int i=0; i<pg->n_edges; i++) {
        pg->edges[i].value = 0.0;
    }
    
    // Data iteration
    for (std::vector<float> inpt: data) {
        
        // Initialization
        for (int i=0; i<pacts->n_nodes; i++) {
            if (pacts->nodes[i].input_idx >= 0) {
                pacts->nodes[i].value = inpt[pacts->nodes[i].input_idx];
            } else {
                pacts->nodes[i].value = uni_dist(gen);
            }
        }
        
        // Mean field iteration
        int cntd_edge_idx;
        for (int k=0; k<n_iter; k++) {
            for (int i=0; i<pacts->n_nodes; i++) {
                if (pacts->nodes[i].input_idx < 0) {
                    pacts->nodes[i].value = pparams->nodes[i].value;
                    for (int j=0; j<pacts->nodes[i].n_cntd_edges; j++) {
                        cntd_edge_idx = pacts->nodes[i].edge_idxs[j];
                        pacts->nodes[i].value += pparams->edges[cntd_edge_idx].value *
                                                            pacts->nodes[pacts->edges[cntd_edge_idx].another_node_idx(i)].value;
                    }
                    pacts->nodes[i].value = sigmoid(pacts->nodes[i].value);
                }
            }
        }
        
        // Store value
        for (int i=0; i<pacts->n_nodes; i++) {
            pg->nodes[i].value += pacts->nodes[i].value;
        }
        for (int i=0; i<pacts->n_edges; i++) {
            pg->edges[i].value += pacts->nodes[pacts->edges[i].node1_idx].value *
                                             pacts->nodes[pacts->edges[i].node2_idx].value;
        }
    }
    
    float data_size = (float)data.size();
    for (int i=0; i<pg->n_nodes; i++) {
        pg->nodes[i].value /= data_size;
    }
    for (int i=0; i<pg->n_edges; i++) {
        pg->edges[i].value /= data_size;
    }
}

void BM::cor_stoc(Graph *pg, int n_iter, int n_samp) {
    pacts->copy_from(*pparams);
    pg->copy_from(*pparams);

    // Initialization
    for (int i=0; i<pg->n_nodes; i++) {
        pg->nodes[i].value = 0.0;
    }
    for (int i=0; i<pg->n_edges; i++) {
        pg->edges[i].value = 0.0;
    }
    
    // Sample iteration
    for (int m=0; m<n_samp; m++) {

        // Initialization
        for (int i=0; i<pacts->n_nodes; i++) {
            pacts->nodes[i].value = samp(uni_dist(gen));
        }
        
        // Gibbs sampling
        int cntd_edge_idx;
        for (int k=0; k<n_iter; k++) {
            for (int i=0; i<pacts->n_nodes; i++) {
                pacts->nodes[i].value = pparams->nodes[i].value;
                for (int j=0; j<pacts->nodes[i].n_cntd_edges; j++) {
                    cntd_edge_idx = pacts->nodes[i].edge_idxs[j];
                    pacts->nodes[i].value += pparams->edges[cntd_edge_idx].value *
                                                        pacts->nodes[pacts->edges[cntd_edge_idx].another_node_idx(i)].value;
                }
                pacts->nodes[i].value = samp(sigmoid(pacts->nodes[i].value));
            }
        }
        
        // Store value
        for (int i=0; i<pacts->n_nodes; i++) {
            pg->nodes[i].value += pacts->nodes[i].value;
        }
        for (int i=0; i<pacts->n_edges; i++) {
            pg->edges[i].value += pacts->nodes[pacts->edges[i].node1_idx].value *
                                             pacts->nodes[pacts->edges[i].node2_idx].value;
        }
    }
    
    float samp_size = (float)n_samp;
    for (int i=0; i<pg->n_nodes; i++) {
        pg->nodes[i].value /= samp_size;
    }
    for (int i=0; i<pg->n_edges; i++) {
        pg->edges[i].value /= samp_size;
    }
}

void BM::updates_stoc(std::vector<std::vector<float> > &data, Graph *pg, int n_iter, int n_iter_samp, int n_samp) {
    Graph ph;
    cor_cdby_data_stoc(data, pg, n_iter);
    cor_stoc(&ph, n_iter_samp, n_samp);
    
    for (int i=0; i<pg->n_nodes; i++)
        pg->nodes[i].value -= ph.nodes[i].value;
    for (int i=0; i<pg->n_edges; i++)
        pg->edges[i].value -= ph.edges[i].value;
}

double BM::reconst_cost(std::vector<std::vector<float> > &data, int n_iter) {
    pacts->copy_from(*pparams);
    float value = 0.0;
    
    // Data iteration
    for (std::vector<float> inpt: data) {
        
        // Initialization
        for (int i=0; i<pacts->n_nodes; i++) {
            if (pacts->nodes[i].input_idx >= 0) {
                pacts->nodes[i].value = inpt[pacts->nodes[i].input_idx];
            } else {
                pacts->nodes[i].value = uni_dist(gen);
            }
        }
        
        // Mean field iteration, visible to hidden
        int cntd_edge_idx;
        for (int k=0; k<n_iter; k++) {
            for (int i=0; i<pacts->n_nodes; i++) {
                if (pacts->nodes[i].input_idx < 0) {
                    pacts->nodes[i].value = pparams->nodes[i].value;
                    for (int j=0; j<pacts->nodes[i].n_cntd_edges; j++) {
                        cntd_edge_idx = pacts->nodes[i].edge_idxs[j];
                        pacts->nodes[i].value += pparams->edges[cntd_edge_idx].value *
                                                            pacts->nodes[pacts->edges[cntd_edge_idx].another_node_idx(i)].value;
                    }
                    pacts->nodes[i].value = sigmoid(pacts->nodes[i].value);
                }
            }
        }
        
        // Mean field iteration, hidden to visible
        for (int k=0; k<n_iter; k++) {
            for (int i=0; i<pacts->n_nodes; i++) {
                if (pacts->nodes[i].input_idx >= 0) {
                    pacts->nodes[i].value = pparams->nodes[i].value;
                    for (int j=0; j<pacts->nodes[i].n_cntd_edges; j++) {
                        cntd_edge_idx = pacts->nodes[i].edge_idxs[j];
                        pacts->nodes[i].value += pparams->edges[cntd_edge_idx].value *
                                                            pacts->nodes[pacts->edges[cntd_edge_idx].another_node_idx(i)].value;
                    }
                    pacts->nodes[i].value = sigmoid(pacts->nodes[i].value);
                }
            }
        }
        
        // Calculate cost
        for (int i=0; i<pacts->n_nodes; i++) {
            if (pacts->nodes[i].input_idx >= 0) {
                value -= inpt[pacts->nodes[i].input_idx] * safe_log(pacts->nodes[i].value);
                value -= (1.0 - inpt[pacts->nodes[i].input_idx]) * safe_log(1.0 - pacts->nodes[i].value);
            }
        }
    }
    
    value /= (float)data.size();
    return value;
}
