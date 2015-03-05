
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

struct spectrum {

    size_t samples() const {
        return values.size();
    }

    std::vector<float> values;

    uint16_t wl_start;
    uint16_t wl_step;

    std::string id;

    float operator*(const spectrum &other) {
        if (other.wl_start != wl_start || other.wl_step != wl_step ||
                other.values.size() != values.size()) {
            throw std::invalid_argument("Incompatible spectra");
        }

        double vsum = 0;
        for(size_t i = 0; i < values.size(); i++) {
            double m = values[i] * other.values[i];
            vsum += m;
        }

        return static_cast<float>(vsum);
    }

    spectrum operator+(const spectrum &other) {
        if (other.wl_start != wl_start || other.wl_step != wl_step ||
                other.values.size() != values.size()) {
            throw std::invalid_argument("Incompatible spectra");
        }

        spectrum res;
        res.wl_start = wl_start;
        res.wl_step = wl_step;
        res.id = id;
        res.values.resize(values.size());

        for(size_t i = 0; i < values.size(); i++) {
            res.values[i] = values[i] + other.values[i];
        }

        return res;
    }
};


class spectra {
public:

    spectra() : storage(nullptr), n_spectra(0), n_samples(0) {}
    spectra(size_t spectra, size_t samples, uint16_t start, uint16_t step)
            : storage(nullptr), n_spectra(spectra), n_samples(samples),
              wl_start(start), wl_step(step), ids() {
        allocate();
    }

    spectra(spectra &&o) : storage(o.storage), n_spectra(o.n_spectra), n_samples(o.n_samples),
                           wl_start(o.wl_start), wl_step(o.wl_step), ids(std::move(o.ids)) {
        o.storage = nullptr;
        o.n_spectra = 0;
        o.n_samples = 0;
    }

    ~spectra() {
        if (storage != nullptr) {
            std::allocator<float> al;
            al.deallocate(storage, n_spectra * n_samples);
        }
    }

    float *data() {
        return storage;
    }

    const float *data() const {
        return storage;
    }

    size_t num_spectra() const {
        return n_spectra;
    }

    size_t num_samples() const {
        return n_samples;
    }

    uint16_t lambda_start() const {
        return wl_start;
    }

    uint16_t lambda_step() const {
        return wl_step;
    }

    ssize_t find_spectrum(const std::string &id) {
        return -1;
    }

    spectrum operator[](size_t n) const {
        if (n > n_spectra) {
            throw std::out_of_range("spectrum requested is oor");
        }

        spectrum s;
        s.wl_start = wl_start;
        s.wl_step = wl_step;

        if (!ids.empty() && ids.size() >= n) {
            s.id = ids[n];
        }

        s.values.resize(n_samples);
        float *ptr = storage + (n * n_samples);
        std::memcpy(s.values.data(), ptr, sizeof(float) * n_samples);
        return s;
    }

private:
    void allocate() {
        size_t n = n_spectra * n_samples;
        if (n < 1) {
            storage = nullptr;
            return;
        }

        std::allocator<float> al;
        storage = al.allocate(n);
    }

private:
    float *storage;
    size_t n_spectra;
    size_t n_samples;

    uint16_t wl_start;
    uint16_t wl_step;

    std::vector<std::string> ids;
};

static void dump_sepctra(const spectra &spec) {
    for (size_t i = 0; i < spec.num_spectra(); i++) {
        spectrum s = spec[i];

        for (size_t k = 0; k < s.samples(); k++) {
            std::cerr << s.values[k] << ", ";
        }

        std::cerr << std::endl;
    }
}

spectra parse_csv(const std::string path) {
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

    while (i != s.cend()) {
        bool r = boost::spirit::qi::phrase_parse(i, s.cend(), g, boost::spirit::qi::blank, v);

        if (first_line) {

            if (v.size() < 2) {
                throw std::invalid_argument("Invalid spectral data");
            }

            values.resize(v.size() - 1);
            first_line = false;
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
            float tmp = std::stod(v[k+1]);
            std::vector<float> &samples = values[k];
            samples.push_back(tmp);
        }


        v.clear();
    }

    if (values.empty() || lambda.empty() || lambda.size() < 2) {
        //fixme, < 2
        return spectra();
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
    spectra sp(n_spectra, n_samples, wl_start, wl_step);

    for (size_t i = 0; i < values.size(); i++) {
        const std::vector<float> &vs = values[i];
        float *dest = sp.data() + i * n_samples;
        memcpy(dest, vs.data(), sizeof(float) * n_samples);
    }

    return sp;
}



int main(int argc, char **argv) {
    namespace po = boost::program_options;

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

    sp.read(h5x::TypeId::Float , sp_size, spec.data());
    dump_sepctra(spec);

    spectra cf = parse_csv(cones);
    dump_sepctra(cf);

    spectrum M = cf[1];
    spectrum L = cf[2];

    spectrum LM = L + M;

    spectrum wmax = spec[spec.num_spectra() - 1];

    float lum = wmax * LM;

    std::cout << lum << std::endl;

    fd.close();
    return 0;
}