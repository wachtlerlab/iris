
#include <iris.h>
#include <misc.h>

#include <iostream>


#include <boost/program_options.hpp>

#include <random>
#include <numeric>
#include <algorithm>
#include <csv.h>
#include <fit.h>
#include <data.h>
#include <fs.h>

int main(int argc, char **argv) {

    namespace po = boost::program_options;

    std::string infile_path;
    bool fit_freq = false;
    bool only_stdout = false;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("fit-frequency", po::value<bool>(&fit_freq), "also fit sin frequency [default=false]")
            ("file", po::value<std::string>(&infile_path)->required())
            ("stdout", po::value<bool>(&only_stdout));

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

    fs::file fd(infile_path);
    std::string raw = fd.read_all();
    iris::data::isodata input = iris::data::store::yaml2isodata(raw);

    std::vector<double> x(input.samples.size());
    std::vector<double> y(input.samples.size());
    std::transform(input.samples.cbegin(), input.samples.cend(), x.begin(),
                   [](const iris::data::isodata::sample &s) {
                       return s.stimulus;
                   });

    std::transform(input.samples.cbegin(), input.samples.cend(), y.begin(),
                   [](const iris::data::isodata::sample &s) {
                       return s.response;
                   });

    iris::sin_fitter fitter(x, y, fit_freq);
    bool res = fitter();

    std::cerr << "success: " << res << std::endl;
    std::cout << fitter.p[0] << " " << fitter.p[1] << " " << fitter.p[2] << " ";
    if (fit_freq) {
        std::cout << fitter.p[3];
    }
    std::cout << std::endl;

    if (res) {
        std::string tstamp = iris::make_timestamp();
        iris::data::isoslant iso(input.identifier());
        iso.dl = fitter.p[0];
        iso.phi = fitter.p[1];
        iso.subject = input.subject;
        iso.display = input.display;

        std::string outdata = iris::data::store::isoslant2yaml(iso);
        if (only_stdout) {
            std::cout << outdata << std::endl;
        } else {
            fs::file outfile(iso.identifier() + ".isoslant");
            outfile.write_all(outdata);
        }
    }

    return res ? 0 : -1;
}

