
#include <fit.h>

#include <cminpack-1/cminpack.h>
#include <iostream>

namespace iris {

static int call_opt(void *p,
                    int m,
                    int n,
                    const double *x,
                    double *fvec,
                    int iflag)
{
    fitter *opt = static_cast<fitter *>(p);
    return opt->eval(m, n, x, fvec);
}


bool fitter::operator()() {
    double tol = tolerance();
    const int m = num_variables();
    const int n = num_parameter();
    const int lwa = m*n+5*n+m;
    std::vector<int> iwa(n);
    std::vector<double> wa(lwa);
    std::vector<double> fvec(m);

    double *p = params();
    void *user_data = static_cast<void *>(this);
    fit_info = lmdif1(call_opt, user_data, m, n, p, fvec.data(), tol, iwa.data(), wa.data(), lwa);
    return fit_info == 1 || fit_info == 2 || fit_info == 3;
}

int gamma_fitter::eval(int m, int n, const double *p, double *fvec) const {
    if (n != 3 || m != x.size()) {
        throw std::invalid_argument("Invalid data passed to GF");
    }

    for (int i = 0; i < m; i++) {
        const double Azero = p[0];
        const double A = p[1];
        const double gamma = p[2];

        fvec[i] = y[i] - func(Azero, A, gamma, x[i]);
    }

    return 0;
}

int sin_fitter::eval(int m, int n, const double *p, double *fvec) const {

    double A = p[0];
    double phi = p[1];
    double offset = p[2];
    double f = fit_frequency ? p[3] : 1.0;

    for(int i = 0; i < x.size(); i++) {
        fvec[i] = y[i] - (offset + A * cos(f * x[i] - phi));
    }

    return 0;
}


int rgb2sml_fitter::eval(int m, int n, const double *p, double *fvec) const {

    const int N = m / (3*3);

    const double *A0 = p;
    const double *A = p + 3;
    const double *g = p + 3 + 3*3;

    for (int cone = 0; cone < 3; cone++) {
        for (int channel = 0; channel < 3; channel++) {
            for (int intensity = 0; intensity < N; intensity++) {
                int i = 3*N*cone + N*channel + intensity;
                double deviate = y[i] - (A0[cone] + A[3 * cone + channel] * std::pow(x[i], g[channel]));
                fvec[i] = deviate * 1.0/std::pow(y[i], we);
            }
        }
    }

    return 0;
}


} // iris::