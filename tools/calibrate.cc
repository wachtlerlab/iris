
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>


static const char vs_simple[] = R"SHDR(
#version 330

layout(location = 0) in vec2 pos;

uniform mat4 viewport;

void main()
{
    gl_Position = viewport * vec4(pos, 0.0, 1.0);
}
)SHDR";

static const char fs_simple[] = R"SHDR(
#version 330

out vec4 finalColor;
uniform vec4 plot_color;

void main() {
    finalColor = plot_color;
}
)SHDR";

static void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}

static void check_gl_error(const std::string &prefix = "") {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << prefix << "OpenGL error: " << err << std::endl;
    }
}

void wnd_key_cb(GLFWwindow *wnd, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(wnd, GL_TRUE);
    }
}

namespace gl = glue;

int main(int argc, char **argv)
{
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);

    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    std::cout << "Monitor: " << glfwGetMonitorName(primary) << std::endl;

    int n_modes;
    const GLFWvidmode* modes = glfwGetVideoModes(primary, &n_modes);

    for (int i = 0; i < n_modes; i++) {
        std::cout << i << ": " << modes[i].width << " x " << modes[i].height;
        std::cout << " \t " << modes[i].redBits << " " << modes[i].greenBits << " " << modes[i].blueBits;
        std::cout << " @ " << modes[i].refreshRate << "Hz" << std::endl;
    }

    const GLFWvidmode *mm = modes + (n_modes - 1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 2);

    GLFWwindow* window = glfwCreateWindow(mm->width, mm->height, "Calibration", primary, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, wnd_key_cb);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);

    // *****
    gl::shader vs = gl::shader::make(vs_simple, GL_VERTEX_SHADER);
    gl::shader fs = gl::shader::make(fs_simple, GL_FRAGMENT_SHADER);

    vs.compile();
    fs.compile();

    gl::program prg = gl::program::make();
    prg.attach({vs, fs});
    prg.link();

    std::vector<float> box = { -0.5f,  0.5f,
                               -0.5f, -0.5f,
                                0.5f, -0.5f,

                                0.5f,  0.5f,
                               -0.5f,  0.5f,
                                0.5f, -0.5f};

    gl::buffer bb = gl::buffer::make();
    gl::vertex_array va = gl::vertex_array::make();

    bb.bind();
    va.bind();

    bb.data(box);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bb.unbind();
    va.unbind();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::mat4 vp;
    if (height > width) {
        float scale = width / static_cast<float>(height);
        vp = glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0f));
    } else {
        float scale = height / static_cast<float>(width);
        vp = glm::scale(glm::mat4(1), glm::vec3(scale, 1.0f, 1.0f));
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        prg.use();
        GLfloat point_color[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
        prg.uniform("plot_color", point_color);
        prg.uniform("viewport", vp);

        va.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        va.unbind();
        prg.unuse();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;


}