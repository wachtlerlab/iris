
#include <spectra.h>
#include <numeric>
#include <fstream>

#include <csv.h>

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



spectra spectra::from_csv(const std::string &path) {

    iris::csv_file csvfs(path);

    std::vector<uint16_t> lambda;

    typedef std::vector<float> fv_t;
    std::vector<fv_t> values;
    bool first_line = true;

    std::vector<std::string> header;

    for (const auto &line : csvfs) {

        if (first_line) {

            if (line.nfields() < 2) {
                throw std::invalid_argument("Invalid spectral data");
            }

            values.resize(line.nfields() - 1);
            first_line = false;

            header = line.fields();
            continue; //ignore the header

        } else {
            if (values.size() != line.nfields() - 1) {
                std::cerr << values.size() << " vs. " << line.nfields() << std::endl;
                throw std::invalid_argument("Invalid spectral data");
            }
        }

        uint16_t la = static_cast<uint16_t>(std::stoi(line.fields()[0]));
        lambda.push_back(la);
        
        for(size_t k = 0; k < values.size(); k++) {
            float tmp = std::stof(line.fields()[k + 1]);
            std::vector<float> &samples = values[k];
            samples.push_back(tmp);
        }
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