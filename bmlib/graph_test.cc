#include "graph.h"
#include "status.h"
#include "test_utils.h"

void test_cntd_to(void) {
    Node node("node1");
    
    node.cntd_to(0);
    check(node.n_cntd_edges == 1);
    node.cntd_to(1);
    check(node.n_cntd_edges == 2);
    
    node.dis_cntd_to(0);
    check(node.n_cntd_edges == 1);
    node.dis_cntd_to(1);
    check(node.n_cntd_edges == 0);
}

void test_get_other_node(void) {
    Edge edge1("edge1", 0, 1, 0.6);
    check(edge1.another_node_idx(0) == 1);
}

void test_add_node(void) {
    Graph g(2, 0);
    check(g.add_node("node1", 0, 0.5).ok);
    check(!g.add_node("node1", 1, 0.9).ok);
}

void test_add_edge(void) {
    Graph g(2, 2);
    check(g.add_node("node1", 0, 0.5).ok);
    check(g.add_node("node2", 1, 0.5).ok);
    check(g.add_edge("edge1", "node1", "node2", 0.5).ok);
    check(!g.add_edge("edge2", "node1", "node3", 0.5).ok);
    check(!g.add_edge("edge1", "node1", "node2", 0.5).ok);
}

void test_remove_edge(void) {
    Graph g(3, 3);
    check(g.add_node("node1", 0, 0.5).ok);
    check(g.add_node("node2", 1, 0.5).ok);
    check(g.add_node("node3", 2, 0.5).ok);
    check(g.add_edge("edge1", "node1", "node2", 0.5).ok);
    check(g.add_edge("edge2", "node1", "node3", 0.5).ok);
    
    check(!g.remove_edge("edge3").ok);
    check(g.remove_edge("edge1").ok);
    check(g.n_edges == 1);
    check(g.n_nodes == 3);
    
    std::vector<std::string> node_names;
    check(g.get_nodes_cntd_to_edge("edge2", &node_names).ok);
    check(node_names.size() == 2);
    check((node_names[0] == "node1" && node_names[1] == "node3") ||
               (node_names[1] == "node3" && node_names[0] == "node1"));
    
    std::vector<std::string> edge_names;
    check(g.get_edges_cntd_to_node("node1", &edge_names).ok);
    check(edge_names.size() == 1);
    check(edge_names[0] == "edge2");
}

void test_remove_node(void) {
    Graph g(5, 2);
    check(g.add_node("node1", 0, 0.5).ok);
    check(g.add_node("node2", 1, 0.5).ok);
    check(g.add_node("node3", -1, 0.5).ok);
    check(g.add_edge("edge1", "node1", "node2", 0.5).ok);
    check(g.add_edge("edge2", "node1", "node3", 0.5).ok);
    
    check(!g.remove_node("node2").ok);
    check(g.remove_edge("edge1").ok);
    check(g.remove_node("node2").ok);
    check(g.n_nodes == 2);
    
    std::vector<std::string> node_names;
    check(g.get_nodes_cntd_to_edge("edge2", &node_names).ok);
    check(node_names.size() == 2);
    check((node_names[0] == "node1" && node_names[1] == "node3") ||
               (node_names[1] == "node3" && node_names[0] == "node1"));
    
    std::vector<std::string> edge_names;
    check(g.get_edges_cntd_to_node("node1", &edge_names).ok);
    check(edge_names.size() == 1);
    check(edge_names[0] == "edge2");
}

void test_get(void) {
    Graph g(5, 2);
    check(g.add_node("node1", 0, 1.0).ok);
    check(g.add_node("node2", 1, 2.0).ok);
    check(g.add_node("node3", 2, 3.0).ok);
    check(g.add_edge("edge1", "node1", "node2", 4.0).ok);
    
    Node node; Edge edge;
    check(g.get_node("node1", &node).ok);
    check(node.input_idx == 0);
    check(node.n_cntd_edges == 1);
    check(g.get_node("node2", &node).ok);
    check_float(node.value, 2.0);
    check(g.get_edge("edge1", &edge).ok);
    check_float(edge.value, 4.0);
}

void test_get_cntd(void) {
    Graph g(5, 2);
    check(g.add_node("node1", 0, 1.0).ok);
    check(g.add_node("node2", 1, 2.0).ok);
    check(g.add_node("node3", 2, 3.0).ok);
    check(g.add_edge("edge1", "node1", "node2", 4.0).ok);
    check(g.add_edge("edge2", "node2", "node3", 4.0).ok);
    
    std::vector<std::string> node_names;
    check(g.get_nodes_cntd_to_edge("edge1", &node_names).ok);
    check(node_names.size() == 2);
    check((node_names[0] == "node1" && node_names[1] == "node2") ||
               (node_names[1] == "node2" && node_names[0] == "node1"));
    
    std::vector<std::string> edge_names;
    check(g.get_edges_cntd_to_node("node1", &edge_names).ok);
    check(edge_names.size() == 1);
    check(edge_names[0] == "edge1");
}

void test_copy(void) {
    Graph g(5, 2);
    check(g.add_node("node1", 0, 1.0).ok);
    check(g.add_node("node2", 1, 2.0).ok);
    check(g.add_node("node3", 2, 3.0).ok);
    check(g.add_edge("edge1", "node1", "node2", 4.0).ok);
    
    Graph h(0, 0); Node node; Edge edge;
    h.copy_from(g);
    
    check(h.get_node("node1", &node).ok);
    check(node.input_idx == 0);
    check_float(node.value, 1.0);
    check(node.n_cntd_edges == 1);
    check(h.get_edge("edge1", &edge).ok);
    check_float(edge.value, 4.0);
}

int main() {
    test_cntd_to();
    test_get_other_node();
    test_add_node();
    test_add_edge();
    test_remove_edge();
    test_remove_node();
    test_get();
    test_get_cntd();
    test_copy();
}