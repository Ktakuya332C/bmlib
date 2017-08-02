#include "graph.h"
#include <cstring>

Node::Node(void) {
    std::strcpy(name, "");
    input_idx = -1;
    value = 0.0;
    
    n_cntd_edges = 0;
}

Node::Node(std::string in_name) {
    std::strcpy(name, in_name.c_str());
    input_idx = -1;
    value = 0.0;
    
    n_cntd_edges = 0;
}

Node::Node(std::string in_name, short in_idx, float in_val) {
    std::strcpy(name, in_name.c_str());
    input_idx = in_idx;
    value = in_val;
    
    n_cntd_edges = 0;
}

void Node::cntd_to(int edge_idx) {
    edge_idxs[n_cntd_edges] = edge_idx;
    n_cntd_edges++;
}

void Node::dis_cntd_to(int edge_idx) {
    for (int i=0; i<n_cntd_edges; i++) {
        if (edge_idxs[i] == edge_idx) {
            std::memcpy(edge_idxs+i, edge_idxs+i+1, sizeof(int)*(n_cntd_edges - i - 1));
            n_cntd_edges--;
        }
    }
}

Edge::Edge(void) {
    std::strcpy(name, "");
    std::strcpy(name, "");
    value = 0.0;
    node1_idx = -1;
    node2_idx = -1;
}

Edge::Edge(std::string in_name, float in_val) {
    std::strcpy(name, in_name.c_str());
    value = in_val;
    node1_idx = -1;
    node2_idx = -1;
}

Edge::Edge(std::string in_name, int in_nd1_idx, int in_nd2_idx, float in_val) {
    std::strcpy(name, in_name.c_str());
    value = in_val;
    node1_idx = in_nd1_idx;
    node2_idx = in_nd2_idx;
}

int Edge::another_node_idx(int node_idx) {
    if (node_idx == node1_idx) {
        return node2_idx;
    } else if (node_idx == node2_idx) {
        return node1_idx;
    } else {
        return -1;
    }
}

Graph::Graph(void) {
    max_nodes = 0;
    max_edges = 0;
    n_nodes = 0;
    n_edges = 0;
    
    nodes = new Node[0];
    edges = new Edge[0];
}

Graph::Graph(int in_max_nodes, int in_max_edges) {
    max_nodes = in_max_nodes;
    max_edges = in_max_edges;
    
    nodes = new Node[max_nodes];
    n_nodes = 0;
    edges = new Edge[max_edges];
    n_edges = 0;
}

Graph::~Graph(void) {
    delete[] nodes;
    delete[] edges;
}

int Graph::count_cntd_edges(std::string name) {
    int node_idx = -1;
    for (int i=0; i<n_nodes; i++) {
        if (std::strcmp(nodes[i].name, name.c_str()) == 0) {
            node_idx = i;
        }
    }
    if (node_idx < 0) return -1;
    
    return nodes[node_idx].n_cntd_edges;
}

Status Graph::add_node(std::string name, short input_idx, float bias) {
    for (int i=0; i<n_nodes; i++) {
        if (std::strcmp(nodes[i].name, name.c_str()) == 0)
            return Status(false, "There already exists node named "+name);
    }
    if (n_nodes >= max_nodes) return Status(false, "There already is fulll of nodes");
    
    nodes[n_nodes] = Node(name, input_idx, bias);
    n_nodes++;
    
    return Status(true);
}

Status Graph::remove_node(std::string name) {
    for (int i=0; i<n_edges; i++) {
        if (std::strcmp(nodes[edges[i].node1_idx].name, name.c_str()) == 0 || 
            std::strcmp(nodes[edges[i].node2_idx].name, name.c_str()) == 0) {
            return Status(false, "There still exists edges connecting the node named "+name);
        }
    }
    
    int idx = -1;
    for (int i=0; i<n_nodes; i++) {
        if (std::strcmp(nodes[i].name, name.c_str()) == 0) idx = i;
    }
    if (idx < 0) return Status(false, "There exits no node named "+name);
    
    std::memcpy(nodes+idx, nodes+idx+1, sizeof(Node)*(n_nodes - idx - 1));
    n_nodes--;
    
    for (int i=0; i<n_edges; i++) {
        if (edges[i].node1_idx > idx) edges[i].node1_idx--;
    }
    
    return Status(true);
}

Status Graph::add_edge(std::string name, std::string nd_name1, std::string nd_name2, float weight) {
    return add_edge_fast(name, nd_name1, nd_name2, weight, false);
}

Status Graph::add_edge_fast(std::string name, std::string nd_name1, std::string nd_name2, float weight, bool fast) {
    int node1_idx = -1;
    int node2_idx = -1;
    for (int i=0; i<n_nodes; i++) {
        if (std::strcmp(nodes[i].name, nd_name1.c_str()) == 0) node1_idx = i;
        if (std::strcmp(nodes[i].name, nd_name2.c_str()) == 0) node2_idx = i;
    }
    
    if (node1_idx < 0) return Status(false, "Node "+nd_name1+" not found");
    if (node2_idx < 0) return Status(false, "Node "+nd_name2+" not found");
    if (n_edges >= max_edges) return Status(false, "There already is full of edges");
    
    if ( !fast ) {
        for (int i=0; i<n_edges; i++) {
            if (std::strcmp(edges[i].name, name.c_str()) == 0)
                return Status(false, "There already exists edge named "+name);
        }
    }
    
    edges[n_edges] = Edge(name, node1_idx, node2_idx, weight);
    nodes[node1_idx].cntd_to(n_edges);
    nodes[node2_idx].cntd_to(n_edges);
    n_edges++;
    
    return Status(true);
}

Status Graph::remove_edge(std::string name) {
    int idx = -1;
    for (int i=0; i<n_edges; i++)
        if (std::strcmp(edges[i].name, name.c_str()) == 0) idx = i;
    if (idx < 0) return Status(false, "There exits no edge named "+name);
    
    std::memcpy(edges+idx, edges+idx+1, sizeof(Edge)*(n_edges - idx - 1));
    n_edges--;
    
    for (int i=0; i<n_nodes; i++) {
        for (int j=0; j<nodes[i].n_cntd_edges; j++) {
            if (nodes[i].edge_idxs[j] > idx) nodes[i].edge_idxs[j]--;
        }
    }
        
    return Status(true);
}

Status Graph::get_node(std::string name, Node * pnode) {
    for (int i=0; i<n_nodes; i++) {
        if (std::strcmp(nodes[i].name, name.c_str()) == 0) {
            (*pnode) = nodes[i];
            return Status(true);
        }
    }
    return Status(false, "There is no node named "+name);
}

Status Graph::get_edge(std::string name, Edge *pedge) {
    for (int i=0; i<n_edges; i++) {
        if (std::strcmp(edges[i].name, name.c_str()) == 0) {
            (*pedge) = edges[i];
            return Status(true);
        }
    }
    return Status(false, "There is no edge named "+name);
}

void Graph::copy_from(Graph &g) {
    delete[] nodes;
    delete[] edges;
    
    nodes = new Node[g.max_nodes];
    edges = new Edge[g.max_edges];
    std::memcpy(nodes, g.nodes, sizeof(Node)*g.n_nodes);
    std::memcpy(edges, g.edges, sizeof(Edge)*g.n_edges);
    n_nodes = g.n_nodes;
    n_edges = g.n_edges;
}
