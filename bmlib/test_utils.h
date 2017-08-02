#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <iostream>

#define check(success) _check(success, __func__, __LINE__)
void _check(bool success, std::string func_name, int line_num) {
    if (!success)
        std::cout << "Failed: " << func_name << " at line " << line_num << std::endl; 
}

#define check_double(pred, ans) check(_test_double(pred, ans, 1e-4))
#define check_stoc_double(pred, ans) check(_test_double(pred, ans, 0.1))
bool _test_double(double pred, double ans, double eps) {
    if ((ans - eps < pred) && (pred < ans + eps)) {
        return true;
    } else {
        return false;
    }
}

#endif // TEST_UTILS_H_