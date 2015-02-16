#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <stdexcept>

namespace device {

class sleeper {
public:
    typedef std::chrono::milliseconds dur_t;
    typedef dur_t::rep rep;

    sleeper(rep global, rep local) :
            night(global), nap(local), cur_nap(local) {
        reset();
    }

    void reset() {
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
            cur_nap = nap;
        }

        auto the_nap = std::min(cur_nap, night - t_elapsed);

        dur_t timeout(the_nap);
        std::this_thread::sleep_for(timeout);

        return true;
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

    std::vector<char> recv_line(sleeper::rep read_timeout = 5000) {
        sleeper timeout(read_timeout, 1);

        std::vector<char> buf;

        char ch = '\0';

        do {
            ssize_t nread = read(fd, &ch, 1);
            if (nread < 0) {
                throw std::runtime_error("r failed");
            }

            buf.push_back(ch);

            if (!timeout.sleep(nread == 0)) {
                throw std::runtime_error("w failed: timeout");
            }

        } while (ch != '\n');

        return buf;
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

    std::vector<char> send_and_recv_line(const std::string &cmd, sleeper::rep t_pause = 200) {
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

    typedef unsigned long status;

public:
    pr655(const serial &port) : io(port) {}

    static pr655 open(const std::string &path) {
        serial s = serial::open(path);
        pr655 meter(s);
        return meter;
    }

    bool start() {
        io.send_and_recv("PHOTO", 12, 1000);
        return true;
    }

    void stop() {
        io.send_data("Q");
        io.eatup();
    }

    std::string serial_number() {
        std::vector<char> resp = io.send_and_recv("D110", 16);
        return std::string(resp.data());
    }

    std::string model_number() {
        std::vector<char> resp = io.send_and_recv("D111", 14);
        return std::string(resp.data());
    }

    void units(bool metric) {
        std::string cmd = metric ? "SU1" : "SU0";
        std::vector<char> resp = io.send_and_recv(cmd, 7);
    }

    void measure() {
        io.send_data("M5");
        io.wait(1000);

        std::vector<char> header = io.recv_data(39, 30000);
        std::cout << header.data() << std::endl;

        //qqqqq,UUUU,w.wwwe+eee,i.iiie-ee,p.pppe+ee CRLF [16]
        for(uint32_t i = 0; i < 101; i++) {
            std::vector<char> line = io.recv_data(16);
            char *s_end = nullptr;
            unsigned long lambda = std::strtoul(line.data(), &s_end, 10);
            double ri = std::strtod(line.data() + 5, &s_end);
            std::cout << lambda << " | " << ri << std::endl;
        }

    }

    status parse_status(std::vector<char> data) {
        char *s_end;

        if (data.size() < 4) {
            std::invalid_argument("Cannot parse status code (< 4)");
        }

        unsigned long code = strtoul(data.data(), &s_end, 10);
        return code;
    }

private:
    serial io;
};


}

int main(int argc, char **argv) {

    device::pr655 meter = device::pr655::open(argv[1]);

    meter.start();

    std::string res = meter.serial_number();
    std::cerr << res << std::endl;

    res = meter.model_number();
    std::cerr << res << std::endl;

    meter.units(true);

    meter.measure();

    meter.stop();

    return 0;
}