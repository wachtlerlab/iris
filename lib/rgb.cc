#include "rgb.h"

#include <algorithm>
#include <utility>
#include <iomanip>

namespace iris {

std::tuple<int, int, int> rgb::as_int(float base) const {
    int a[3];
    std::transform(raw, raw + 3, a, [base](const float v){
        return static_cast<int>(v * base);
    });
    return std::make_tuple(a[0], a[1], a[2]);
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