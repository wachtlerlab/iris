
#include <iris.h>

#include <glue/basic.h>
#include <glue/window.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <dkl.h>
#include <misc.h>

#include <numeric>

#include <boost/program_options.hpp>


int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string mdev;
    double contrast = 0.16;
    size_t number = 360;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("contrast,c", po::value<double>(&contrast))
            ("number,N", po::value<size_t>(&number))
            ("monitor", po::value<std::string>(&mdev));

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
        po::notify(vm);

    } catch (const std::exception &e) {
        std::cerr << "Error while parsing commad line options: " << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
        return 1;
    }

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }

    iris::data::store store = iris::data::store::default_store();

    if (mdev.empty()) {
        mdev = store.default_monitor();
    }

    iris::data::monitor moni = store.load_monitor(mdev);
    iris::data::monitor::mode mode = moni.default_mode;
    iris::data::display display = store.make_display(moni, mode, "gl");
    iris::data::rgb2lms rgb2lms = store.load_rgb2lms(display);

    std::cerr << "[D] monitor: " << mdev << std::endl;
    std::cerr << "[D] config: " << rgb2lms.qualified_id() << std::endl;
    std::cerr << "[D] contrast: " << contrast << std::endl;

    std::vector<double> phi = iris::linspace(0.0, 2 * M_PI, number);
    iris::dkl dkl = iris::dkl(rgb2lms.dkl_params, iris::rgb::gray(rgb2lms.gray_level));

    std::cout << "angle, r, g, b" << std::endl;
    for (const double angle : phi) {
        iris::rgb color = dkl.iso_lum(angle, contrast);
        double deg = angle / M_PI * 180.0;
        std::cout << deg << ", " << color << std::endl;
    }

}
