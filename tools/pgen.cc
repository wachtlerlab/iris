
#include <glue/colors.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>

namespace gl = glue;

void print_color(const gl::color::rgb &c) {
    std::cout << std::fixed<< std::setprecision(5);
    std::cout << c.r << ",";
    std::cout << c.g << ",";
    std::cout << c.b << std::endl;
    std::cout << std::defaultfloat;
}

void print_color_hex(const gl::color::rgb &c) {

    int r = static_cast<int>(c.r * 255.0f);
    int g = static_cast<int>(c.g * 255.0f);
    int b = static_cast<int>(c.b * 255.0f);

    std::cout << std::hex << std::setw(2) << std::setfill('0') << r;
    std::cout << std::hex << std::setw(2) << std::setfill('0') << g;
    std::cout << std::hex << std::setw(2) << std::setfill('0') << b;
}

std::vector<gl::color::rgb> make_stim(const std::vector<float> &steps) {
    std::vector<gl::color::rgb> res;

    const size_t n_steps = steps.size();

    for (size_t i = 0; i < n_steps; i++) {
        const auto v = steps[i];
        gl::color::rgb c = {v, 0.0f, 0.f};

        res.push_back(c);

        std::swap(c.data[0], c.data[1]);
        res.push_back(c);

        std::swap(c.data[1], c.data[2]);
        res.push_back(c);

        c.r = c.g = c.b = v;
        res.push_back(c);
    }

    return res;
}

std::vector<float> make_steps(size_t n) {
    std::vector<float> steps(n);

    float step = 1.0f / n;
    float cur = 0.0f;

    std::generate(steps.begin(), steps.end(), [&]{ cur += step; return cur; });

    return steps;
}

int main(int argc, char **argv) {

    std::vector<float> steps = make_steps(16);
    std::vector<gl::color::rgb> colors = make_stim(steps);

    for (const auto &c : colors) {
        print_color_hex(c);
        std::cout << ": ";
        print_color(c);
    }

    return 0;
}