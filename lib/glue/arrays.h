
#ifndef GLUE_ARRAYS_H
#define GLUE_ARRAYS_H

#include <glue/named.h>

namespace glue {

class vertex_array : public named<vertex_array> {
public:
    vertex_array(GLuint name) : named(name, delete_name) { }

    vertex_array() : named(nullptr) { }

    static vertex_array make() {
        vertex_array va = make_name();
        return va;
    }

    inline void bind() {
        glBindVertexArray(name());
    }

    static inline void unbind() {
        glBindVertexArray(0);
    }

    static GLuint make_name() {
        GLuint my_name;
        glGenVertexArrays(1, &my_name);
        return my_name;
    }

    static void delete_name(GLuint my_name) {
        glDeleteVertexArrays(1, &my_name);
    }

};

}

#endif