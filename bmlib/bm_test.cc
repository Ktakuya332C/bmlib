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
    check_float(node.value, 0.5);
    check(bm.pparams->get_edge("edge1", &edge).ok);
    check_float(edge.value, 1.0);
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
    check_stoc_float(node.value, 0.909178);
    check(g.get_edge("edge2", &edge).ok);
    check_stoc_float(edge.value, 0.909178 * 0.803636);
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
    check_stoc_float(node.value, 0.650244);
    check(g.get_edge("edge2", &edge).ok);
    check_stoc_float(edge.value, 0.475366);
}

void test_reconst_cost(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    std::vector<std::vector<float> > data(2, std::vector<float>(1, 1.0));
    float val;
    check(bm.reconst_cost(data, &val, 10).ok);
    check_float(val, 0.218608);
}

void test_part_func(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    float val;
    check(bm.part_func_ais(&val, 10000, 10).ok);
    check_stoc_float(val, 4.035266);
    check(bm.part_func_det(&val).ok);
    check_float(val, 4.035266);
    
    Graph g;
    g.copy_from((*bm.pparams));
    check(g.add_value_to_node("node1", -0.1).ok);
    check(g.add_value_to_node("node2", 0.1).ok);
    check(g.add_value_to_edge("edge1", 0.1).ok);
    
    check(bm.part_func_imp(g, 4.116641, &val, 100).ok);
    check_stoc_float(val, 4.035266);
}

void test_kl(void) {
    BM bm(3, 2);
    check(bm.add_vis_node("node1", 0, 0.5).ok);
    check(bm.add_hid_node("node2", 0.5).ok);
    check(bm.add_hid_node("node3", 0.5).ok);
    check(bm.add_bm_edge("edge1", "node1", "node2", 1.0).ok);
    check(bm.add_bm_edge("edge2", "node2", "node3", 1.0).ok);
    
    float val;
    std::vector<std::vector<float> > data(2, std::vector<float>(1, 1.0));
    check(bm.kl_det(data, &val).ok);
    check_float(val, 0.231462);
}

int main() {
    test_alloc();
    test_adds();
    test_cor_cdby_data_stoc();
    test_cor_stoc();
    test_reconst_cost();
    test_part_func();
    test_kl();
}