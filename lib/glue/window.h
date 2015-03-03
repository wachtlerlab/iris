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

    window(const window &) = delete;
    window& operator=(const window &) = delete;

    window(window &&other) : wnd(other.wnd) {
        other.wnd = nullptr;
    }

    window& operator=(window &&other) {
        if (other.wnd == this->wnd) {
            return *this;
        }
        std::swap(this->wnd, other.wnd);
        return *this;
    }

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

    virtual void framebuffer_size_changed(extent size) {}


private:
    GLFWwindow *wnd;
};


} //glue::

#endif