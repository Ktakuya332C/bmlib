#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cassert>
#include <cstdlib>
#include "graph.h"
#include "bm.h"
#include "math_utils.h"

#define IMG_SIZE 784
#define N_HID 100
#define IN_FILE "input/t10k-images-idx3-ubyte"
#define N_TRAIN 1000
#define B_SIZE 20
#define N_ITER 10
#define N_ITER_SAMP 2
#define N_SAMP 20
#define LR 0.01

int reverse_int(int i) {
    unsigned char c1, c2, c3, c4;
    c1 = i & 255;
    c2 = (i>>8) & 255;
    c3 = (i>>16) & 255;
    c4 = (i>>24) & 255;
    return ((int)c1<<24)+((int)c2<<16)+((int)c3<<8)+c4;
}

void load_mnist(std::vector<std::vector<float> > *data) {
    std::ifstream fin(IN_FILE);
    if (!fin.is_open()) std::exit(-1);
    
    int n_imgs, n_rows, n_cols, n_data;
    fin.read((char*)&n_imgs, sizeof(n_imgs)); // To deal with magic number
    fin.read((char*)&n_imgs, sizeof(n_imgs));
    n_imgs = reverse_int(n_imgs);
    fin.read((char*)&n_rows, sizeof(n_rows));
    n_rows = reverse_int(n_rows);
    fin.read((char*)&n_cols, sizeof(n_cols));
    n_cols = reverse_int(n_cols);
    n_data = n_rows * n_cols;
    
    (*data).resize(n_imgs, std::vector<float>(n_data));
    
    unsigned char tmp;
    for (int img_idx=0; img_idx<n_imgs; img_idx++) {
        for (int data_idx=0; data_idx<n_data; data_idx++) {
            fin.read((char*)&tmp, sizeof(tmp));
            (*data)[img_idx][data_idx] = (float)tmp / 256.0;
        }
    }
    
    fin.close();
}

void check(Status s) {
    if (!s.ok) {
        std::cout << s.msg << std::endl;
        std::exit(-1);
    }
}

int main() {
    std::cout << "Load mnist" << std::endl;
    std::vector<std::vector<float> > data;
    load_mnist(&data);
    
    std::cout << "Construct RBM" << std::endl;
    std::normal_distribution<float> norm_dist(0.0, 1.0 / IMG_SIZE);
    BM bm(IMG_SIZE + N_HID, IMG_SIZE * N_HID);
    for (int i=0; i<IMG_SIZE; i++)
        check(bm.add_vis_node("vis_node"+std::to_string(i), i, 0.0));
    for (int j=0; j<N_HID; j++)
        check(bm.add_hid_node("hid_node"+std::to_string(j), 0.0));
    for (int i=0; i<IMG_SIZE; i++) {
        for (int j=0; j<N_HID; j++) {
            check(bm.add_bm_edge_fast("edge"+std::to_string(i)+"_"+std::to_string(j),
                                          "vis_node"+std::to_string(i), "hid_node"+std::to_string(j), norm_dist(gen)));
        }
    }
    
    std::cout << "Train RBM" << std::endl;
    std::uniform_int_distribution<int> int_dist(0, data.size()-1);
    Graph updates;
    std::vector<std::vector<float> > batch(B_SIZE);
    int batch_idx;
    float value;
    for (int k=0; k<N_TRAIN; k++) {
        for (int i=0; i<B_SIZE; i++) {
            batch_idx = int_dist(gen);
            batch[i] = data[batch_idx];
        }
        
        bm.updates_stoc(batch, &updates, N_ITER, N_ITER_SAMP, N_SAMP);
        check(bm.learn(updates, LR));
        
        check(bm.reconst_cost(batch, &value, N_ITER));
        std::cout << "Epoch " << k << " cost " <<  value << std::endl; 
    }

}
