
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
    Optimizer *opt = static_cast<Optimizer *>(p);
    return opt->eval(m, n, x, fvec);
}


bool GammaFitter::operator()() {
    double tol = 1.49012e-8;
    const int m = static_cast<int>(x.size());
    const int n = 3;
    const int lwa = m*n+5*n+m;
    int iwa[n];
    double wa[lwa];
    double fvec[m];
    int info;

    res[0] = y[0];
    res[1] = 0.0003;
    res[2] = 2.2;

    void *user_data = static_cast<void *>(this);
    info = lmdif1(call_opt, user_data, m, n, res, fvec, tol, iwa, wa, lwa);


    return true;
}

int GammaFitter::eval(int m, int n, const double *p, double *fvec) const {
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


int ConvFitter::eval(int m, int n, const double *p, double *fvec) const {

    const int N = m / (3*3);

    const double *A0 = p;
    const double *A = p + 3;
    const double *g = p + 3 + 3*3;

    for (int cone = 0; cone < 3; cone++) {
        for (int channel = 0; channel < 3; channel++) {
            for (int intensity = 0; intensity < N; intensity++) {
                int i = 3*N*cone + N*channel + intensity;
                double deviate = y[i] - (A0[cone] + A[3 * cone + channel] * std::pow(x[i], g[channel]));
                fvec[i] = deviate * 1.0/std::pow(x[i], 1.1);
            }
        }
    }

    return 0;
}

bool ConvFitter::operator()() {
    double tol = 1.49012e-8;
    const int m = static_cast<int>(y.size());
    const int n = static_cast<int>(sizeof(res)/ sizeof(double));
    const int lwa = m*n+5*n+m;
    int iwa[n];
    double wa[lwa];
    double fvec[m];
    int info;

    res[0] = res[1] = res[2] = 0.01;

    res[3] = res[6] = res[9] = 0.00005;
    res[4] = res[7] = res[10] = 0.00001;
    res[5] = res[8] = res[11] = 0.00001;

    res[12] = res[13] = res[14] = 0.9;

    std::cerr << "m: " << m << ", n: " << n << std::endl;

    void *user_data = static_cast<void *>(this);
    info = lmdif1(call_opt, user_data, m, n, res, fvec, tol, iwa, wa, lwa);

    std::cerr << "fit-info: " << info << std::endl;

    return true;
}

} // iris::