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

    spectrum operator*(const spectrum &other);
    spectrum operator+(const spectrum &other);

    float integrate() const;

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

    ssize_t find_spectrum(const std::string &id) const;

    spectrum operator[](size_t n) const;

    spectrum operator[](const std::string &name) const;

    void names(std::vector<std::string> data) {
        ids = std::move(data);
    }

    std::vector<std::string> names() const {
        return ids;
    }

private:
    void allocate();

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