

#include <iostream>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <serial.h>
#include <pr655.h>

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string device;
    std::string mtype;

    po::options_description opts("pr655 commandline tool");
    opts.add_options()
            ("help", "produce help message")
            ("device", po::value<std::string>(&device))
            ("measurement", po::value<std::string>(&mtype), "for now only 'spectrum' is valid");

    po::positional_options_description pos;
    pos.add("measurement", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }

    std::cerr << device << std::endl;

    device::pr655 meter = device::pr655::open(device);

    std::string prefix = "# ";
    try {
        bool could_start = meter.start();
        if (! could_start) {
            std::cerr << "Could not start remote mode" << std::endl;
            meter.stop();
            return -1;
        }

        std::string res = meter.serial_number();
        std::cout << prefix << res << std::endl;

        res = meter.model_number();
        std::cout << prefix << res << std::endl;

        meter.units(true);

        device::pr655::response<bool> bres = meter.sync_mode(device::pr655::SyncMode::Adaptive);

        if (!bres) {
            std::cout << "[W] could not set SyncMode to Adaptive" << std::endl;
        }

        std::cout << prefix << "status: " << meter.istatus().code << std::endl;

        meter.measure();

        device::pr655::cfg config = meter.config();

        std::cout << prefix << " λ start \t stop \t step" << std::endl;
        std::cout << prefix << "   " <<  config.wl_start;
        std::cout << " \t " << config.wl_stop << " \t " << config.wl_inc << std::endl;

        std::cout << std::endl;

        spectral_data data = meter.spectral();

        std::cout << prefix << "spectral data" << std::endl;
        std::cout << prefix << "λ \t ri" << std::endl;

        for (size_t i = 0; i < data.data.size(); i++) {
            std::cout << data.wl_start + i * data.wl_step << " \t ";
            std::cout << data.data[i] << std::endl;
        }

        std::cout << std::endl;

        device::pr655::response<device::pr655::brightness> br = meter.brightness_pm();
        if (br) {
            device::pr655::brightness b = br.data;
            std::cout << "Luminance: " << b.Y << std::endl;
            std::cout << "x: " << b.x << ", y: " << b.y << std::endl;
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    meter.stop();

    return 0;
}