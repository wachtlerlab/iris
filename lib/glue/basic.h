
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


struct rect : public point, extent {
    rect() : point(0.f, 0.f), extent(0.f, 0.f) {}
    rect(float x, float y, float width, float height)
            : point(x, y), extent(width, height) {}
    rect(int x, int y, int width, int height)
            : point(1.f*x, 1.f*y), extent(1.f*width, 1.f*height) { }

};

void glue_start();

}

#endif
