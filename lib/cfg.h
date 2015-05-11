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

class entity {
public:

    entity() : id(""), rev(0) {}
    entity(const std::string &id, uint64_t rev) : id(id), rev(rev) {}

    const std::string &identifier() const { return id; }
    uint64_t revision() const { return rev; };

    virtual std::string uid() const {
        char buf[1024] = {0, };
        snprintf(buf, sizeof(buf), "%llu@%s", rev, id.c_str());
        return std::string(buf);
    }

    explicit operator bool() const { return !id.empty(); }


private:
    std::string id;
    uint64_t    rev;
};

// hardware data

class monitor : public entity {
public:

    monitor(const std::string &id, uint64_t rev) : entity(id, rev) { }

    const std::string &display_name() const { return name; }
    const std::string &notes() const { return note; }
    const std::string &monitor_type() const { return type; };

private:
    std::string name;
    std::string type;
    std::string note;
};

//TODO: move to base library
template<typename T>
struct extent_t {

    extent_t() : width(T(0)), height(T(0)) {}
    extent_t(T w, T h) : width(w), height(h) {}

    T width;
    T height;
};

typedef extent_t<float> extent_f;

class mode : public entity {
public:
    mode(const std::string &id, uint64_t rev) : entity(id, rev) { }

    const std::string monitor() const { return monitor_uid; }

    const extent_f& size() const { return phy_size; }
    const extent_f& resolution() const { return res; }

    float refresh_rate() const { return refresh; }
    int color_depth() const { return cdepth; }

private:
    // the monitor this mode belongs to
    std::string monitor_uid;

    //physical size in mm
    extent_f phy_size;

    //resolution in px
    extent_f res;

    float refresh; // refresh rate of monitor in Hz
    int   cdepth; //color-depth
};


struct calibration : public entity {
public:
    calibration(const std::string &id, uint64_t ctime) : entity(id, ctime) { }

    const std::string profile() const { return profile_id; }
    const dkl::parameter& parameter() const { return rgb2lms; };

    const std::string monitor() const { return monitor_id; }
    const std::string mode() const { return mode_id; };

    virtual std::string uid() const override {
        //FIXME: format should be __MODE__@__DATETIME
        char buf[1024] = {0, };
        snprintf(buf, sizeof(buf), "%llu@%s", revision(), identifier().c_str());
        return std::string(buf);
    }

private:
    std::string mode_id;
    std::string monitor_id;
    std::string profile_id;
    dkl::parameter rgb2lms;
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

    monitor default_monitor() const;
    mode default_mode(const monitor &for_monitor) const;
    calibration selected_calibration(const mode &mode) const;



private:
    fs::file loc;
};

} //iris::cfg
} //iris::


#endif //IRIS_CFG_H
