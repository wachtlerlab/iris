
#include <spectra.h>
#include <numeric>
#include <fstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_ascii.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

namespace iris {

// ***********
// spectrum

spectrum spectrum::operator*(const spectrum &other) {
    if (other.wl_start != wl_start || other.wl_step != wl_step ||
                other.values.size() != values.size()) {
            throw std::invalid_argument("Incompatible spectra");
    }

    spectrum res(wl_start, wl_step);
    res.resize(values.size());

    for(size_t i = 0; i < values.size(); i++) {
       res[i] = values[i] * other.values[i];
    }

    return res;
 }

spectrum spectrum::operator+(const spectrum &other) {
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

double spectrum::integrate() const {
    double res = std::accumulate(values.cbegin(), values.cend(), 0.0);
    res *= wl_step;
    return res;
}


// ***********
// spectra

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

spectra spectra::from_csv(const std::string &path) {
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

//****

ssize_t spectra::find_spectrum(const std::string &id) const {

    bool found = false;
    size_t i;
    for (i = 0; i < ids.size(); i++) {
        if (id == ids[i]) {
            found = true;
            break;
        }
    }

    return found ? i : -1;
}

spectrum spectra::operator[](size_t n) const {
    if (n > n_spectra) {
        throw std::out_of_range("spectrum requested is oor");
    }

    spectrum s(wl_start, wl_step);

    if (!ids.empty() && ids.size() >= n) {
        s.name(ids[n]);
    }

    s.resize(n_samples);
    float *ptr = storage + (n * n_samples);
    std::memcpy(s.data(), ptr, sizeof(float) * n_samples);
    return s;
}

spectrum spectra::operator[](const std::string &name) const {
    ssize_t pos = find_spectrum(name);
    if (pos < 0) {
        return spectrum();
    }

    return this->operator[](static_cast<size_t>(pos));
}

void spectra::allocate() {
    size_t n = n_spectra * n_samples;
    if (n < 1) {
        storage = nullptr;
        return;
    }

    std::allocator<float> al;
    storage = al.allocate(n);
}

} // iris::