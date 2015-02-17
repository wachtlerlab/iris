

#include <iostream>
#include <stdexcept>

#include <serial.h>
#include <pr655.h>

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

        std::cout << meter.istatus().code << std::endl;

        meter.measure();

        device::pr655::cfg config = meter.config();
        std::cout << config.wl_start << " " << config.wl_stop << " " << config.wl_inc << std::endl;

        meter.spectral();

        std::cout << meter.istatus().code << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    meter.stop();

    return 0;
}