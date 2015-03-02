#ifndef CLN_SPECTRUM_H
#define CLN_SPECTRUM_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct spectrum {

    uint16_t           wl_start;
    uint16_t           wl_step;

    std::vector<float> data;

    //optional
    std::string unit;
};


struct spectra {

    uint16_t wl_start;
    uint16_t wl_step;

    std::map<std::string, std::vector<float>> data;

    //optional
    std::string unit;
};

#endif