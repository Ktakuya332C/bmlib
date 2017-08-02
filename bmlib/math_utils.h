#ifndef BMLIB_MATH_UTILS_H_
#define BMLIB_MATH_UTILS_H_

#include <random>
#include <cmath>

extern std::random_device rd;
extern std::mt19937 gen;
extern std::uniform_real_distribution<float> uni_dist;

float samp(float x);

float sigmoid(float x);

float safe_log(float x);

#endif // BMLIB_MATH_UTILS_H_