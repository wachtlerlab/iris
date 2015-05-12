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

    entity() : my_id("") { }
    entity(std::string the_id) : my_id(the_id) { }

    virtual std::string identifier() const { return my_id; }
    virtual std::string qualified_id() const { return my_id; };

protected:
    std::string my_id;
};


struct monitor : public entity {

    std::string vendor;
    std::string name;
    std::string year;
    std::string notes;

    struct mode {
        float width;  // in px
        float height;

        float refresh; //in Hz

        //color depth
        int r, g, b;
    };

    mode default_mode;
};

struct monitor_settings : public entity {
    std::string operator[](const std::string &key) const;
};

struct display {
    std::string monitor_id;
    std::string settings_id;

    struct hw {
        std::string uid;
        std::string link;
        std::string card;
    };

    hw platform;

    std::string   gfx;
    monitor::mode mode;
};

struct rgb2lms : public entity {

    float width; //screen width in mm
    float height; //screen width in mm

    display dsy;

    dkl::parameter dkl_params;
};

// subject data

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
 *   						/ __MODE__@DATETIME_0.rgb2lms
 *   						/ __MODE__@DATETIME_1.rgb2lms
 *               / default -> __ID __ [symlink]
 */

class store {
public:

private:
    fs::file loc;
};

} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
