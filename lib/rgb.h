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

};

}

#endif