#ifndef GLUE_CONTAINER_H
#define GLUE_CONTAINER_H

#include <glue/widget.h>

#include <vector>

namespace glue {

class container : widget {



private:
    std::vector<widget::owner> children;
};

}

#endif