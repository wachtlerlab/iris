#ifndef IRIS_DKL_H
#define IRIS_DKL_H

#include <vector>
#include <rgb.h>

namespace iris {

struct sml {

    double s;
    double m;
    double l;

    sml() : s(), m(), l() { }
    sml(double s, double m, double l) : s(s), m(m), l(l) { }
    explicit sml(double d[3]) : s(d[0]), m(d[1]), l(d[2]) { }
    explicit sml(float d[3]) : s(d[0]), m(d[1]), l(d[2]) { }


    inline const double & operator[](const size_t n) const {
        switch (n) {
            case 0: return s;
            case 1: return m;
            case 2: return l;
            default: throw std::out_of_range("OOB access");
        }
    }

    inline double & operator[](const size_t n) {
        const double &cr = const_cast<const sml *>(this)->operator[](n);
        return const_cast<double &>(cr);
    }
};

class dkl {
public:

    struct parameter {
        double A_zero[3];
        double A[9];
        double gamma[3];

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

    rgb iso_lum(double phi, double c) const;

    rgb reference_gray() const {
        return ref_gray;
    }

    void reference_gray(const rgb &ref) {
        ref_gray = ref;
    }

    void iso_slant(double delta_lumen, double phase) {
        iso_dl = delta_lumen;
        iso_phi = phase;
    }

private:
    rgb       ref_gray;
    parameter params;
    parameter params_rgb2sml;
    parameter params_sml2rgb;

    double iso_dl;
    double iso_phi;
};

} //iris::


#endif