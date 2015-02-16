#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <stdexcept>

struct spectral_data {

    bool is_valid;

    std::vector<uint16_t> points;
    std::vector<float>    data;
};

namespace device {

class sleeper {
public:
    typedef std::chrono::milliseconds dur_t;
    typedef dur_t::rep rep;

    sleeper(rep global, rep local) :
            night(global), nap(local), cur_nap(local) {
        start();
    }

    void start() {
        bedtime = std::chrono::system_clock::now();
    }

    bool sleep(bool sleep_longer) {

        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<dur_t>(now - bedtime); //in rep, i.e. ms

        const rep t_elapsed = elapsed.count();

        if (t_elapsed > night) {
            return false;
        }

        if (sleep_longer) {
            cur_nap *= 2;
        } else {
            reset_local();
        }

        auto the_nap = std::min(cur_nap, night - t_elapsed);

        dur_t timeout(the_nap);
        std::this_thread::sleep_for(timeout);

        return true;
    }

    bool maybe_sleep(bool should_sleep) {
        if (should_sleep) {
            return sleep(true);
        }

        reset_local();
        return true;
    }

    void reset_local() {
        cur_nap = nap;
    }

private:
    rep night;   // max sleeping until we can do, i.e. timeout
    rep nap;     // the starting consecutive amount of sleeping
    rep cur_nap; // the amount of consecutive sleeping we will do

    std::chrono::system_clock::time_point bedtime; // when the night started
};


class serial {

    serial(int fd) : fd(fd) { }

public:

    serial() : fd(-1) { }

    serial(const serial &other) {
        fd = dup(other.fd);
    }

    static serial open(const std::string &str) {
        int fd = ::open(str.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd < 0) {
            throw std::runtime_error("Could not open device file");
        }

        struct termios tio;

        int res = tcgetattr(fd, &tio);

        if (res < 0) {
            throw std::runtime_error("Could not get tc attrs");
        }

        cfsetispeed(&tio, B9600);
        cfsetospeed(&tio, B9600);

        // Set 8n1
        tio.c_cflag &= ~PARENB;
        tio.c_cflag &= ~CSTOPB;
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;

        // no flow control (flag unavailable under POSIX, so just hope...)
        tio.c_cflag &= ~CRTSCTS;

        // turn on READ & ignore ctrl lines
        tio.c_cflag |= CREAD | CLOCAL;

        // turn off s/w flow ctrl
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);

        // make raw
        tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tio.c_oflag &= ~OPOST;

        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME] = 10;

        res = tcsetattr(fd, TCSANOW, &tio);
        if (res < 0) {
            throw std::runtime_error("Could not set tc attrs");
        }

        return serial(fd);
    }

    void send_data(const std::string &str) {
        const char *data = str.c_str();
        const size_t size = str.size();
        size_t pos = 0;

        //lets say we sleep 10ms per char
        sleeper timeout(static_cast<sleeper::rep>(size) * 10, 1);

        while (pos < size) {

            ssize_t n = write(fd, data + pos, 1);
            if (n < 0) {
                throw std::runtime_error("w failed: io error");
            }

            pos += static_cast<size_t>(n);
            if (!timeout.sleep(n == 0)) {
                throw std::runtime_error("w failed: timeout");
            }
        }

        char cr[1] = {'\r'};

        for (; ;) {
            ssize_t n = write(fd, cr, 1);
            if (n == 1) {
                break;
            }

            if (!timeout.sleep(n == 0)) {
                throw std::runtime_error("w failed: timeout");
            }
        }
    };

    std::vector<char> recv_data(size_t to_read, sleeper::rep read_timeout = 5000) {
        std::vector<char> buf(to_read + 1, 0);

        size_t pos = 0;

        sleeper timeout(read_timeout, 1);

        while (pos < to_read) {
            ssize_t nread = read(fd, buf.data() + pos, to_read - pos);
            if (nread < 0) {
                throw std::runtime_error("r failed");
            }

            pos += static_cast<size_t>(nread);

            if (!timeout.sleep(nread == 0)) {
                throw std::runtime_error("w failed: timeout");
            }
        }

        return buf;
    }

    std::string recv_line(sleeper::rep read_timeout = 5000) {
        sleeper timeout(read_timeout, 1);

        std::vector<char> buf;

        char ch = '\0';

        do {
            ssize_t nread = read(fd, &ch, 1);
            if (nread < 0) {
                throw std::runtime_error("r failed");
            }

            if (nread > 0 && ch != '\r' && ch != '\n') {
                buf.push_back(ch);
            }

            if (!timeout.maybe_sleep(nread == 0)) {
                throw std::runtime_error("w failed: timeout");
            }

        } while (ch != '\n');

        return std::string(buf.data(), buf.size());
    };

    void eatup() {
        char buff[255];
        while((read(fd, buff, sizeof(buff))) > 0) {
            wait(10);
        }
    }

    std::vector<char> send_and_recv(const std::string &cmd,
                                    size_t             to_read,
                                    sleeper::rep       t_rest = 200,
                                    sleeper::rep       t_read = 5000) {
        send_data(cmd);
        wait(t_rest);
        return recv_data(to_read, t_read);
    }

    std::string send_and_recv_line(const std::string &cmd, sleeper::rep t_pause = 200) {
        send_data(cmd);
        wait(t_pause);
        return recv_line();
    }

    ~serial() {
        close(fd);
    }

    explicit operator bool() const {
        return fd > 0;
    }

    void wait(sleeper::rep time_to_wait) {
        std::chrono::milliseconds dura(time_to_wait);
        std::this_thread::sleep_for(dura);
    }

private:
    int fd;
};

class pr655 {
public:
    typedef int status;

    struct cfg {
        unsigned short n_points;

        unsigned short wl_start;
        unsigned short wl_stop;
        unsigned short wl_inc;

        float bandwidth;

    };

    pr655(const serial &port) : io(port) {}

    static pr655 open(const std::string &path) {
        serial s = serial::open(path);
        pr655 meter(s);
        return meter;
    }

    bool start() {
        std::vector<char> resp = io.send_and_recv("PHOTO", 12, 1000);
        std::string line(resp.data(), resp.size());
        std::cerr << "[" << line << "]" << std::endl;

        return !line.compare(1, 11, "REMOTE MODE");
    }

    void stop() {
        io.send_data("Q");
        io.eatup();
    }

    std::string serial_number() {
        std::string resp = io.send_and_recv_line("D110");
        status res = parse_status(resp);

        return resp;
    }

    std::string model_number() {
        std::string resp = io.send_and_recv_line("D111");
        status res = parse_status(resp);

        return resp;
    }

    void units(bool metric) {
        std::string cmd = metric ? "SU1" : "SU0";
        std::string resp = io.send_and_recv_line(cmd);
        status res = parse_status(resp);

    }

    status istatus() {
        std::string resp = io.send_and_recv_line("I");
        return parse_status(resp);
    }

    cfg config() {
        std::string resp = io.send_and_recv_line("D120");
        status res = parse_status(resp);
        if (res != 0) {
            return cfg();
        }
        //qqqqq,pp,bw,bb,ee,ii,nrp,frp,lrp CRLF
        int pp, bb, ee, ii, nrp, frp, lrp;
        float bw;

        cfg hwcfg;

        int ret = std::sscanf(resp.c_str(), "%hu,%f,%hu,%hu,%hu,%d,%d,%d",
                              &hwcfg.n_points, &hwcfg.bandwidth,
                              &hwcfg.wl_start, &hwcfg.wl_stop, &hwcfg.wl_inc,
                              &nrp, &frp, &lrp);

        if (ret != 8) {
            throw std::runtime_error("Could not parse config line");
        }

        return hwcfg;
    }

    spectral_data measure() {
        io.send_data("M5");

        std::string header = io.recv_line(50000);
        status res = parse_status(header);

        spectral_data data;
        data.is_valid = res == 0;

        // |qqqqq,UUUU,w.wwwe+eee,i.iiie-ee,p.pppe+ee CRLF [16]
        // |wl,spectral data CRLF

        for(uint32_t i = 0; i < 101; i++) {
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

    status parse_status(std::string &resp) {

        if (resp.size() < 5) {
            throw std::invalid_argument("Cannot parse status code (< 5)");
        }

        int code = std::stoi(resp);
        std::cerr << "[D] L: [" << resp.data() << "] (" << resp.size() << ") -> " << code << std::endl;

        size_t to_erase = resp.size() > 5 && resp[5] == ',' ? 6 : 5;
        resp.erase(0, to_erase);

        return code;
    }


private:
    serial io;
};


}

int main(int argc, char **argv) {

    device::pr655 meter = device::pr655::open(argv[1]);

    try {
        bool could_start = meter.start();
        if (! could_start) {
            std::cerr << "Could not start remote mode" << std::endl;
        }

        std::string res = meter.serial_number();
        std::cerr << res << std::endl;

        res = meter.model_number();
        std::cerr << res << std::endl;

        meter.units(true);

        std::cout << meter.istatus() << std::endl;

        meter.measure();

        device::pr655::cfg config = meter.config();
        std::cout << config.wl_start << " " << config.wl_stop << " " << config.wl_inc << std::endl;

        std::cout << meter.istatus() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    meter.stop();

    return 0;
}