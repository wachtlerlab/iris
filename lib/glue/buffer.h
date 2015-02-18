
#ifndef GLUE_BUFFER_H
#define GLUE_BUFFER_H

#include <glue/named.h>

#include <vector>

namespace glue {

class buffer : public named<buffer> {
public:
    buffer() : named(nullptr) { }
    buffer(GLuint name) : named(name) { }

    static buffer make() {
        buffer buf = make_name();
        return buf;
    }

    inline void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, name());
    }

    static inline void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    template<typename T>
    void data(const std::vector <T> &the_data, GLenum usage = GL_STATIC_DRAW) {
        const size_t bytes = sizeof(T) * the_data.size();
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bytes), the_data.data(), usage);
    }

    void sub_data(size_t offset, float v) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(offset * sizeof(float)),
                        static_cast<GLsizeiptr>(sizeof(float)), &v);
    }

    template<typename T>
    void sub_data(size_t offset, const std::vector <T> &v) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(offset * sizeof(T)),
                        static_cast<GLsizeiptr>(sizeof(T) * v.size()),
                        v.data());
    }

    void get_sub_data(size_t offset, float &v) {
        glGetBufferSubData(GL_ARRAY_BUFFER,
                           static_cast<GLintptr>(offset * sizeof(float)),
                           static_cast<GLsizeiptr>(sizeof(float)), &v);
    }


    inline void *map(GLenum access) {
        return glMapBuffer(GL_ARRAY_BUFFER, access);
    }

    inline bool unmap() {
        return glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    static GLuint make_name() {
        GLuint my_name;
        glGenBuffers(1, &my_name);
        return my_name;
    }

    static void delete_name(GLuint my_name) {
        glDeleteBuffers(1, &my_name);
    }
};

} //glue::

#endif