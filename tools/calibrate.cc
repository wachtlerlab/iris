
#include <h5x/File.hpp>

#include <boost/program_options.hpp>

#include <memory>
#include <iostream>
#include <string>
#include <bitset>
#include <cmath>

#include <fstream>
#include <spectra.h>
#include <rgb.h>
#include <fit.h>
#include <dkl.h>
#include <fs.h>
#include <data.h>
#include <misc.h>

static void dump_sepctra(const iris::spectra &spec) {

    std::vector<std::string> names = spec.names();
    if (!names.empty()) {
        for (const auto &name : names) {
            std::cerr << name << " ";
        }
        std::cerr << std::endl;
    }

    for (size_t i = 0; i < spec.num_spectra(); i++) {
        iris::spectrum s = spec[i];

        for (size_t k = 0; k < s.samples(); k++) {
            std::cerr << s[k] << ", ";
        }

        std::cerr << std::endl;
    }
}

std::vector<double> cmp_luminance(const h5x::File &fd, const iris::spectra &cf, const iris::spectra &spec)
{
    iris::spectrum L = cf[1];
    iris::spectrum M = cf[2];

    iris::spectrum Lumeff = L+M;

    h5x::DataSet ls = fd.openData("luminance");
    h5x::NDSize ls_size = ls.size();
    std::vector<double> lum_meter(ls_size[0]);
    ls.read(h5x::TypeId::Double, ls_size, lum_meter.data());

    std::vector<double> res(ls_size[0], 0);

    std::cerr << "measure \t calc \t Δ " << std::endl;

    for (size_t p = 0; p < spec.num_spectra(); p++) {
        iris::spectrum sp = spec[p];

        double lc = (Lumeff * sp).integrate();
        double delta =  lc - lum_meter[p];
        double epc = delta / lum_meter[p];

        res[p] = delta;

        std::cerr << lum_meter[p] << " \t " << lc;
        std::cerr << " \t " << delta << " \t " << epc * 100 << std::endl;
    }

    return res;

}

static void save_calibration_to_h5(h5x::File &fd,
                                   std::vector<double> &x,
                                   std::vector<double> &y,
                                   size_t nspec,
                                   const iris::dkl::parameter & dklp) {
    h5x::Group cag = fd.openGroup("rgb2sml", true);

    h5x::DataSet cai;
    h5x::NDSize cai_dims = {3UL, 3UL, nspec};
    if (cag.hasData("cone-activations")) {
        cai = cag.openData("cone-activations");
    } else {
        cai = cag.createData("cone-activations", h5x::TypeId::Float, cai_dims);
    }

    cai.setExtent(cai_dims);
    cai.write(h5x::TypeId::Double, cai_dims, y.data());

    cai.setAttr("levels", x);

    cag.setData("Azero", dklp.A_zero);
    cag.setData("gamma", dklp.gamma);

    h5x::DataSet caA;
    h5x::NDSize caA_dims = {3, 3};
    if (cag.hasData("A")) {
        caA = cag.openData("A");
    } else {
        caA = cag.createData("A", h5x::TypeId::Double, caA_dims);
    }

    caA.write(h5x::TypeId::Double , caA_dims, dklp.A);
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;
    using namespace iris;

    std::string input;
    std::string cones;
    double weight_exp = 1.1;
    bool check_lum = false;
    float dsp_width = -1;
    float dsp_height = -1;

    bool only_stdout = false;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("cone-fundamentals,c", po::value<std::string>(&cones)->required())
            ("weight-exponent,w", po::value<double>(&weight_exp))
            ("check-luminance", po::value<bool>(&check_lum))
            ("width,W", po::value<float>(&dsp_width))
            ("height,H", po::value<float>(&dsp_height))
            ("input", po::value<std::string>(&input)->required())
            ("stdout", po::value<bool>(&only_stdout));

    po::positional_options_description pos;
    pos.add("input", 1);
    pos.add("cone-fundamentals", 1);

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

    if (dsp_height < 0 || dsp_width < 0) {
        std::cerr << "[E] Need height and width paramters";
        return 2;
    }

    h5x::File fd = h5x::File::open(input, "r+");

    if (!fd.hasData("spectra") || !fd.hasData("patches")) {
        std::cerr << "File missing spectra or patches" << std::endl;
        return 1;
    }


    h5x::DataSet sp = fd.openData("spectra");
    h5x::DataSet ps = fd.openData("patches");

    h5x::NDSize sp_size = sp.size();
    h5x::NDSize ps_size = ps.size();

    spectra spec(sp_size[0], sp_size[1], 380, 4);

    sp.read(h5x::TypeId::Float, sp_size, spec.data());

    spectra cf = iris::spectra::from_csv(fs::file(cones));

    std::vector<iris::rgb> stim(ps_size[0]);
    ps.read(h5x::TypeId::Float, ps_size, stim.data());

    std::vector<double> y;
    std::vector<double> x;

    size_t nspec = 0;

    for (size_t cone = 0; cone < 3; cone++) {
        spectrum cs = cf[cone];

        for (size_t source = 0; source < 3; source++) {

            for (size_t p = 0; p < stim.size(); p++) {
                iris::rgb kanon = stim[p];

                std::bitset<3> bs;
                for (size_t j = 0; j < 3; j++) {
                    auto jc = std::fpclassify(kanon[j]);
                    bs.set(j, j == source ? jc != FP_ZERO : jc == FP_ZERO);
                }

                if (bs.all()) {
                    double l = (spec[p] * cs).integrate();
                    double v = kanon[source] * 255.0;
                    x.push_back(v);
                    y.push_back(l);

                    if (cone == 0 && source == 0) {
                        nspec++;
                    }
                }
            }
        }
    }


    rgb2sml_fitter fitter(x, y, weight_exp);
    fitter();

    dkl::parameter dklp = fitter.rgb2sml();

    std::string tstamp = iris::make_timestamp();
    data::rgb2lms rgb2lms(tstamp);
    rgb2lms.dkl_params = dklp;

    fd.getAttr("gray-level", rgb2lms.gray_level);
    fd.getAttr("display.monitor", rgb2lms.dsy.monitor_id);
    fd.getAttr("display.settings", rgb2lms.dsy.settings_id);
    fd.getAttr("display.link", rgb2lms.dsy.link_id);
    fd.getAttr("display.gfx", rgb2lms.dsy.gfx);
    fd.getAttr("mode.height", rgb2lms.dsy.mode.height);
    fd.getAttr("mode.width", rgb2lms.dsy.mode.width);
    fd.getAttr("mode.refresh", rgb2lms.dsy.mode.refresh);
    fd.getAttr("mode.depth.r", rgb2lms.dsy.mode.r);
    fd.getAttr("mode.depth.g", rgb2lms.dsy.mode.g);
    fd.getAttr("mode.depth.b", rgb2lms.dsy.mode.b);

    rgb2lms.dataset = input;

    rgb2lms.height = dsp_height;
    rgb2lms.width = dsp_width;

    std::string outdata =  data::store::rgb2lms2yaml(rgb2lms);

    if (only_stdout) {
        std::cout << outdata << std::endl;
    } else {
        fs::file outfile(rgb2lms.identifier() + ".rgb2lms");
        outfile.write_all(outdata);
    }

    if (check_lum) {
        std::cerr << "Luminance check: " << std::endl;
        cmp_luminance(fd, cf, spec);
    }

    fd.close();

    h5x::File caf = h5x::File::open(tstamp + ".cac", "w+");
    save_calibration_to_h5(caf, x, y, nspec, dklp);
    caf.close();

    return 0;
}
