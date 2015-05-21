
#include <glue/text.h>

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

#include "fs.h"

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
    std::vector<double> B(K*K);

    mat_eye(K, K, B.data());

    cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, N, M, K, 1.0, A, N, B.data(), K, 0.0, res, N);
}

static int
do_dgesdd (char jobz, int m, int n, double *A, int lda, double *s, double *U, int ldu, double *Vt, int ldvt)
{
    int info;
#ifdef __APPLE__
    double  cwork;
    int     lwork;
    std::vector<int> iwork(8*std::min(m ,n));

    lwork = -1;

    dgesdd_(&jobz,
            &m, &n, A, &lda,
            s,
            U, &ldu,
            Vt, &ldvt,
            &cwork, &lwork, iwork.data(), &info);

    if (info != 0) {
        std::cerr << "dgesdd_: error during workspace estimation: " << info << std::endl;
        return info;
    }

    lwork = (int) cwork;
    std::vector<double> work(lwork);

    dgesdd_(&jobz,
            &m, &n, A, &lda,
            s,
            U, &ldu,
            Vt, &ldvt,
            work.data(), &lwork, iwork.data(), &info);


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

    std::vector<double> s(k);
    std::vector<double> U(ldu * m);
    std::vector<double> Vt(ldvt * n);
    std::vector<double> At(n*m);
    std::vector<double> Ax(n*m);

    //double s[k];
    //double U[ldu * m];
    //double Vt[ldvt * n];
    //double At[n*m];
    //double Ax[n*m];

    //NB: we work in col-major because of dgesdd, so "transpose" A
    mat_trans(m, n, A, At.data());

    info = do_dgesdd('S', m, n, At.data(), lda, s.data(), U.data(), ldu, Vt.data(), ldvt);

    if (info != 0) {
        return -1;
    }

    std::vector<double> Sw(k*k, 0.0);

    for(size_t i = 0; i < k; i++) {
        Sw[i*(k+1)] = 1.0/s[i];
    }

    std::vector<double> X(k*n);

    cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, n, n, n, 1.0, Vt.data(), ldvt, Sw.data(),   k, 0.0,  X.data(), k);
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, m, n, m, 1.0,  X.data(),    k,  U.data(), ldu, 0.0, Ax.data(), m);

    //see NB above
    mat_trans(n, m, Ax.data(), Ai);

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

dkl::parameter dkl::parameter::from_csv_data(const std::string &data) {
    typedef csv_iterator<std::vector<char>::const_iterator> csv_siterator;
    enum class parse_state : int {
        A_ZERO, A_MAT1, A_MAT2, A_MAT3, GAMMA, FIN
    };

    parameter res;
    parse_state state = parse_state::A_ZERO;

    std::vector<char> chars;
    std::u32string utf32 = glue::u8to32(data);
    std::vector<char32_t> ascii;
    std::copy_if(utf32.begin(), utf32.end(), std::back_inserter(ascii), [](const char32_t ch){
        return ch < 0x80;
    });

    std::transform(ascii.begin(), ascii.end(), std::back_inserter(chars), [](const char32_t ch){
        return static_cast<char>(ch);
    });

    for (auto iter = csv_siterator(chars.cbegin(), chars.cend(), ',');
         iter != csv_siterator();
         ++iter) {
        auto rec = *iter;

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

dkl::parameter dkl::parameter::from_csv(const std::string &path) {
    fs::file fd(path);
    std::string data = fd.read_all();
    return from_csv_data(data);
}



dkl::dkl(const dkl::parameter  &init, const rgb &gray)
 : ref_gray(gray), params(init), iso_dl(0.0) {
    params_sml2rgb = params.invert();
}

rgb dkl::sml2rgb(const sml &input) const {
    double x[3];
    for(size_t i = 0; i < 3; i++) {
        x[i] = input[i] + params_sml2rgb.A_zero[i];
    }

    double c[3];

    const int M = 3;
    const int N = 3;

    const double *A = params_sml2rgb.A;

    cblas_dgemv(CblasRowMajor, CblasNoTrans, M, N, 1.0, A, M, x, 1, 0.0, c, 1);

    rgb res;
    for(size_t i = 0; i < 3; i++) {
        res[i] = static_cast<float>(std::pow(c[i], params_sml2rgb.gamma[i])) / 255.0f;
    }

    return res;
}

sml dkl::rgb2sml(const rgb &input) const {
    double x[3];

    for(size_t i = 0; i < 3; i++) {
        x[i] = std::pow(input[i]*255.0, params.gamma[i]);
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

rgb dkl::iso_lum(double phi, double c) const {

    double ldelta = iso_dl *
            (std::cos(phi) * std::cos(iso_phi) +
             std::sin(phi) * std::sin(iso_phi));

    float g_level = ref_gray.r;
    g_level += static_cast<float>(ldelta);
    rgb ref = rgb::gray(g_level);
    sml t = rgb2sml(ref);

    bool e = false; //do euclidean

    const double p_sin = std::sin(phi);
    const double p_cos = std::cos(phi);

    t.s = t.s * (1.0 + 3.0 * c * p_sin);
    t.m = t.m * (1.0 - (c/dist(t.m, t.l, e))*p_cos);
    t.l = t.l * (1.0 + (c/dist(t.l, t.m, e))*p_cos);

    return sml2rgb(t);
}

} //iris::