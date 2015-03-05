
#include <h5x/File.hpp>

#include <boost/program_options.hpp>

#include <memory>
#include <iostream>
#include <string>


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

    spectrum operator[](size_t n) {
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



int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string input;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("input", po::value<std::string>(&input)->required());

    po::positional_options_description pos;
    pos.add("input", 1);

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

    for (size_t i = 0; i < spec.num_spectra(); i++) {
        spectrum s = spec[i];

        for (size_t k = 0; k < s.samples(); k++) {
            std::cerr << s.values[k] << ", ";
        }

        std::cerr << std::endl;
    }

    fd.close();
    return 0;
}