#ifndef GLUE_NAMED_H
#define GLUE_NAMED_H

#include <glue/basic.h>
#include <cstddef>
#include <utility>

namespace glue {

template<typename T, typename U = void*>
class named {

public:
    named(std::nullptr_t) : ctrl(nullptr) { }

    template<typename... Args>
    explicit named(GLuint name, Args&&... args) {
        if (name != 0) {
            ctrl = new control_block(name, std::forward<Args>(args)...);
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

protected:
    U& playload() {
        return ctrl->payload;
    }

    const U& payload() const {
        return ctrl->payload;
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
        template<typename... Args>
        control_block(GLuint name, Args&&... args)
                : refcount(1), name(name), payload(std::forward<Args>(args)...) { }

        int incref() { return ++refcount; }
        int decref() { return --refcount; }

        int refcount;
        GLuint name;
        U payload;
    };

private:
    control_block *ctrl;
};



} //glue::

#endif
