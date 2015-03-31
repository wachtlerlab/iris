#ifndef GLUE_COLOR_H
#define GLUE_COLOR_H

#ifdef HAVE_IRIS
#include <rgb.h>
#endif

namespace glue {
namespace color {

struct rgba {

    rgba() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    rgba(float r, float g, float b) : rgba(r, g, b, 1.0f) {}
    rgba(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    #ifdef HAVE_IRIS
    rgba(const iris::rgb &o) : r(o.r), g(o.g), b(o.b), a(1.0f) { }
    rgba& operator=(const iris::rgb &o) {
        r = o.r; g = o.g; b = o.b; a = 1.0f;
        return *this;
    }
    #endif

    float r;
    float g;
    float b;
    float a;
};


} //glue::color::
} //glue::

#endif //include guard