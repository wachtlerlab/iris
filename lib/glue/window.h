#ifndef GLUE_WINDOW_H
#define GLUE_WINDOW_H

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <glue/basic.h>
#include <glue/monitor.h>

#include <string>

namespace glue {

class window {
protected:
    window(GLFWwindow *window);

public:
    static window make(int height, int width, const std::string &title, monitor m = monitor{});

    virtual ~window();

    bool should_close() const {
        return glfwWindowShouldClose(wnd) != 0;
    }

    void swap_buffers() {
        glfwSwapBuffers(wnd);
    }

    void make_current_context() {
        glfwMakeContextCurrent(wnd);
    }

private:
    static void fb_size_cb(GLFWwindow *gl_win, int width, int height);
    static void cursor_cb(GLFWwindow *gl_win, double x, double y);
    static void mouse_button_cb(GLFWwindow *gl_win, int button, int action, int mods);
    static void key_cb(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    GLFWwindow *wnd;
};


} //glue::

#endif