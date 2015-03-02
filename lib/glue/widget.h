#ifndef GLUE_WIDGET_H
#define GLUE_WIDGET_H

#include <glue/basic.h>

#include <glm/matrix.hpp>
#include <memory>

namespace glue {

class frame {
public:
    frame() {}
    frame(glm::mat4 r) : vp(r) { }

    frame subframe(rect bounds) const;

private:
    glm::mat4 vp;
};


class widget {
public:
    typedef std::shared_ptr<widget> owner;
    typedef std::weak_ptr<widget> ptr;

    virtual void render();
    virtual void resized(frame fr);

private:
    glm::mat4 vp;
};

}


#endif