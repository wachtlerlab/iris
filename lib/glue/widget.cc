
#include <glue/window.h>

#include <iostream>

namespace glue {

frame frame::subframe(rect bounds) const {
    return glue::frame();
}

//*******

void widget::render() {

}

void widget::resized(frame fr) {
    std::cout << "resized!" << std::endl;
}
}