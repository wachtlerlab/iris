
#ifndef GLUE_SHADER_H
#define GLUE_SHADER_H

#include <glue/named.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>
#include <vector>

namespace glue {

class shader : public named<shader> {
public:

    shader() : named(nullptr) {}
    shader(GLuint name) : named(name) { }

    void compile();

    static shader make(const std::string &text, GLenum shader_type);

    static void delete_name(GLuint my_name) {
        glDeleteShader(my_name);
    }

};

class program : public named<program> {
public:

    program() : named(nullptr) { }
    program(GLuint name) : named(name) { }

    static program make() {
        program prg = make_name();
        return prg;
    }

    void link();

    void attach(const shader &shdr) {
        glAttachShader(name(), shdr.name());
    }

    void attach(const std::vector<shader> &shdrs) {
        for (const auto &shdr : shdrs) {
            attach(shdr);
        }
    }

    static GLuint make_name() {
        return glCreateProgram();
    }

    void detach(const shader &shdr) {
        glDetachShader(name(), shdr.name());
    }

    void detach(const std::vector<shader> &shdrs) {
        for (const auto &shdr : shdrs) {
            detach(shdr);
        }
    }

    inline void use() {
        glUseProgram(name());
    }

    static inline void unuse() {
        glUseProgram(0);
    }

    GLint attrib(const char *attrib_name) {
        return glGetAttribLocation(name(), attrib_name);
    }

    GLint uniform(const char *uniform_name) {
        return glGetUniformLocation(name(), uniform_name);
    }

    void uniform(const char *uniform_name, const glm::mat3& m) {
        glUniformMatrix3fv(uniform(uniform_name), 1, GL_FALSE, glm::value_ptr(m));
    }

    void uniform(const char *uniform_name, const glm::mat4& m) {
        glUniformMatrix4fv(uniform(uniform_name), 1, GL_FALSE, glm::value_ptr(m));
    }

    void uniform(const char *uniform_name, GLint v) {
        glUniform1i(uniform(uniform_name), v);
    }

    void uniform(const char *uniform_name, float (&v)[4]) {
        glUniform4fv(uniform(uniform_name), 1, v);
    }

    void uniform(const char *uniform_name, float v) {
        glUniform1f(uniform(uniform_name), v);
    }

    static void delete_name(GLuint my_name) {
        glDeleteProgram(my_name);
    }
};


} //glue::

#endif
