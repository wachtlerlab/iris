#ifndef IRIS_DKL_H
#define IRIS_DKL_H

#include <vector>
#include <rgb.h>

namespace iris {

struct sml {

    float s;
    float m;
    float l;

};

class dkl {
public:
    struct parameter {
        double A_zero[3];
        double A[3*3];
        double gamma[3];
    };

public:
    rgb sml2rgb(const sml &input) const;
    std::vector<rgb> sml2rgb(const std::vector<sml> &input) const;

    sml rgb2sml(const rgb &input) const;
    std::vector<sml> sml2rgb(const std::vector<sml> &input) const;

private:
    static paramter make_inverse(const parameter &p) const;

private:
    parameter params_rgb2sml;
    parameter params_sml2rgb;
};

} //iris::


#endif