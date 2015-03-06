
#include <fit.h>

#include <cminpack-1/cminpack.h>

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
} // iris::