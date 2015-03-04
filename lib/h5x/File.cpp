
#include "File.hpp"

#include <sys/stat.h>

namespace h5x {


File File::open(const std::string &path, const std::string &mode) {

    if (mode.empty()) {
        throw std::invalid_argument("invalid open mode");
    }

    const char *p = mode.c_str(); //null-terminated

    File fd;
    if (p[0] == 'r') {
        unsigned int flags = p[1] == '+' ? H5F_ACC_RDWR : H5F_ACC_RDONLY;
        fd = H5Fopen(path.c_str(), flags, H5P_DEFAULT);
    } else if (p[0] == 'w') {
        fd = H5Fcreate(path.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    } else if (p[0] == 'a') {
        struct stat statbuf;
        int res = lstat(path.c_str(), &statbuf);
        if (res != 0) {
            if (errno == ENOENT) {
                fd = H5Fcreate(path.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            } else {
                throw std::runtime_error("lstate failed");
            }
        } else {
            fd = H5Fopen(path.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
        }
    } else {
        throw std::invalid_argument("invalid open mode");
    }

    return fd;
}
}