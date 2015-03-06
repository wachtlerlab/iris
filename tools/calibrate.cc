
#include <h5x/File.hpp>

#include <boost/program_options.hpp>

#include <memory>
#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_ascii.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <fstream>
#include <spectra.h>
#include <rgb.h>
#include <fit.h>

namespace csv {
namespace qi      = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii   = boost::spirit::ascii;

struct record {
    std::vector<std::string> fields;
};

template<typename Iter, typename Skip>
struct csv_grammar : qi::grammar<Iter, std::vector<std::string>(), Skip> {

    csv_grammar() : csv_grammar::base_type(rec) {
        using qi::alpha;
        using qi::lexeme;
        using ascii::char_;

        qstr %= lexeme['"' >> +(char_ - '"') >> '"'];
        str  %= +(char_ - (qi::eol | ',' | "\""));

        field = str | qstr;

        rec %= field >> *(',' >> field ) >> (qi::eol | qi::eoi);
    }

    qi::rule<Iter, std::string(), Skip> str;
    qi::rule<Iter, std::string(), Skip> qstr;
    qi::rule<Iter, std::string(), Skip> field;
    qi::rule<Iter, std::vector<std::string>(), Skip> rec;
};

};

static void dump_sepctra(const iris::spectra &spec) {

    std::vector<std::string> names = spec.names();
    if (!names.empty()) {
        for (const auto &name : names) {
            std::cerr << name << " ";
        }
        std::cerr << std::endl;
    }

    for (size_t i = 0; i < spec.num_spectra(); i++) {
        iris::spectrum s = spec[i];

        for (size_t k = 0; k < s.samples(); k++) {
            std::cerr << s[k] << ", ";
        }

        std::cerr << std::endl;
    }
}

struct lhist {

    lhist() : komma(0), tab(0) {}

    long komma;
    long tab;

    long& operator[](std::string::value_type v) {
        switch (v) {
            case ',': return komma;
            case '\t': return tab;
            default:
                throw std::invalid_argument("Baeh!");
        }
    }

    static lhist make(const std::string &str) {
        lhist h;
        for (auto delim : keys()) {
            long n = std::count(str.begin(), str.end(), delim);
            h[delim] = n;
        }

        return h;
    }

    static lhist make(std::vector<lhist> &hst) {
        lhist h;

        for (auto delim : keys()) {
            for (lhist &cur : hst) {
                h[delim] += cur[delim];
            }
        }

        return h;
    }

    static std::string keys() {
        return ",\t";
    }
};


std::vector<lhist> mk_line_histogram(const std::string &input) {
    std::istringstream stream(input);

    std::vector<lhist> histogram{};

    for (std::string line; std::getline(stream, line); ) {
        lhist h = lhist::make(line);
        histogram.push_back(std::move(h));
    }

    return histogram;
}

iris::spectra parse_csv(const std::string path) {
    std::ifstream in(path);
    in.unsetf(std::ios::skipws);

    typedef std::istreambuf_iterator<char> siter;
    std::string s((siter(in)), siter());

    csv::csv_grammar<std::string::const_iterator, boost::spirit::qi::blank_type> g;

    std::vector<std::string> v;
    auto i = s.cbegin();

    std::vector<uint16_t> lambda;

    typedef std::vector<float> fv_t;
    std::vector<fv_t> values;
    bool first_line = true;

    std::vector<std::string> header;

    while (i != s.cend()) {
        bool r = boost::spirit::qi::phrase_parse(i, s.cend(), g, boost::spirit::qi::blank, v);

        if (first_line) {

            if (v.size() < 2) {
                throw std::invalid_argument("Invalid spectral data");
            }

            values.resize(v.size() - 1);
            first_line = false;

            header = std::move(v);
            v.clear();
            continue; //ignore the header

        } else {
            if (values.size() != v.size() - 1) {
                std::cerr << values.size() << " vs. " << v.size() << std::endl;
                throw std::invalid_argument("Invalid spectral data");
            }
        }

        uint16_t la = static_cast<uint16_t>(std::stoi(v[0]));
        lambda.push_back(la);

        std::cout << "[";
        for (const auto &k : v) {
            std::cout << k << ", ";
        }

        std::cout << "]" << std::endl;

        for(size_t k = 0; k < values.size(); k++) {
            float tmp = std::stof(v[k + 1]);
            std::vector<float> &samples = values[k];
            samples.push_back(tmp);
        }

        v.clear();
    }

    if (values.empty() || lambda.empty() || lambda.size() < 2) {
        //fixme, < 2
        return iris::spectra();
    }

    int steps = lambda[1] - lambda[0];

    if (steps < 0) {
        throw std::invalid_argument("error in wavelength data");
    }

    for (size_t i = 1; (i+1) < lambda.size(); i++) {
        if (lambda[i+1] - lambda[i] != steps) {
            throw std::invalid_argument("irregular sampling is unsupported");
        }
    }

    uint16_t wl_start = lambda[0];
    uint16_t wl_step = static_cast<uint16_t>(steps);

    size_t n_samples = values[0].size();
    size_t n_spectra = values.size();
    iris::spectra sp(n_spectra, n_samples, wl_start, wl_step);

    for (size_t i = 0; i < values.size(); i++) {
        const std::vector<float> &vs = values[i];
        float *dest = sp.data() + i * n_samples;
        memcpy(dest, vs.data(), sizeof(float) * n_samples);
    }

    if(!header.empty()) {
        header.erase(header.begin());
        sp.names(header);
    }

    return sp;
}



int main(int argc, char **argv) {
    namespace po = boost::program_options;
    using namespace iris;

    std::string input;
    std::string cones;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("cone-fundamentals,c", po::value<std::string>(&cones)->required())
            ("input", po::value<std::string>(&input)->required());

    po::positional_options_description pos;
    pos.add("input", 1);
    pos.add("cone-fundamentals", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }

    h5x::File fd = h5x::File::open(input, "r");

    if (!fd.hasData("spectra") || !fd.hasData("patches")) {
        std::cerr << "File missing spectra or patches" << std::endl;
        return 1;
    }

    h5x::DataSet sp = fd.openData("spectra");
    h5x::DataSet ps = fd.openData("patches");

    h5x::NDSize sp_size = sp.size();
    h5x::NDSize ps_size = ps.size();

    std::cerr << sp_size << std::endl;
    std::cerr << ps_size << std::endl;

    spectra spec(sp_size[0], sp_size[1], 380, 4);

    sp.read(h5x::TypeId::Float, sp_size, spec.data());
    dump_sepctra(spec);

    spectra cf = parse_csv(cones);
    dump_sepctra(cf);

    std::vector<iris::rgb> stim(ps_size[0]);
    ps.read(h5x::TypeId::Float, ps_size, stim.data());

    std::vector<double> y;
    std::vector<double> x;

    for (size_t cone = 0; cone < 3; cone++) {
        spectrum cs = cf[cone];
        std::cerr << cs.name() << std::endl;

        for (size_t source = 0; source < 3; source++) {
            std::cerr << source << std::endl;

            for (size_t p = 0; p < stim.size(); p++) {
                iris::rgb kanon = stim[p];

                std::bitset<3> bs;
                for (size_t j = 0; j < 3; j++) {
                    auto jc = fpclassify(kanon.raw[j]);
                    bs.set(j, j == source ? jc != FP_ZERO : jc == FP_ZERO);
                }

                if (bs.all()) {
                    double l = (spec[p] * cs).integrate();
                    double v = kanon.raw[source] * 255.0;
                    x.push_back(v);
                    y.push_back(l);
                }
            }
        }
    }

    std::cerr << x.size() << " " << y.size() << std::endl;

    for (size_t p = 0; p < x.size(); p++) {
        std::cout << x[p] << ", " << y[p] << std::endl;
    }

    ConvFitter fitter(x, y);
    fitter();

    for(double p : fitter.res) {
        std::cout << p << std::endl;
    }

    fd.close();
    return 0;
}