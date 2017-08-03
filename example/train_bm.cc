#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include "graph.h"
#include "bm.h"
#include "math_utils.h"

#define IMG_SIZE 8
#define N_HID 4
#define N_TRAIN 100
#define N_ITER 10
#define N_ITER_SAMP 2
#define N_SAMP 20
#define LR 0.01

void check(Status s) {
    if (!s.ok) {
        std::cout << s.msg << std::endl;
        std::exit(-1);
    }
}

int main() {
    std::cout << "Assign data" << std::endl;
    std::vector<std::vector<float> > data = {
        {1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0},
        {1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0},
    };
    
    std::cout << "Construct RBM" << std::endl;
    std::normal_distribution<float> norm_dist(0.0, 1.0 / (float)(IMG_SIZE+N_HID));
    BM bm(IMG_SIZE + N_HID, (IMG_SIZE +N_HID)*(IMG_SIZE +N_HID));
    for (int i=0; i<IMG_SIZE; i++)
        check(bm.add_vis_node("vis_node"+std::to_string(i), i, 0.0));
    for (int j=0; j<N_HID; j++)
        check(bm.add_hid_node("hid_node"+std::to_string(j), 0.0));
    for (int i=0; i<IMG_SIZE; i++) {
        for (int j=i+1; j<IMG_SIZE; j++) {
            check(bm.add_bm_edge_fast("vvedge"+std::to_string(i)+"_"+std::to_string(j),
                                          "vis_node"+std::to_string(i), "vis_node"+std::to_string(j), norm_dist(gen)));
        }
    }
    for (int i=0; i<N_HID; i++) {
        for (int j=i+1; j<N_HID; j++) {
            check(bm.add_bm_edge_fast("hhedge"+std::to_string(i)+"_"+std::to_string(j),
                                          "hid_node"+std::to_string(i), "hid_node"+std::to_string(j), norm_dist(gen)));
        }
    }
    for (int i=0; i<IMG_SIZE; i++) {
        for (int j=0; j<N_HID; j++) {
            check(bm.add_bm_edge_fast("vhedge"+std::to_string(i)+"_"+std::to_string(j),
                                          "vis_node"+std::to_string(i), "hid_node"+std::to_string(j), norm_dist(gen)));
        }
    }
    
    std::cout << "Train BM" << std::endl;
    Graph updates;
    for (int k=0; k<N_TRAIN; k++) {
        bm.updates_stoc(data, &updates, N_ITER, N_ITER_SAMP, N_SAMP);
        check(bm.learn(updates, LR));
        
        std::cout << "Epoch " << k << " cost " << bm.reconst_cost(data, N_ITER) << std::endl; 
    }

}
