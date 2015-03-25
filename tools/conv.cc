#include <iris.h>
#include <misc.h>

#include <iostream>

#include <boost/program_options.hpp>

#include <random>
#include <numeric>
#include <algorithm>
#include <csv.h>
#include <fit.h>


int main(int argc, char **argv) {

    namespace po = boost::program_options;

    std::string ca_path;
    std::string infile_path;

    double iso_dl = 0.0;
    double iso_phi = 0.0;
    double contrast = 0.17;

    po::options_description opts("IRIS conversion tool");
    opts.add_options()
            ("help", "produce help message")
            ("is-dl", po::value<double>(&iso_dl))
            ("is-phi", po::value<double>(&iso_phi))
            ("calibration,c", po::value<std::string>(&ca_path)->required())
            ("file", po::value<std::string>(&infile_path)->required());

    po::positional_options_description pos;
    pos.add("file", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
        po::notify(vm);

        if (vm.count("is-dl") != vm.count("is-phi")) {
            throw std::invalid_argument("Need lumen and phase!");
        }

    } catch (const std::exception &e) {
        std::cerr << "Error while parsing commad line options: " << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
        return 1;
    }

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }

    iris::dkl::parameter params = iris::dkl::parameter::from_csv(ca_path);
    std::cerr << "Using rgb2sml calibration:" << std::endl;
    params.print(std::cerr);

    iris::rgb refpoint(0.666f, 0.666f, 0.666f);
    iris::dkl cspace(params, refpoint);

    if (vm.count("is-dl")) {
        cspace.iso_slant(iso_dl, iso_phi);
    }

    iris::csv_file fd(infile_path);
    for(const auto &rec : fd) {
        if (rec.is_empty() || rec.is_comment()) {
            continue;
        }

        if (rec.nfields() < 1) {
            std::cerr << "wrong csv file" << std::endl;
            return -1;
        }

        double x = rec.get_double(0);
        iris::rgb c = cspace.iso_lum(x, contrast);
        std::cout << x << ", " << c << std::endl;
    }

    return 0;
}
