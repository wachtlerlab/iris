#ifndef IRIS_RGB_H
#define IRIS_RGB_H

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

}

#endif