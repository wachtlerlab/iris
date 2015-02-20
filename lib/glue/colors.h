#ifndef GLUE_COLOR_H
#define GLUE_COLOR_H

namespace glue {
namespace color {

struct rgb {

    rgb(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}

    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        struct {
            float data[4];
        };
    };
};


} //glue::color::
} //glue::

#endif //include guard