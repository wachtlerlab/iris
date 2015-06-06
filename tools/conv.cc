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


int main(int argc, char **argv) {

    namespace po = boost::program_options;

    std::string mdev;
    std::string infile_path;
    std::string sid;

    double contrast = 0.17;

    po::options_description opts("IRIS conversion tool");
    opts.add_options()
            ("help", "produce help message")
            ("monitor", po::value<std::string>(&mdev))
            ("subject,S", po::value<std::string>(&sid))
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

    iris::data::store store = iris::data::store::default_store();

    if (mdev.empty()) {
        mdev = store.default_monitor();
    }

    iris::data::monitor moni = store.load_monitor(mdev);
    iris::data::monitor::mode mode = moni.default_mode;
    iris::data::display display = store.make_display(moni, mode, "gl");
    iris::data::rgb2lms rgb2lms = store.load_rgb2lms(display);


    std::cout << "[I] Monitor: " << moni.name << " [" << mdev << "]" << std::endl;

    iris::dkl::parameter params = rgb2lms.dkl_params;
    std::cerr << "[I] rgb2sml calibration:" << std::endl;
    params.print(std::cerr);

    std::cerr << "[I] gray level: " << rgb2lms.gray_level << std::endl;
    iris::rgb refpoint = iris::rgb::gray(rgb2lms.gray_level);
    iris::dkl cspace(params, refpoint);

    if (vm.count("subject")) {
        std::vector<iris::data::subject> hits = store.find_subjects(sid);
        if (hits.empty()) {
            std::cerr << "Coud not find subject [" << sid << "]" << std::endl;
        } else if (hits.size() > 1) {
            std::cerr << "Ambigous subject string (> 1 hits): " << std::endl;
            for (const auto &s : hits) {
                std::cerr << "\t" << s.initials << std::endl;
            }
        }

        const iris::data::subject subject = hits.front(); // size() == 1 asserted
        iris::data::isoslant iso = store.load_isoslant(subject);
        cspace.iso_slant(iso.dl, iso.phi);
    }

    std::cerr << "[I] contrast: " << contrast << std::endl;

    iris::csv_file fd(infile_path);
    std::cout << "angle, r, g, b";
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
        std::cout << std::endl << x << ", " << c;
    }

    return 0;
}

