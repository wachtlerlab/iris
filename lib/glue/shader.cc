
#include <glue/shader.h>

namespace glue {

shader shader::make(const std::string &text, GLenum shader_type) {
    shader shdr  = glCreateShader(shader_type);
    if (!shdr) {
        std::cerr << "[E] could not create shader" << std::endl;
    }

    const char *src = text.c_str();
    glShaderSource(shdr.name(), 1, &src, nullptr);

    return shdr;
}

void shader::compile() {
    const GLuint my_name = name();

    glCompileShader(my_name);

    GLint status;
    glGetShaderiv(my_name, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLint len;
        glGetShaderiv(my_name, GL_INFO_LOG_LENGTH, &len);

        std::vector<char> compile_log(static_cast<size_t>(len), 0);
        glGetShaderInfoLog(my_name, len, nullptr, compile_log.data());

        std::cerr << "shader compilation failed! Log:" << std::endl;
        std::cerr << compile_log.data() << std::endl;
    }
}

void program::link() {
    const GLuint my_name = name();

    glLinkProgram(name());
    GLint status;
    glGetProgramiv(my_name, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        GLint len;
        glGetProgramiv(my_name, GL_INFO_LOG_LENGTH, &len);

        std::vector<char> compile_log(static_cast<size_t>(len), 0);
        glGetProgramInfoLog(my_name, len, nullptr, compile_log.data());

        std::cerr << "program linking failed! Log:" << std::endl;
        std::cerr << compile_log.data() << std::endl;
    }
}

} // glue::