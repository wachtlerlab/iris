#include "rgb.h"

#include <algorithm>
#include <utility>
#include <iomanip>
#include <cmath>
#include <cfloat>


namespace iris {

std::tuple<int, int, int> rgb::as_int(float base) const {
    int a[3];
    std::transform(raw, raw + 3, a, [base](const float v){
        return static_cast<int>(v * base);
    });
    return std::make_tuple(a[0], a[1], a[2]);
}

std::tuple<bool, bool, bool> rgb::as_bits() const {
    bool a[3];
    std::transform(raw, raw + 3, a, [](const float v){
        switch (std::fpclassify(v)) {
            case FP_ZERO:
                return false;
            default:
                return true;
        }
    });

    return std::make_tuple(a[0], a[1], a[2]);
}


rgb rgb::clamp(uint8_t *report, float lower, float upper) const {
    rgb res;
    uint8_t f = 0;

    for(size_t i = 0; i < 3; i++) {

        if (raw[i] < lower || std::isnan(raw[i])) {
            res.raw[i] = lower;
            f = f | (uint8_t(1) << i);
        } else if (raw[i] > upper) {
            res.raw[i] = upper;
            f = f | (uint8_t(1) << i);
        } else {
            res.raw[i] = raw[i];
        }
    }

    if (report) {
        *report = f;
    }

    return res;
}

std::vector<rgb> rgb::gen(const std::vector<rgb> &base, const std::vector<float> &steps) {

    std::vector<rgb> result;
    result.reserve(base.size() * steps.size());

    for (const float step : steps) {
        for (rgb c : base) {
            std::replace_if(c.raw, c.raw + 3, [](const float v){ return v > 0.0f; }, step);
            result.push_back(c);
        }
    }

    return result;
}


std::vector<float> rgb::linspace(size_t n) {
    std::vector<float> steps(n);

    float step = 1.0f / n;
    float cur = 0.0f;

    std::generate(steps.begin(), steps.end(), [&]{ cur += step; return cur; });

    return steps;
}


rgb rgb::from_hex(const std::string &hexstring) {

    if (hexstring.size() != 6) {
        throw std::invalid_argument("RRGGBB format supported now");
    }

    iris::rgb color;
    for (size_t i = 0; i < 3; i++) {
        std::string sub = hexstring.substr(2*i, 2);
        size_t pos = 0;
        unsigned long c = std::stoul(sub, &pos, 16);
        if (pos == 0) {
            throw std::runtime_error("Error while parsing RRGGBB format");
        }

        color.raw[i] = c / 255.0f;
    }

    return iris::rgb();
}


std::ostream& operator<<(std::ostream &os, const rgb &color) {
    auto flags = os.flags();

    if ((flags & std::ostream::hex) != 0) {
        int r, g, b;
        std::tie(r, g, b) = color.as_int(255.0f);
        os << std::setw(2) << std::setfill('0') << std::fixed << r;
        os << std::setw(2) << std::setfill('0') << std::fixed << g;
        os << std::setw(2) << std::setfill('0') << std::fixed << b;
    } else {
        os << color.r << ", " << color.g << ", " << color.b;
    }

    return os;
}

} //iris::