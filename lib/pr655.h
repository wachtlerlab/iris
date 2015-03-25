#ifndef IRIS_PR655_H
#define IRIS_PR655_H

#include <serial.h>

struct spectral_data {

    bool is_valid;

    uint16_t           wl_start;
    uint16_t           wl_step;
    std::vector<float> data;
};

namespace device {

class pr655 {
public:

    template<typename T>
    struct response {

        response(int code) : code(code) { }
        response(int code, const T &payload) : code(code), data(payload) { }

        explicit operator bool() const {
            return code == 0;
        }

        template<typename U>
        response<U> map(const std::function<U(const T&)> &f) const {
            if (code == 0) {
                return response<U>(code, f(data));
            } else {
                return response<U>(code, U());
            }
        }

        int code;
        T data;
    };

    struct cfg {
        unsigned short n_points;

        unsigned short wl_start;
        unsigned short wl_stop;
        unsigned short wl_inc;

        float bandwidth;
    };

    struct brightness {
        float Y;
        float x;
        float y;
        float ut;
        float vt;
    };

    pr655() : io(), lm(-1) {}

    pr655(const serial &port) : io(port), lm(-1) {}

    pr655(serial &&port) : io(std::move(port)), lm(-1) {}

    pr655& operator=(const pr655 &other) {
        if (this == &other) {
            return *this;
        }

        try {
            stop();
        } catch (...) {
        }

        io = other.io;
        lm = other.lm;
        hw = other.hw;

        return *this;
    }

    static pr655 open(const std::string &path) {
        pr655 meter(serial::open(path));
        return meter;
    }

    bool start();

    void stop();

    std::string serial_number() {
        response<std::string> res = io_cmd("D110");
        return res.data;
    }

    std::string model_number() {
        response<std::string> res = io_cmd("D111");
        return res.data;
    }

    void units(bool metric) {
        std::string cmd = metric ? "SU1" : "SU0";
        response<std::string> res = io_cmd(cmd);
    }

    response<std::string> istatus() {
        return io_cmd("I");
    }

    cfg config() {
        response<std::string> res = io_cmd("D120");

        response<cfg> rcfg = res.map<cfg>([](const std::string &line) {
            return parse_hw_config(line);
        });

        return rcfg.data;
    }

    spectral_data spectral();

    response<brightness> brightness_pm();

    bool measure();

private:

    response<std::string> io_cmd(const std::string &cmd);
    static response<std::string> parse_status(const std::string &resp);
    static cfg parse_hw_config(const std::string &line);
    static brightness parse_brightness(const std::string &line);

private:
    serial      io;
    std::time_t lm;
    cfg         hw;
};

}

#endif