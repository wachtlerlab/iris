#ifndef IRIS_RGB_H
#define IRIS_RGB_H

#include <vector>
#include <ostream>
#include <tuple>

#include <cstring>

namespace iris {

struct rgb {

    constexpr rgb() : r(0.0f), g(0.0f), b(0.0f) {}
    constexpr rgb(float r, float g, float b) : r(r), g(g), b(b) {}

    union {
        struct {
            float r;
            float g;
            float b;
        };

        float raw[3];
    };

    rgb& operator=(const rgb &o) {
        memcpy(raw, o.raw, sizeof(o.raw));
        return *this;
    }

    inline const float & operator[](const size_t n) const {
        switch (n) {
            case 0: return r;
            case 1: return g;
            case 2: return b;
            default: throw std::out_of_range("OOB access");
        }
    }

    inline float & operator[](const size_t n) {
        const float &cr = const_cast<const rgb *>(this)->operator[](n);
        return const_cast<float &>(cr);
    }

    std::tuple<int, int, int> as_int(float base = 255.0f) const;
    std::tuple<bool, bool, bool> as_bits() const;
    rgb clamp(uint8_t *report = nullptr, float lower=0.f, float upper=1.f) const;

    static std::vector<rgb> gen(const std::vector<rgb> &base, const std::vector<float> &steps);
    static std::vector<float> linspace(size_t n);

    static rgb from_hex(const std::string &hexstring);

    static constexpr rgb white() { return rgb(1.0f, 1.0f, 1.0f);}
    static constexpr rgb black() { return rgb(0.0f, 0.0f, 0.0f); }
    static constexpr rgb gray(float level = 0.5f) { return rgb(level, level, level); }
    static constexpr rgb red(float level = 1.0f) { return rgb(level, 0.0f, 0.0f); }
    static constexpr rgb green(float level = 1.0f) { return rgb(0.0f, level, 0.0f); }
    static constexpr rgb blue(float level = 1.0f) { return rgb(0.0f, 0.0f, level); }
    static constexpr rgb yellow(float level = 1.0f) { return rgb(level, level, 0.0f); }
    static constexpr rgb cyan(float level = 1.0f) { return rgb(0.0f, level, level); }
    static constexpr rgb magenta(float level = 1.0f) { return rgb(level, 0.0f, level); }

};


std::ostream& operator<<(std::ostream &os, const rgb &color);

}

#endif