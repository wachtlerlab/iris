#ifndef IRIS_FIT_H
#define IRIS_FIT_H

#include <vector>
#include <cmath>

namespace iris {

struct Optimizer {
    virtual int eval(int m, int n, const double *p, double *fvec) const = 0;
};


class GammaFitter : public Optimizer {
public:

    GammaFitter(const std::vector<double> x, const std::vector<double> y) : x(x), y(y) { }

    static double func(const double Azero, const double A, const double gamma, const double x) {
        return Azero + A * std::pow(x, gamma);
    }

    virtual int eval(int m, int n, const double *p, double *fvec) const;
    bool operator()();

    double Azero() const {
        return res[0];
    }

    double A() const {
        return res[1];
    }

    double gamma() const {
        return res[2];
    }

private:
    std::vector<double> x;
    std::vector<double> y;
    double res[3];
};

class rgb2sml_fitter : public Optimizer {
public:
    rgb2sml_fitter(const std::vector<double> x, const std::vector<double> y) : x(x), y(y) { }

    virtual int eval(int m, int n, const double *p, double *fvec) const;
    bool operator()();

    std::vector<double> x;
    std::vector<double> y;
    double res[3 + 3*3 + 3];
};

}

#endif