#include "bm.h"
#include "test_utils.h"

void test_alloc(void) {
    BM bm(3, 2);
}

void test_adds(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.pparams->n_nodes == 1);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.pparams->n_nodes == 2);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.pparams->n_nodes == 3);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.pparams->n_edges == 1);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    check(bm.pparams->n_edges == 2);
    
    Node node; Edge edge;
    check(bm.pparams->get_node("node1", &node).ok);
    check(node.input_idx == 0);
    check_double(node.value, 0.5);
    check(bm.pparams->get_edge("edge1", &edge).ok);
    check_double(edge.value, 1.0);
}

void test_cor_cdby_data_stoc(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    std::vector<std::vector<float> > data(2, std::vector<float>(1, 1.0));
    Graph g;
    bm.cor_cdby_data_stoc(data, &g, 10);
    
    Node node; Edge edge;
    check(g.get_node("node2", &node).ok);
    check_stoc_double(node.value, 0.909178);
    check(g.get_edge("edge2", &edge).ok);
    check_stoc_double(edge.value, 0.909178 * 0.803636);
}

void test_cor_stoc(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.0).ok);
    check(bm.add_hid_node("node3", 0.0).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 0.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    Graph g;
    bm.cor_stoc(&g, 100, 1000);
    
    Node node; Edge edge;
    check(g.get_node("node2", &node).ok);
    check_stoc_double(node.value, 0.650244);
    check(g.get_edge("edge2", &edge).ok);
    check_stoc_double(edge.value, 0.475366);
}

void test_reconst_cost(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    std::vector<std::vector<float> > data(2, std::vector<float>(1, 1.0));
    check_double(bm.reconst_cost(data, 10), 0.218608);
}

int main() {
    test_alloc();
    test_adds();
    test_cor_cdby_data_stoc();
    test_cor_stoc();
    test_reconst_cost();
}