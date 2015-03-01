
#include <glue/monitor.h>

#include <boost/program_options.hpp>

#include <iostream>

namespace gl = glue;

int main(int argc, char **argv) {

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    std::vector<gl::monitor> monitors = gl::monitor::monitors();

    for (const gl::monitor &m : monitors) {
        std::cout << "Monitor: " <<  m.name();
        if (m.is_primary()) {
            std::cout << " [primary]";
        }
        std::cout << std::endl;

        gl::extent phy = m.physical_size();
        std::cout << "\t size: " << phy.width << "×" << phy.height << " [mm]" << std::endl;

        std::cout << "\t ppi: " << m.ppi() << std::endl;

        std::cout << "\t modes:" << std::endl;
        gl::monitor::mode cm = m.current_mode();
        for (const gl::monitor::mode &md : m.modes()) {
            if (std::memcmp(&md, &cm, sizeof(md)) == 0) {
                std::cout << "\t * ";
            }  else {
                std::cout << "\t   ";
            }
            std::cout << md.size.width << "×" << md.size.height;
            std::cout << " @ " << md.rate << "Hz";

            std::cout << "  [" << md.red << ", " << md.green << ", " << md.blue << "]";
            std::cout << std::endl;
        }

    }

    return 0;
}