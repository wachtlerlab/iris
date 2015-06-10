
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
    double offset = -1.0;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("fit-frequency", po::value<bool>(&fit_freq), "also fit sin frequency [default=false]")
            ("offset", po::value<double>(&offset), "fix the offset [default=fit it]")
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

    iris::sin_fitter fitter(x, y, fit_freq, offset);
    bool res = fitter();

    std::cerr << "success: " << res << std::endl;
    std::cout << fitter.amplitude() << " " << fitter.phase() << " ";
    std::cout << fitter.offset() << (offset < 0 ? " " : "* ");
    std::cout << fitter.frequency() << (fit_freq ? " " : "* ");
    std::cout << std::endl;

    if (res) {
        std::string tstamp = iris::make_timestamp();
        iris::data::isoslant iso(input.identifier());
        iso.dl = fitter.amplitude();
        iso.phi = fitter.phase();
        iso.subject = input.subject;
        iso.display = input.display;
        iso.rgb2lms = input.rgb2lms;
        std::cerr << "[I] subject: " << iso.subject << std::endl;
        std::cerr << "[I] rgb2lms: " << iso.rgb2lms << std::endl;

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

