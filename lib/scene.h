#ifndef IRIS_SCENE_H
#define IRIS_SCENE_H

#include <iris.h>

#include <glue/basic.h>
#include <glue/window.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>

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


} //iris::scene::
} //iris::

#endif //IRIS_SCENE_H
