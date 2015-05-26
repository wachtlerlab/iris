
#include <glue/basic.h>
#include <stdexcept>

namespace glue {


void glue_start() {
    if (!glfwInit()) {
        throw std::runtime_error("Could not initialize GL subsystem");
    }
}

}