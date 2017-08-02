#ifndef BMLIB_BM_H_
#define BMLIB_BM_H_

#include "graph.h"
#include "status.h"

class BM {
    public:
        BM(int max_nodes, int max_edges);
        ~BM(void);
        
        Graph *pparams, *pacts;
        
        Status add_vis_node(std::string name, short input_idx, float bias);
        Status add_hid_node(std::string name, float bias);
        Status add_bm_edge(std::string name, std::string nd_name1, std::string nd_name2, float weight);
        Status add_bm_edge_fast(std::string name, std::string nd_name1, std::string nd_name2, float weight);
        
        Status learn(Graph &updates, float learning_rate);
        
        void cor_cdby_data_stoc(std::vector<std::vector<float> > &data, Graph *pg, int n_iter);
        void cor_stoc(Graph *pg, int n_iter, int n_samp);
        void updates_stoc(std::vector<std::vector<float> > &data, Graph *pg, int n_iter, int n_iter_samp, int n_samp);
        
        double reconst_cost(std::vector<std::vector<float> > &data, int n_iter);
        double part_func_ais(int n_step, int n_samp);
};
#endif // BMLIB_BM_H_