#ifndef GLUE_WINDOW_H
#define GLUE_WINDOW_H

#include <glue/basic.h>
#include <glue/monitor.h>

#include <string>

namespace glue {

class window {
protected:
    window(GLFWwindow *window);
    static GLFWwindow* make(int height, int width, const std::string &title, monitor m = monitor{});

    void init();
    void cleanup();

public:
    window() : wnd(nullptr) { }
    window(int height, int width, const std::string &title, monitor m = monitor{});
    window(const std::string &title, monitor m);

    window(const window &) = delete;
    window& operator=(const window &) = delete;

    window(window &&other) : wnd(other.wnd) {
        other.cleanup();
        other.wnd = nullptr;
        init();
    }

    window& operator=(window &&other) {
        if (other.wnd == this->wnd) {
            return *this;
        }

        this->cleanup();
        other.cleanup();

        std::swap(this->wnd, other.wnd);

        this->init();
        other.init();

        return *this;
    }

    virtual ~window();

    bool should_close() const {
        return glfwWindowShouldClose(wnd) != 0;
    }

    void swap_buffers() {
        glfwSwapBuffers(wnd);
    }

    void make_current_context();

    extent framebuffer_size() const {
        int width, height;
        glfwGetFramebufferSize(wnd, &width, &height);
        return extent(width, height);
    }

    virtual void framebuffer_size_changed(extent size) { }
    virtual void mouse_button_event(int button, int action, int mods) { }
    virtual void pointer_moved(point pos) { }
    virtual void key_event(int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(wnd, GL_TRUE);
        }
    }

    explicit operator GLFWwindow*() {
        return wnd;
    }

    explicit operator const GLFWwindow*() const {
        return wnd;
    }

    explicit operator bool() const {
        return wnd != nullptr;
    }

private:
    GLFWwindow *wnd;
};


} //glue::

#endif
