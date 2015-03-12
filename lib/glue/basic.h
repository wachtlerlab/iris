
#ifndef GLUE_BASIC_H
#define GLUE_BASIC_H

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

namespace glue {

struct point {

    float x;
    float y;

    point() : x(0.0f), y(0.0f) {}
    point(float x, float y) : x(x), y(y) { }
};


struct extent {
    float width;
    float height;

    extent() : width(0.0f), height(0.0f) {}
    extent(float width, float height) : width(width), height(height) { }
    extent(int width, int height) : width(1.0f * width), height(1.0f * height) { }
};


struct rect {

    union {
        struct {
            point start;
            extent size;
        };
        struct {
            float x;
            float y;
            float width;
            float height;
        };
    };

    rect() : start(), size() {}
    rect(float x, float y, float width, float height)
            : start(x, y), size(width, height) {
    }


    bool is_inside(float x, float y) const {
        return x > start.x && x < start.x + size.width && y > start.y && y < start.y + size.height;
    }

};


}

#endif
