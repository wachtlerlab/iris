
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

    std::string infile_path;
    bool fit_freq = false;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("fit-frequency", po::value<bool>(&fit_freq), "also fit sin frequency [default=false]")
            ("file", po::value<std::string>(&infile_path)->required());

    po::positional_options_description pos;
    pos.add("file", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
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


    iris::csv_file fd(infile_path);

    std::vector<double> x;
    std::vector<double> y;
    for(const auto &rec : fd) {
        if (rec.is_empty() || rec.is_comment()) {
            continue;
        }

        if (rec.nfields() != 2) {
            std::cerr << "wrong csv file" << std::endl;
            return -1;
        }

        x.push_back(rec.get_double(0));
        y.push_back(rec.get_double(1));
    }

    iris::sin_fitter fitter(x, y, fit_freq);
    bool res = fitter();

    std::cerr << "success: " << res << std::endl;
    std::cout << fitter.p[0] << " " << fitter.p[1] << " " << fitter.p[2] << " ";
    if (fit_freq) {
        std::cout << fitter.p[3];
    }
    std::cout << std::endl;

    return res ? 0 : -1;
}

