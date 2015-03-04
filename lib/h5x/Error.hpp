// Copyright © 2015 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#ifndef H5X_EXCEPTION_H
#define H5X_EXCEPTION_H

#include <hdf5.h>

#include <stdexcept>
#include <string>

namespace h5x {

class H5Exception : public std::exception {
public:
    H5Exception(const std::string &message)
            : msg(message) {

    }

    const char *what() const noexcept {
        return msg.c_str();
    }

private:
    std::string msg;
};


class H5Error : public H5Exception {
public:
    H5Error(herr_t err, const std::string &msg)
    : H5Exception(msg), error(err) {
    }

    static void check(herr_t result, const std::string &msg_if_fail) {
        if (result < 0) {
            throw H5Error(result, msg_if_fail);
        }
    }

private:
    herr_t      error;
};

namespace check {

template<typename T>
inline typename std::enable_if<! std::is_same<T, size_t>::value, size_t>::type
fits_in_size_t(T size, const std::string &msg_if_fail) {
    if (size > std::numeric_limits<size_t>::max()) {
        throw std::out_of_range(msg_if_fail);
    }
    return static_cast<size_t>(size);
}

template<typename T>
inline typename std::enable_if<std::is_same<T, size_t>::value, size_t>::type
fits_in_size_t(T size, const std::string &msg_if_fail) {
    return size;
}

} // nix::check::


} // h5x::


#endif