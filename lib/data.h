#ifndef IRIS_CFG_H
#define IRIS_CFG_H

#include <string>
#include <vector>
#include <cstdint>

#include <dkl.h>
#include <fs.h>
#include <spectra.h>


namespace iris {
namespace data {

struct entity {

    entity() : my_id("") { }
    explicit entity(std::string the_id) : my_id(the_id) { }

    virtual std::string identifier() const { return my_id; }
    virtual std::string qualified_id() const { return my_id; };

protected:
    std::string my_id;
};


struct monitor : public entity {

    using entity::entity;

    std::string vendor;
    std::string name;
    std::string year;
    std::string notes;
    std::string serial;

    struct mode {
        float width;  // in px
        float height;

        float refresh; //in Hz

        //color depth
        int r, g, b;
    };

    mode default_mode;

    struct settings : public entity {
        std::string operator[](const std::string &key) const;
    };

};

struct display {

    std::string monitor_id;
    std::string settings_id;
    std::string link_id;

    std::string   gfx;
    monitor::mode mode;
};


struct rgb2lms : public entity {
    using entity::entity;

    float width; //screen width in mm
    float height; //screen width in mm

    //neutral gray that calibration was done with as bg
    float gray_level;

    display dsy;

    dkl::parameter dkl_params;

    std::string dataset;
};

// subject data

struct subject : entity {
    using entity::entity;

    std::string initials;
    std::string name;
};

struct isoslant : entity {
    using entity::entity;

    double dl;
    double phi;

    //metadata
    int64_t     timestamp;
    std::string monitor;
    int64_t     calib;
};


///

/* store dir layout:
 *     / version
 *     / monitors / __ID__ / __ID__.monitor (or __ID__.monitor)
 *                         / __DATETIME__.settings
 *                         / __DATETIME__.rgb2lms
 *     / default.monitor -> /monitors/ __ID __ [symlink]
 *     / links.cfg
 */

class store {
public:

    static store default_store();

    //monitor functions
    std::string default_monitor() const;
    std::vector<std::string> list_monitors() const;
    monitor load_monitor(const std::string &uid) const;

    std::string latest_settings(const monitor &monitor) const;
    std::vector<std::string> list_settings(const monitor &monitor) const;

    //calibration functions
    rgb2lms load_rgb2lms(const display &display) const;


    display make_display(const monitor &monitor, const monitor::mode &mode, const std::string &gfx) const;

    // yaml config IO

    static monitor yaml2monitor(const std::string &data);
    static rgb2lms yaml2rgb2lms(const std::string &data);
    static std::string monitor2yaml(const monitor &monitor);
    static std::string display2yaml(const display &display);
    static std::string rgb2lms2yaml(const rgb2lms &rgb2lms);

private:
    store(const fs::file &path);

private:
    fs::file base;
};

} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
