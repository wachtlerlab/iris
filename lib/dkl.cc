
#include <dkl.h>
#include <csv.h>

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#else
# ifdef HAVE_MKL
#  include <mkl_cblas.h>
#  include <mkl_lapacke.h>
# elif HAVE_ACML
#  include <acml.h>
# else
# include <cblas.h>
# include <lapacke.h>
# endif
#endif

#include <cmath>

namespace iris {

static void
mat_eye(int m, int n, double *A) {
    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            A[i*n+j] = i == j ? 1.0 : 0.0;
        }
    }
}

static void
mat_dump(int m, int n, const double *A) {
    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            std::cerr << A[i*n+j] << " ";
        }
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
}


static void
mat_trans(int M, int N, const double *A, double *res) {
    //totally lame matrix tranpose implementation

    const int K = M;
    double B[K*K];

    mat_eye(K, K, B);

    cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, N, M, K, 1.0, A, N, B, K, 0.0, res, N);
}

static int
do_dgesdd (char jobz, int m, int n, double *A, int lda, double *s, double *U, int ldu, double *Vt, int ldvt)
{
    int info;
#ifdef __APPLE__
    double  cwork;
    int     lwork;
    int     iwork[8*std::min(m ,n)];

    lwork = -1;

    dgesdd_(&jobz,
            &m, &n, A, &lda,
            s,
            U, &ldu,
            Vt, &ldvt,
            &cwork, &lwork, iwork, &info);

    if (info != 0) {
        std::cerr << "dgesdd_: error during workspace estimation: " << info << std::endl;
        return info;
    }

    lwork = (int) cwork;
    double work[lwork];

    dgesdd_(&jobz,
            &m, &n, A, &lda,
            s,
            U, &ldu,
            Vt, &ldvt,
            work, &lwork, iwork, &info);


#else
    info = LAPACKE_dgesdd (LAPACK_COL_MAJOR, jobz, m, n, A, lda, s,
			               U, ldu, Vt, ldvt);
#endif

    return info;
}

static int
mat_inv(int m, int n, const double  *A, double *Ai) {

    const int lda  = m;
    const int ldu  = m;
    const int ldvt = n;
    int info;

    const int k = std::min(m, n);
    double s[k];
    double U[ldu * m];
    double Vt[ldvt * n];
    double At[n*m];
    double Ax[n*m];

    //NB: we work in col-major because of dgesdd, so "transpose" A
    mat_trans(m, n, A, At);

    info = do_dgesdd('S', m, n, At, lda, s, U, ldu, Vt, ldvt);

    if (info != 0) {
        return -1;
    }

    double Sw[k*k];
    std::fill_n(Sw, k*k, 0.0);

    for(size_t i = 0; i < k; i++) {
        Sw[i*(k+1)] = 1.0/s[i];
    }

    double X[k*n];

    cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, n, n, n, 1.0, Vt, ldvt, Sw,   k, 0.0,  X, k);
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, m, n, m, 1.0,  X,    k,  U, ldu, 0.0, Ax, m);

    //see NB above
    mat_trans(n, m, Ax, Ai);

    return 0;
}

void dkl::parameter::print(std::ostream &os) const {

    auto pre = os.precision();
    auto w = os.width();

    os.precision(9);
    os.width(9);

    os << "# A₀ [A₀S, A₀M, A₀L]" << std::endl;
    os << A_zero[0] << ", " << A_zero[1] << ", " << A_zero[2];
    os << std::endl << std::endl;

    os << "# A [ArS, AgS, AbS; " << std::endl;
    os << "#    ArM, AgM, AbM; " << std::endl;
    os << "#    ArL, AgL, AbL ]" << std::endl;

    for(size_t i = 0; i < 9; i++) {
       os << A[i] << ((i+1) % 3 == 0 ? "\n" : ", ");
    }

    os << std::endl;

    os << "# ɣ [rˠ, gˠ, bˠ]" << std::endl;
    os << gamma[0] << ", " << gamma[1] << ", " << gamma[2] << std::endl;

    os.width(w);
    os.precision(pre);
}


dkl::parameter dkl::parameter::make_inverse(const dkl::parameter &p) {
    //invert A via SVD

    parameter p_inv;

    for(size_t i = 0; i < 3; i++) {
        p_inv.gamma[i] = 1.0/p.gamma[i];
    }

    mat_inv(3, 3, p.A,  p_inv.A);

    for(size_t i = 0; i < 3; i++) {
        p_inv.A_zero[i] = p.A_zero[i] * -1.0;
    }

    return p_inv;
}



dkl::parameter dkl::parameter::from_csv(const std::string &path) {
    enum class parse_state : int {
        A_ZERO, A_MAT1, A_MAT2, A_MAT3, GAMMA, FIN
    };

    csv_file fd(path);
    std::cout << path << std::endl;
    parameter res;

    parse_state state = parse_state::A_ZERO;
    for (const auto &rec : fd) {
        if (rec.is_comment() || rec.is_empty()) {
            continue;
        }

        if (rec.nfields() != 3) {
            throw std::runtime_error("Invalid CSV data for dkl::paramter");
        }

        double *dest;
        switch(state) {
            case parse_state::A_ZERO: dest = res.A_zero; break;
            case parse_state::A_MAT1: dest = res.A;      break;
            case parse_state::A_MAT2: dest = res.A + 3;  break;
            case parse_state::A_MAT3: dest = res.A + 6;  break;
            case parse_state::GAMMA:  dest = res.gamma;  break;
            case parse_state::FIN:
            std::cerr << "[W] extra data after parameter data" << std::endl;
                return res;
        }

        for (size_t k = 0; k < 3; k++) {
            dest[k] = rec.get_double(k);
        }

        state = static_cast<parse_state>(static_cast<int>(state) + 1);
    }

    return res;
}



dkl::dkl(const dkl::parameter  &init, const rgb &gray)
 : ref_gray(gray), params(init) {
    params_sml2rgb = params.invert();
}

rgb dkl::sml2rgb(const sml &input) const {
    double x[3];
    for(size_t i = 0; i < 3; i++) {
        x[i] = input.raw[i] + params_sml2rgb.A_zero[i];
    }

    double c[3];

    const int M = 3;
    const int N = 3;

    const double *A = params_sml2rgb.A;

    cblas_dgemv(CblasRowMajor, CblasNoTrans, M, N, 1.0, A, M, x, 1, 0.0, c, 1);

    rgb res;
    for(size_t i = 0; i < 3; i++) {
        res.raw[i] = static_cast<float>(std::pow(c[i], params_sml2rgb.gamma[i])) / 255.0f;
    }

    return res;
}

sml dkl::rgb2sml(const rgb &input) const{
    double x[3];

    for(size_t i = 0; i < 3; i++) {
        x[i] = std::pow(input.raw[i]*255.0, params.gamma[i]);
    }

    double c[3];
    memcpy(c, params.A_zero, sizeof(c));

    // Y ← αAX + βY
    const int M = 3;
    const int N = 3;

    const double *A = params.A;

    cblas_dgemv(CblasRowMajor, CblasNoTrans, M, N, 1.0, A, M, x, 1, 1.0, c, 1);
    return sml(c);
}

double dist(double a, double b, bool euclidean=true) {
    const double r = a/b;

    if (euclidean) {
        return std::sqrt(1 + std::pow(r, 2.0));
    } else {
        return 1 + r;
    }
}

rgb dkl::iso_lum(double phi, double c) {
    sml t = rgb2sml(ref_gray);

    bool e = false; //do euclidean

    const double p_sin = std::sin(phi);
    const double p_cos = std::cos(phi);

    t.s = t.s * (1.0 + 3.0 * c * p_sin);
    t.m = t.m * (1.0 - (c/dist(t.m, t.l, e))*p_cos);
    t.l = t.l * (1.0 + (c/dist(t.l, t.m, e))*p_cos);

    return sml2rgb(t);
}
}