#ifndef IRIS_DKL_H
#define IRIS_DKL_H

#include <vector>
#include <rgb.h>

namespace iris {

struct sml {

    sml() : s(), m(), l() { }
    sml(double s, double m, double l) : s(s), m(m), l(l) { }
    explicit sml(double d[3]) : s(d[0]), m(d[1]), l(d[2]) { }
    explicit sml(float d[3]) : s(d[0]), m(d[1]), l(d[2]) { }

    union {
        struct {
            double s;
            double m;
            double l;
        };

        double raw[3];
    };
};

class dkl {
public:

    struct parameter {
        union {
            struct {
                double A_zero[3];
                double A[9];
                double gamma[3];
            };

            double raw[15];
        };

        void print(std::ostream &os) const;

        parameter invert() const {
            return make_inverse(*this);
        }

        static parameter make_inverse(const parameter &p);
        static parameter from_csv(const std::string &path);
    };

public:

    dkl(const parameter &init, const rgb &gray);

    rgb sml2rgb(const sml &input) const;
    sml rgb2sml(const rgb &input) const;

    rgb iso_lum(double phi, double c);

private:
    rgb       ref_gray;
    parameter params;
    parameter params_rgb2sml;
    parameter params_sml2rgb;
};

} //iris::


#endif