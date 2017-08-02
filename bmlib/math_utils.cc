#include "math_utils.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> uni_dist(0.0, 1.0);

float samp(float x) {
    if (x > uni_dist(gen)) {
        return 1.0;
    } else {
        return 0.0;
    }
}

float sigmoid(float x) {
    return 1.0 / (1.0 + exp(-x));
}

float safe_log(float x) {
    if (x < 1e-32) {
        return -32.0;
    } else {
        return log(x);
    }
}
