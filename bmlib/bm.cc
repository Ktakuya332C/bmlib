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

float BM::anel_hamil(float beta) {
    float value = 0.0;
    for (int i=0; i<pacts->n_nodes; i++) {
        value -= pparams->nodes[i].value * pacts->nodes[i].value;
    }
    for (int i=0; i<pparams->n_edges; i++) {
        value -= beta * pparams->edges[i].value *
                      pacts->nodes[pacts->edges[i].node1_idx].value *
                      pacts->nodes[pacts->edges[i].node2_idx].value;
    }
    return value;
}

Status BM::part_func_ais(float *out_log_z, int n_step, int n_samp) {
    pacts->copy_from(*pparams);
    double log_z = 0.0;
    double beta;
    
    // Sample iteration
    for (int m=0; m<n_samp; m++) {
        
        // First sample
        for (int i=0; i<pacts->n_nodes; i++) {
            pacts->nodes[i].value = samp(sigmoid(pparams->nodes[i].value));
        }
        
        // Update
        log_z += anel_hamil(0.0);
        log_z -= anel_hamil(1.0 / (float)n_step);
        
        // Step iteration
        int idx;
        for (int k=0; k<n_step; k++) {
            beta = (float)k / (float)n_step;
            
            // Gibbs sample
            for (int i=0; i<pacts->n_nodes; i++) {
                pacts->nodes[i].value = pparams->nodes[i].value;
                for (int j=0; j<pparams->nodes[i].n_cntd_edges; j++) {
                    idx = pparams->nodes[i].edge_idxs[j];
                    pacts->nodes[i].value += beta * pparams->edges[idx].value *
                                                            pacts->nodes[pacts->edges[idx].another_node_idx(i)].value;
                }
                pacts->nodes[i].value = samp(sigmoid(pacts->nodes[i].value));
            }
            
            // Update
            log_z += anel_hamil((float)k / (float)n_step);
            log_z -= anel_hamil((float)(k+1) / (float)n_step);
        }
    }
    log_z /= (float)n_samp;
    
    // Effect of Z_A
    for (int i=0; i<pparams->n_nodes; i++) {
        log_z += log(1+exp(pparams->nodes[i].value));
    }
    *out_log_z = log_z;
    return Status(true);
}

Status BM::part_func_det(float *out_log_z) {
    if (pparams->n_nodes > 31)
        return Status(false, "Too many nodes to calculate deterministic partition functions");
    float z = 0.0;
    
    int node_idx;
    for (int i=0; i<(1<<pparams->n_nodes); i++) {
        node_idx = 0;
        for (int j=0; j<pparams->n_nodes; j++) {
            pacts->nodes[j].value = (float)((i>>node_idx)&1);
            node_idx++;
        }
        z += exp(-anel_hamil(1.0));
    }
    *out_log_z = log(z);
    return Status(true);
}

Status BM::part_func_imp(Graph &params, float in_log_z, float *out_log_z, int n_samp) {    
    if (pacts->n_nodes != params.n_nodes)
        return Status(false, "Input parameter graph has different number of nodes");
    if (pacts->n_edges != params.n_edges)
        return Status(false, "Input parameter graph has different number of edges");
    for (int i=0; i<pacts->n_nodes; i++) {
        if (std::strcmp(pacts->nodes[i].name, params.nodes[i].name) != 0) {
            return Status(false, "Input parameter graph has different name node in different place ");
        }
    }
    for (int i=0; i<pacts->n_edges; i++) {
        if (std::strcmp(pacts->nodes[i].name, params.nodes[i].name) != 0) {
            return Status(false, "Input parameter graph has different name edge in different place ");
        }
    }
    
    // Initialization
    float z_ratio = 0.0;
    pacts->copy_from(*pparams);
    BM bm(pacts->max_nodes, pacts->max_edges);
    bm.pparams->copy_from(params);
    bm.pacts->copy_from(params);
    
    // Sample iteration
    for (int k=0; k<n_samp; k++) {
        
        // Sample
        int idx;
        for (int i=0; i<pacts->n_nodes; i++) {
            pacts->nodes[i].value = bm.pparams->nodes[i].value;
            for (int j=0; j<pacts->nodes[i].n_cntd_edges; j++) {
                idx = pacts->nodes[i].edge_idxs[j];
                pacts->nodes[i].value += bm.pparams->edges[idx].value *
                                                        pacts->nodes[pacts->edges[idx].another_node_idx(i)].value;
            }
            pacts->nodes[i].value = samp(sigmoid(pacts->nodes[i].value));
        }
        
        // Calculate z for this sample
        bm.pacts->copy_from(*pacts);
        z_ratio += exp(-anel_hamil(1.0) + bm.anel_hamil(1.0));
    }
    z_ratio /= (float)n_samp;
    
    *out_log_z = in_log_z + log(z_ratio);
    
    return Status(true);
}

Status BM::reconst_cost(std::vector<std::vector<float> > &data, float *cost, int n_iter) {
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
    *cost = value;
    return Status(true);
}

#include <iostream>
Status BM::kl_det(std::vector<std::vector<float> > &data, float *cost) {
    pacts->copy_from(*pparams);
    
    // Data iteration
    float value;
    int n_vis, n_hid, hid_idx;
    for (std::vector<float> &inpt: data) {
        value = 0.0;
        
        // Initialization
        n_vis = 0;
        for (int i=0; i<pacts->n_nodes; i++) {
            if (pacts->nodes[i].input_idx >= 0) {
                pacts->nodes[i].value = inpt[pacts->nodes[i].input_idx];
                n_vis++;
            }
        }
        
        // Hidden activation iteration
        n_hid = pacts->n_nodes - n_vis;
        for (int i=0; i<(1<<n_hid); i++) {
            
            // insert hidden activatin corresponds to i
            hid_idx = 0;
            for (int j=0; j<pacts->n_nodes; j++) {
                if (pacts->nodes[j].input_idx < 0) {
                    pacts->nodes[j].value = (float)((i>>hid_idx)&1);
                    hid_idx++;
                }
            }
            value += exp(-anel_hamil(1.0));
        }
        
        *cost -= log(value);
    }
    *cost /= (float)data.size();
    
    // Partition function
    Status s = part_func_det(&value);
    if (!s.ok) return s;
    *cost += value;
    
    *cost -= log((float)data.size());
    
    value = 0.0;
    int n_dupls[data.size()];
    for (int i=0; i<data.size(); i++) {
        n_dupls[i] = 1;
        for (int j=0; j<data.size(); j++) {
            if (i == j) continue;
            if (data[i] == data[j]) n_dupls[i]++;
        }
    }
    for (int i=0; i<data.size(); i++)
        value += log(n_dupls[i]);
    value /= (float)data.size();
    *cost += value;
    
    return Status(true);
}