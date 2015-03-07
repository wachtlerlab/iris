#ifndef IRIS_FIT_H
#define IRIS_FIT_H

#include <vector>
#include <cmath>

namespace iris {

struct fitter {
    virtual bool operator()();

    //interface to be implemented
    virtual int eval(int m, int n, const double *p, double *fvec) const = 0;
    virtual int num_parameter() const = 0;
    virtual int num_variables() const = 0;

    virtual double *params() = 0;

    // optional interface
    virtual double tolerance() const {
        return 1.49012e-8;
    }

protected:
    int fit_info;
};


class gamma_fitter : public fitter {
public:

    gamma_fitter(const std::vector<double> x, const std::vector<double> y) : x(x), y(y) {
        res[0] = y[0];
        res[1] = 0.0003;
        res[2] = 2.2;

    }

    static double func(const double Azero, const double A, const double gamma, const double x) {
        return Azero + A * std::pow(x, gamma);
    }

    virtual int eval(int m, int n, const double *p, double *fvec) const;

    double Azero() const {
        return res[0];
    }

    double A() const {
        return res[1];
    }

    double gamma() const {
        return res[2];
    }


    virtual int num_parameter() const override {
        return 3;
    }

    virtual int num_variables() const override {
        return static_cast<int>(x.size());
    }

    virtual double *params() override {
        return res;
    }

private:
    std::vector<double> x;
    std::vector<double> y;
    double res[3];
};

class rgb2sml_fitter : public fitter {
public:
    rgb2sml_fitter(const std::vector<double> x, const std::vector<double> y) : x(x), y(y) {
        res[0] = res[1] = res[2] = 0.01;

        res[3] = res[6] = res[9] = 0.00005;
        res[4] = res[7] = res[10] = 0.00001;
        res[5] = res[8] = res[11] = 0.00001;

        res[12] = res[13] = res[14] = 0.9;
    }

    virtual int eval(int m, int n, const double *p, double *fvec) const;

    virtual int num_parameter() const override {
        return 15; // 3 * Ao (AoS, AoM, AoL), 3 * gamma (R, G, B), 3x3 A, Matrix(ArS, AgS..)
    }

    virtual int num_variables() const override {
        return static_cast<int>(x.size());
    }

    virtual double *params() {
        return res;
    }

    std::vector<double> x;
    std::vector<double> y;
    double res[15]; // see num_parameter
};

}

#endif