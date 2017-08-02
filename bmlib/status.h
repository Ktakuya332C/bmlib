#ifndef BMLIB_STATUS_H_
#define BMLIB_STATUS_H_

#include <string>

struct Status {
    Status(bool res): ok(res), msg("") {}
    Status(bool res, std::string err_msg): ok(res), msg(err_msg) {}
    bool ok;
    std::string msg;
};

#endif // BMLIB_STATUS_H_