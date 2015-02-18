#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

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


int main(int argc, char **argv)
{
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);

    GLFWmonitor *primary = glfwGetPrimaryMonitor();

    std::cout << glfwGetMonitorName(primary) << std::endl;

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

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;


}