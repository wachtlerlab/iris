#ifndef GLUE_WINDOW_H
#define GLUE_WINDOW_H

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <glue/basic.h>
#include <glue/monitor.h>
#include <glue/widget.h>

#include <glm/matrix.hpp>

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

    void framebuffer_size_changed(extent size);

    void attach(widget::owner w) {
        child = w;
        child->resized(fr);
    }

private:
    frame make_frame(extent size) const;

private:
    static void fb_size_gl_cb(GLFWwindow *gl_win, int width, int height);
    static void cursor_gl_cb(GLFWwindow *gl_win, double x, double y);
    static void mouse_button_gl_cb(GLFWwindow *gl_win, int button, int action, int mods);
    static void key_gl_cb(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    GLFWwindow    *wnd;
    frame          fr;
    widget::owner  child;
};


} //glue::

#endif