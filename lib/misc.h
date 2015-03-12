#ifndef IRIS_MISC_H
#define IRIS_MISC_H

#include <algorithm>
#include <numeric>
#include <vector>

namespace iris {

template<typename T>
std::vector<T> linspace(T start, T stop, size_t n) {
    T dist = stop - start;
    T step = dist / n;

    std::vector<T> res(n);
    std::iota(res.begin(), res.end(), T(0));
    std::transform(res.begin(), res.end(), res.begin(), [start, step](const double v) {
        return start + v*step;
    });

    return res;
}


}

#endif
