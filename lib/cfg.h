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

struct calibration {

    dkl::parameter rgb2lms_params;

    //metadata
    int64_t     timestamp;
    std::string device;
};

struct monitor {
    std::string name;
    std::string id;

    std::string note;

    //size in mm
    float width;
    float height;

    //calibration stuff
    std::vector<calibration> calibrations;
    int64_t selected_calibration;
};


struct subject {

    std::string initials;
    std::string name;

    //isoslant corrections
    std::vector<isoslant> corrections;
    int64_t selected_correction;
};


} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
