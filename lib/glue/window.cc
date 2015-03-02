
#include <glue/window.h>

#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace glue {


window window::make(int height, int width, const std::string &title, monitor m) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWmonitor *moni = static_cast<GLFWmonitor *>(m);
    GLFWwindow *wnd = glfwCreateWindow(height, width, title.c_str(), moni, nullptr);

    if (!wnd) {
        throw std::runtime_error("Could not create window");
    }



    return window(wnd);
}

window::window(GLFWwindow *window) : wnd(window) {

    glfwSetWindowUserPointer(wnd, this);

    glfwSetKeyCallback(wnd, key_gl_cb);

    glfwSetFramebufferSizeCallback(wnd, fb_size_gl_cb);
    glfwSetCursorPosCallback(wnd, cursor_gl_cb);
    glfwSetMouseButtonCallback(wnd, mouse_button_gl_cb);

    int fb_w, fb_h;
    glfwGetFramebufferSize(wnd, &fb_w, &fb_h);
    extent fb_size(fb_w, fb_h);

    fr = make_frame(fb_size);
}

window::~window() {

    glfwSetFramebufferSizeCallback(wnd, nullptr);
    glfwSetWindowCloseCallback(wnd, nullptr);
    glfwSetWindowSizeCallback(wnd, nullptr);

    glfwSetCursorPosCallback(wnd, nullptr);
    glfwSetKeyCallback(wnd, nullptr);
    glfwSetMouseButtonCallback(wnd, nullptr);
}

#define GET_WND(wnd__) static_cast<window *>(glfwGetWindowUserPointer(wnd__))


void window::framebuffer_size_changed(extent size) {
    fr = make_frame(size);

    if (child) {
        child->resized(fr);
    }
}


frame window::make_frame(extent size) const {
    const float height = size.height;
    const float width = size.width;

    glm::mat4 vp;
    if (height > width) {
        float scale = width / static_cast<float>(height);
        vp = glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0f));
    } else {
        float scale = height / static_cast<float>(width);
        vp = glm::scale(glm::mat4(1), glm::vec3(scale, 1.0f, 1.0f));
    }
    std::cerr << "frame: " << height << " x " << width << std::endl;

    return frame(vp);
}

void window::fb_size_gl_cb(GLFWwindow *gl_win, int width, int height) {
    window *wnd = GET_WND(gl_win);
    glViewport(0, 0, width, height);
    wnd->framebuffer_size_changed(extent(width, height));
}

void window::cursor_gl_cb(GLFWwindow *gl_win, double x, double y) {
    //window *wnd = GET_WND(gl_win);
}

void window::mouse_button_gl_cb(GLFWwindow *gl_win, int button, int action, int mods) {
    //window *wnd = GET_WND(gl_win);
}

void window::key_gl_cb(GLFWwindow *gl_win, int key, int scancode, int action, int mods) {
    //window *wnd = GET_WND(gl_win);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(gl_win, GL_TRUE);
    }
}
}