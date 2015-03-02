
#include <glue/basic.h>
#include <glue/window.h>

namespace gl = glue;

int main(int argc, char **argv) {

    if (!glfwInit()) {
        return -1;
    }

    gl::window wnd = gl::window::make(300, 300, "Test");

    wnd.make_current_context();

    gl::widget::owner w = std::make_shared<gl::widget>();

    wnd.attach(w);

    while (! wnd.should_close()) {

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        wnd.swap_buffers();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}