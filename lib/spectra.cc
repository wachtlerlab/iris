
#include <spectra.h>

namespace iris {

// ***********
// spectrum

float spectrum::operator*(const spectrum &other) {
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

float spectrum::integrate() const {
    double res = std::accumulate(values.cbegin(), values.cend(), 0.0);
    res *= wl_step;
    return static_cast<float>(res);
}


// ***********
// spectra


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