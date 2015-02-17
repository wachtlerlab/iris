
#include "pr655.h"

namespace device {


bool pr655::start() {
    io.send_data("PHOTO");
    std::vector<char> resp = io.recv_data(12, 1000);
    std::string line(resp.data(), resp.size());
    bool res = !line.compare(1, 11, "REMOTE MODE");
    io.wait(1000);
    return res;
}


void pr655::stop() {
    try {
        io.send_data("Q");
        io.eatup();
    } catch (...) {

    }
}

spectral_data pr655::spectral() {

    if (lm < 0) {
        //FIXME: maybe just measure in that case?
        throw std::invalid_argument("Need to measure before getting spectral data");
    }

    response<std::string> res = io_cmd("D5");

    spectral_data data;
    data.is_valid = res.code == 0;
    data.wl_start = hw.wl_start;
    data.wl_step = hw.wl_inc;

    // |qqqqq,UUUU,w.wwwe+eee,i.iiie-ee,p.pppe+ee CRLF [16]
    // |wl,spectral data CRLF

    for (uint32_t i = 0; i < hw.n_points; i++) {
        std::string line = io.recv_line();

        unsigned short lambda;
        float ri;
        std::sscanf(line.c_str(), "%hu,%f", &lambda, &ri);

        data.data.push_back(ri);

        std::cout << lambda << " | " << ri << std::endl;
    }

    return data;
}


bool pr655::measure() {
    response<std::string> res = io_cmd("M120");

    if (res) {
        hw = parse_hw_config(res.data);
        auto now = std::chrono::system_clock::now();
        lm = std::chrono::system_clock::to_time_t(now);
    }

    return res.code == 0;
}

pr655::response<std::string> pr655::io_cmd(const std::string &cmd) {
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

pr655::response<std::string> pr655::parse_status(const std::string &resp) {

    if (resp.size() < 5) {
        throw std::invalid_argument("Cannot parse status code (< 5)");
    }

    int code = std::stoi(resp);
    std::cerr << "[D] L: [" << resp.data() << "] (" << resp.size() << ") -> " << code << std::endl;

    size_t pos = resp.size() > 5 && resp[5] == ',' ? 6 : 5;
    return response<std::string>(code, resp.substr(pos));
}


pr655::cfg pr655::parse_hw_config(const std::string &line) {
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



} //namespace device