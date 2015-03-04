#include "rgb.h"

namespace iris {



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
} //iris::