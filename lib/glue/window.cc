
#include <glue/window.h>

#include <stdexcept>

namespace glue {

#define GET_WND(wnd__) static_cast<window *>(glfwGetWindowUserPointer(wnd__))

static void fb_size_gl_cb(GLFWwindow *gl_win, int width, int height) {
    window *wnd = GET_WND(gl_win);
    glViewport(0, 0, width, height);
    wnd->framebuffer_size_changed(extent(width, height));
}

static void cursor_gl_cb(GLFWwindow *gl_win, double x, double y) {
    window *wnd = GET_WND(gl_win);
    point pos(static_cast<float>(x), static_cast<float>(y));
    wnd->pointer_moved(pos);
}

static void mouse_button_gl_cb(GLFWwindow *gl_win, int button, int action, int mods) {
    window *wnd = GET_WND(gl_win);
    wnd->mouse_button_event(button, action, mods);
}

static void key_gl_cb(GLFWwindow *gl_win, int key, int scancode, int action, int mods) {
    window *wnd = GET_WND(gl_win);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(gl_win, GL_TRUE);
    }

    wnd->key_event(key, scancode, action, mods);
}

GLFWwindow* window::make(int height, int width, const std::string &title, monitor m) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWmonitor *moni = static_cast<GLFWmonitor *>(m);
    GLFWwindow *wnd = glfwCreateWindow(height, width, title.c_str(), moni, nullptr);

    if (!wnd) {
        throw std::runtime_error("Could not create window");
    }

    return wnd;
}

void window::init() {

    glfwSetWindowUserPointer(wnd, this);
    glfwSetKeyCallback(wnd, key_gl_cb);
    glfwSetFramebufferSizeCallback(wnd, fb_size_gl_cb);
    glfwSetCursorPosCallback(wnd, cursor_gl_cb);
    glfwSetMouseButtonCallback(wnd, mouse_button_gl_cb);
}


void window::cleanup() {

    if (wnd == nullptr) {
        return;
    }

    glfwSetFramebufferSizeCallback(wnd, nullptr);
    glfwSetWindowCloseCallback(wnd, nullptr);
    glfwSetWindowSizeCallback(wnd, nullptr);

    glfwSetCursorPosCallback(wnd, nullptr);
    glfwSetKeyCallback(wnd, nullptr);
    glfwSetMouseButtonCallback(wnd, nullptr);
}

window::window(GLFWwindow *window) : wnd(window) {
    init();
}

window::window(int height, int width, const std::string &title, monitor m)
        : window(make(height, width, title, m)) {
}


window::window(const std::string &title, monitor m) {
    std::vector<monitor::mode> mm =  m.modes();
    monitor::mode best = mm.back();

    int height = static_cast<int>(best.size.height);
    int width = static_cast<int>(best.size.width);

    wnd = make(height, width, title, m);
    init();
}

window::~window() {
    cleanup();
}

}