#ifndef GLUE_MONITOR_H
#define GLUE_MONITOR_H

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <glue/basic.h>

#include <vector>
#include <string>

namespace glue {


class monitor {
public:
    struct mode {
        extent size;

        int red;
        int green;
        int blue;

        int rate;
    };

public:
    monitor(GLFWmonitor *m) : handle(m) { }
    monitor() : handle(nullptr) { }

    explicit operator GLFWmonitor*() {
        return handle;
    }

    explicit operator const GLFWmonitor*() const {
        return handle;
    }

public:
    static monitor primary();
    static std::vector<monitor> monitors();

    std::string name() const {
        return std::string(glfwGetMonitorName(handle));
    }

    extent physical_size() const {
        int w, h;
        glfwGetMonitorPhysicalSize(handle, &w, &h);

        return extent(w, h);
    }

    point position() const {
        int x, y;
        glfwGetMonitorPos(handle, &x, &y);
        return point(x, y);
    }

    std::vector<mode> modes() const;

    mode current_mode() const;

    float ppi() const;

    bool is_primary() const {
        return primary().handle == handle;
    }

private:
    GLFWmonitor *handle;
};


} //glue::

#endif
