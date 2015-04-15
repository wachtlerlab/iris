#ifndef IRIS_CFG_H
#define IRIS_CFG_H

#include <string>
#include <vector>
#include <cstdint>

#include "dkl.h"

namespace iris {
namespace cfg {

struct isoslant {

    double dl;
    double phi;

    //metadata
    int64_t     timestamp;
    std::string monitor;
    int64_t     calib;
};


struct monitor {
    std::string id;
    std::string name;

    int64_t timestamp;

    std::string note;

    //for which subsystem was
    //configration made, (i.e. gl)
    std::string gfx;

    //physical size in mm
    float phy_width;
    float phy_height;

    //resolution in px
    float res_width;
    float res_height;

    //calibration stuff
    dkl::parameter rgb2lms;
};


struct subject {
    std::string id;

    std::string initials;
    std::string name;
};



} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
