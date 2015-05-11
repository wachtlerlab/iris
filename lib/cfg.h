#ifndef IRIS_CFG_H
#define IRIS_CFG_H

#include <string>
#include <vector>
#include <cstdint>

#include <dkl.h>
#include <fs.h>
#include <spectra.h>


namespace iris {
namespace cfg {

struct entity {
public:

    entity() : id(""), rev(0) {}
    entity(const std::string &id, uint64_t rev) : id(id), rev(rev) {}

    const std::string &identifier() const { return id; }
    uint64_t revision() const { return rev; };

    std::string uid() const {
        char buf[1024] = {0, };
        snprintf(buf, sizeof(buf), "%llu@%s", rev, id.c_str());
        return std::string(buf);
    }

    explicit operator bool() const { return !id.empty(); }


private:
    std::string id;
    uint64_t    rev;
};


struct monitor {
    std::string id;

    std::string name;
    std::string type;

    std::string note;
};

struct mode {
    std::string id;

    //physical size in mm
    float phy_width;
    float phy_height;

    //resolution in px
    float res_width;
    float res_height;

    float refresh;
    int   cdepth; //color-depth
};


// color-calibrated screen
struct cc_screen {
    std::string id;

    std::string profile;

    monitor selected_monitor;
    mode    selected_mode;

    dkl::parameter rgb2lms;
    std::string    calibation_id;
};


struct isoslant {
    std::string id;

    double dl;
    double phi;

    //metadata
    int64_t     timestamp;
    std::string monitor;
    int64_t     calib;
};


struct subject {
    std::string id;

    std::string initials;
    std::string name;
};


///

/* possible dir layout:
 *     / monitors / __ID__ / device.monitor (or __ID__.monitor)
 *                         / 1024x1200:85$1.mode
 *                         / default.mode -> 1024x1200:5$1.mode [symlink]
 *   						/ __MODE__@DATETIME#1.rgb2lms
 *   						/ __MODE__@DATETIME#1.rgb2lms
 *               / default -> __ID __ [symlink]
 */


} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
