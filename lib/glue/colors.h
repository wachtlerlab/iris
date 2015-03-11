#ifndef GLUE_COLOR_H
#define GLUE_COLOR_H

#ifdef HAVE_IRIS
#include <rgb.h>
#endif

namespace glue {
namespace color {

struct rgb {

    rgb() : r(0.0f), g(0.0f), b(0.0f) {}
    rgb(float r, float g, float b) : r(r), g(g), b(b) {}

    union {
        struct {
            float r;
            float g;
            float b;
        };
        struct {
            float data[3];
        };
    };
};


struct rgba {

    rgba() : r(0.0f), g(0.0f), b(0.0f) {}
    rgba(float r, float g, float b) : rgba(r, g, b, 1.0f) {}
    rgba(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    rgba(const rgb &c) : r(c.r), g(c.g), b(c.b), a(1.0f) {}
    operator rgb() const { return rgb(r, g, b); }

    #ifdef HAVE_IRIS
    rgba(const iris::rgb &o) : r(o.r), g(o.g), b(o.b), a(1.0f) { }
    rgba& operator=(const iris::rgb &o) {
        r = o.r; g = o.g; b = o.b; a = 1.0f;
        return *this;
    }
    #endif

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