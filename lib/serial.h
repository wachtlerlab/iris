#ifndef SPX_SERIAL_H
#define SPX_SERIAL_H

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

    serial(int fd) : fd(fd) {
    }

public:

    serial() : fd(-1) {
    }

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
        while ((read(fd, buff, sizeof(buff))) > 0) {
            wait(10);
        }
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

} //namespace device

#endif //include guard