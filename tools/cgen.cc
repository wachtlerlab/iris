#include <h5x/File.hpp>
#include <rgb.h>

#include <boost/program_options.hpp>

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string path = "-";
    size_t n_steps = 1;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help,h", "show the helpful help. hopefully.")
            ("red,r", "red")
            ("green,g", "green")
            ("blue,b", "blue")
            ("yellow,y", "yellow")
            ("magenta,m", "magenta")
            ("cyan,c", "cyan")
            ("white,w", "white")
            ("steps,N", po::value<size_t>(&n_steps))
            ("file,f", po::value<std::string>(&path));

    po::positional_options_description pos{};
    pos.add("file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }
    std::vector<iris::rgb> base;

    if (vm.count("red") > 0) { base.push_back(iris::rgb::red()); }
    if (vm.count("green")) { base.push_back(iris::rgb::green()); }
    if (vm.count("blue")) { base.push_back(iris::rgb::blue()); }
    if (vm.count("yellow")) { base.push_back(iris::rgb::yellow()); }
    if (vm.count("magenta")) { base.push_back(iris::rgb::magenta()); }
    if (vm.count("cyan")) { base.push_back(iris::rgb::cyan()); }
    if (vm.count("white")) { base.push_back(iris::rgb::white()); }

    std::vector<float> steps = iris::rgb::linspace(n_steps);
    std::vector<iris::rgb> cl = iris::rgb::gen(base, steps);

    if (path == "-") {

        for (auto c : cl) {
            std::cout << std::hex << c << std::endl;
        }

    } else {
        h5x::NDSize dims = {cl.size(), static_cast<size_t>(3)};
        h5x::File fd = h5x::File::open(path, "a");
        h5x::DataSet patches;
        if (!fd.hasData("patches")) {
            patches = fd.createData("patches", h5x::TypeId::Float, dims);
        } else {
            patches = fd.openData("patches");
        }

        patches.setExtent(dims);
        patches.write(h5x::TypeId::Float, dims, static_cast<void *>(cl.data()));
    }

    return 0;
};