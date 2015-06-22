#ifndef IRIS_MISC_H
#define IRIS_MISC_H

#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>

namespace iris {

template<typename T>
std::vector<T> linspace(T start, T stop, size_t n) {
    T dist = stop - start;
    T step = dist / n;

    std::vector<T> res(n);
    std::iota(res.begin(), res.end(), T(0));
    std::transform(res.begin(), res.end(), res.begin(), [start, step](const T v) {
        return start + v*step;
    });

    return res;
}

template<typename T>
std::vector<T> repvec(const std::vector<T> &input, size_t n) {
    std::vector<T> res(n*input.size());

    for(size_t i = 0; i < n; i++) {
        auto output_iter = res.begin();
        std::advance(output_iter, input.size()*i);
        std::copy(input.cbegin(), input.cend(), output_iter);
    }

    return res;
}


template<typename Iterator, typename URNG>
void block_shuffle(Iterator first, Iterator last, size_t blocksize, URNG &&g) {
    typename std::iterator_traits<Iterator>::difference_type n;

    n = last - first;
    size_t nblocks = n / blocksize;
    //todo: handle divison with reminder

    for (size_t i = 0; i < nblocks; i++) {
        Iterator start = first + i * blocksize;
        Iterator finish = start + blocksize;
        std::shuffle(start, finish, std::forward<URNG>(g));
    }
}


template<typename T>
T visual_angle_to_size(T angle, T distance) {
    T agl_rad = angle / T(180) * T(M_PI);
    return T(2) * distance * std::tan(agl_rad/T(2));
}

std::string make_timestamp();

}

#endif
