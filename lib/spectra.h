#ifndef IRIS_SPECTRA_H
#define IRIS_SPECTRA_H

#include <memory>
#include <iostream>
#include <string>
#include <vector>


namespace iris {

class spectrum {
public:
    spectrum() : wl_start(0), wl_step(0), values(), id() { }
    spectrum(uint16_t start, uint16_t step) : wl_start(start), wl_step(step) { }

    spectrum(spectrum &&o) : wl_start(o.wl_start), wl_step(o.wl_step),
                             values(std::move(o.values)), id(std::move(o.id)) {
    }

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

    float& operator[](size_t n) {
        return values[n];
    }

    const float& operator[](size_t n) const {
        return values[n];
    }

    const float *data() const {
        return values.data();
    }

    float *data() {
        return values.data();
    }

    void resize(size_t new_size) {
        values.resize(new_size);
    }

    size_t samples() const {
        return values.size();
    }

    std::string name() const {
        return id;
    }

    void name(const std::string &name) {
        id = name;
    }

    uint16_t start() const {
        return wl_start;
    }

    uint16_t step() const {
        return wl_step;
    }

    ~spectrum() {

    }

private:
    uint16_t wl_start;
    uint16_t wl_step;

    std::vector<float> values;

    std::string id;
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

    ssize_t find_spectrum(const std::string &id) const {

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

    spectrum operator[](size_t n) const {
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

    spectrum operator[](const std::string &name) const {
        ssize_t pos = find_spectrum(name);
        if (pos < 0) {
            return spectrum();
        }

        return this->operator[](static_cast<size_t>(pos));
    }

    void names(std::vector<std::string> data) {
        ids = std::move(data);
    }

    std::vector<std::string> names() const {
        return ids;
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



}

#endif