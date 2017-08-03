#ifndef BMLIB_GRAPH_H_
#define BMLIB_GRAPH_H_

#include <string>
#include <vector>
#include "status.h"

#define MAX_EDGES_PER_ONE_NODE 1000
#define MAX_NAME_LEN 16 

struct Edge;
struct Node {
    Node(void);
    Node(std::string in_name);
    Node(std::string in_name, short in_idx, float in_val);
    
    char name[MAX_NAME_LEN];
    short input_idx;
    float value;
    int edge_idxs[MAX_EDGES_PER_ONE_NODE];
    int n_cntd_edges;
    
    void cntd_to(int edge_idx);
    void dis_cntd_to(int edge_idx);
};

struct Edge {
    Edge(void);
    Edge(std::string in_name, float in_val);
    Edge(std::string in_name, int in_nd1_idx, int in_nd2_idx, float in_val);
    
    char name[MAX_NAME_LEN];
    float value;
    int node1_idx, node2_idx;
    
    int another_node_idx(int node_idx);
};

struct Graph {
    Graph(void);
    Graph(int in_max_nodes, int in_max_edges);
    ~Graph(void);

    int max_nodes, max_edges;
    Node *nodes;
    int n_nodes;
    Edge *edges;
    int n_edges;
    
    int count_cntd_edges(std::string name);
    
    Status add_node(std::string name, short input_idx, float value);
    Status remove_node(std::string name);
    Status add_edge(std::string name, std::string nd_name1, std::string nd_name2, float weight);
    Status add_edge_fast(std::string name, std::string nd_name1, std::string nd_name2, float weight, bool fast);
    Status remove_edge(std::string name);
    
    Status get_node(std::string name, Node *pnode);
    Status get_edge(std::string name, Edge *pedge);
    Status get_nodes_cntd_to_edge(std::string name, std::vector<std::string> *pnode_names);
    Status get_edges_cntd_to_node(std::string name, std::vector<std::string> *pedge_names);
    
    Status add_value_to_node(std::string name, float value);
    Status add_value_to_edge(std::string name, float value);
    void add_values_to_nodes(float value);
    void add_values_to_edges(float value);
    
    void copy_from(Graph &g);
};

#endif // BMLIB_GRAPH_H_