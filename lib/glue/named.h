#ifndef GLUE_NAMED_H
#define GLUE_NAMED_H

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <cstddef>

namespace glue {

template<typename T>
class named {

public:
    named(std::nullptr_t) : ctrl(nullptr) { }

    explicit named(GLuint name) {
        if (name != 0) {
            ctrl = new control_block(name);
        }
    }

    named(const named &other) : ctrl(other.ctrl) {
        incref();
    }

    named &operator=(const named &other) {

        if (this == &other) {
            return *this;
        }

        decref();
        ctrl = other.ctrl;
        incref();

        return *this;
    }

    inline GLuint name() const {
        return ctrl ? ctrl->name : 0;
    }

    virtual ~named() {
        decref();
    }

    void reset() {
        decref();
        ctrl = new control_block(T::make_name());
    }

    explicit operator bool() {
        return ctrl != nullptr && ctrl->name != 0;
    }

private:

    int decref() {
        if (ctrl) {
            if (ctrl->decref() == 0) {
                T::delete_name(ctrl->name);
                delete ctrl;
                ctrl = nullptr;
                return 0;
            }

            return ctrl->refcount;
        }
        return 0;
    }

    int incref() {
        if (ctrl) {
            return ctrl->incref();
        }
        return 0;
    }

    struct control_block {
        control_block(GLuint name) : refcount(1), name(name) { }

        int incref() { return ++refcount; }
        int decref() { return --refcount; }

        int refcount;
        GLuint name;
    };

private:
    control_block *ctrl;
};



} //glue::

#endif