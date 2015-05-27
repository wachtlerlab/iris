#ifndef IRIS_SCENE_H
#define IRIS_SCENE_H

#include <iris.h>

#include <glue/basic.h>
#include <glue/window.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>
#include <glue/text.h>

namespace iris {
namespace scene {

//maybe, maybe this will be some simple 2d scene graph
// for now this is a collection of dumb elements that
// draw themselves on the screen without being concerned
// about performance

class rectangle {
public:
    rectangle() {}
    rectangle(const glue::rect &r, const glue::color::rgba &paint);

    void draw(glm::mat4 vp);
    void init();

    void fg_color(const glue::color::rgba &color);

private:
    glue::rect rect;
    glue::color::rgba paint;

    //gl
    glue::shader vs;
    glue::shader fs;

    glue::program prg;

    glue::buffer bb;
    glue::vertex_array va;
};

class label {
public:
    label() { }
    label(const glue::tf_font &font, const std::string &str, size_t font_size);

    void init();
    void draw(const glm::mat4 &vp);

    glue::extent size() const {
        return mysize;
    }

private:
    glue::tf_font font;
    std::string str;
    size_t fsize;

    glue::program prg;
    glue::buffer vbuffer;
    glue::vertex_array varray;

    GLsizei ntriag;
    glue::extent mysize;
};

} //iris::scene::
} //iris::

#endif //IRIS_SCENE_H
