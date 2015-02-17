#ifndef SPX_PR655_H
#define SPX_PR655_H

#include <serial.h>

struct spectral_data {

    bool is_valid;

    std::vector<uint16_t> points;
    std::vector<float>    data;
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

    pr655(const serial &port) : io(port), lm(-1) {}

    static pr655 open(const std::string &path) {
        serial s = serial::open(path);
        pr655 meter(s);
        return meter;
    }

    bool start() {
        io.send_data("PHOTO");
        std::vector<char> resp = io.recv_data(12, 1000);
        std::string line(resp.data(), resp.size());
        bool res = !line.compare(1, 11, "REMOTE MODE");
        io.wait(1000);
        return res;
    }

    void stop() {
        try {
            io.send_data("Q");
            io.eatup();
        } catch (...) {

        }
    }

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

    spectral_data spectral() {

        if (lm < 0) {
            //FIXME: maybe just measure in that case?
            throw std::invalid_argument("Need to measure before getting spectral data");
        }

        response<std::string> res = io_cmd("D5");

        spectral_data data;
        data.is_valid = res.code == 0;

        // |qqqqq,UUUU,w.wwwe+eee,i.iiie-ee,p.pppe+ee CRLF [16]
        // |wl,spectral data CRLF

        for(uint32_t i = 0; i < hw.n_points; i++) {
            std::string line = io.recv_line();

            unsigned short lambda;
            float ri;
            std::sscanf(line.c_str(), "%hu,%f", &lambda, &ri);

            data.points.push_back(lambda);
            data.data.push_back(ri);

            std::cout << lambda << " | " << ri << std::endl;
        }

        return data;
    }

    bool measure() {
        response<std::string> res = io_cmd("M120");

        if (res) {
            hw = parse_hw_config(res.data);
            auto now = std::chrono::system_clock::now();
            lm = std::chrono::system_clock::to_time_t(now);
        }

        return res.code == 0;
    }

    response<std::string> parse_status(const std::string &resp) {

        if (resp.size() < 5) {
            throw std::invalid_argument("Cannot parse status code (< 5)");
        }

        int code = std::stoi(resp);
        std::cerr << "[D] L: [" << resp.data() << "] (" << resp.size() << ") -> " << code << std::endl;

        size_t pos = resp.size() > 5 && resp[5] == ',' ? 6 : 5;
        return response<std::string>(code, resp.substr(pos));
    }

    response<std::string> io_cmd(const std::string &cmd) {
        if (cmd.empty()) {
            throw std::invalid_argument("Command must not be empty");
        }

        bool is_measurement = cmd[0] == 'M';
        io.send_data(cmd);
        io.wait(200);

        sleeper::rep tout = is_measurement ? 50000 : 5000;
        std::string l = io.recv_line(tout);

        return parse_status(l);
    }

private:

    static cfg parse_hw_config(const std::string &line) {
        //qqqqq,pp,bw,bb,ee,ii,nrp,frp,lrp CRLF
        int pp, bb, ee, ii, nrp, frp, lrp;
        float bw;

        cfg hwcfg;

        int ret = std::sscanf(line.c_str(), "%hu,%f,%hu,%hu,%hu,%d,%d,%d",
                              &hwcfg.n_points, &hwcfg.bandwidth,
                              &hwcfg.wl_start, &hwcfg.wl_stop, &hwcfg.wl_inc,
                              &nrp, &frp, &lrp);

        if (ret != 8) {
            throw std::runtime_error("Could not parse config line");
        }

        return hwcfg;
    }

private:
    serial      io;
    std::time_t lm;
    cfg         hw;
};

}

#endif