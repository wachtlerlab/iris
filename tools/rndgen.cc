#include <iris.h>
#include <misc.h>

#include <iostream>


#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <random>
#include <numeric>
#include <algorithm>
#include <fs.h>
#include <stdlib.h>

bool do_debug = true;

bool mod_constraints_ok(const std::vector<size_t> &numbers, size_t mc) {
    if (mc == 0) {
        return true;
    }

    auto pos = std::adjacent_find(numbers.cbegin(), numbers.cend(),
                                  [mc](const size_t a, const size_t b) {
                                      return a % mc == 0 && b % mc == 0;
                                  });

    bool ok = pos == numbers.cend();

    if (! ok) {
        auto npos = std::distance(numbers.cbegin(), pos);
        std::cerr << "[D] mod reroll: ";
        std::cerr << *pos++ << ", " << *pos;
        std::cerr << npos;
        std::cerr << std::endl;
    }

    return ok;
}

bool mod_divisor_ok(const std::vector<size_t> &numbers, size_t md) {
    if (md == 0) {
        return true;
    }

    auto pos = std::adjacent_find(numbers.cbegin(), numbers.cend(),
                                  [md](const size_t a, const size_t b) {
                                      const auto ad = std::lldiv(a, md);
                                      const auto bd = std::lldiv(b, md);
                                      return ad.quot == bd.quot;
                                  });

    bool ok = pos == numbers.cend();

    if (! ok) {
        auto npos = std::distance(numbers.cbegin(), pos);
        std::cerr << "[D] div reroll: ";
        std::cerr << *pos++ << ", " << *pos;
        std::cerr << " @ " << npos;
        std::cerr << std::endl;
    }

    return ok;
}

int main(int argc, char **argv)
{
    namespace po = boost::program_options;

    size_t N = 0;
    size_t mod_constraint = 0;
    size_t mod_divisor = 0;
    std::string outfile = "";

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("c-mod", po::value<size_t>(&mod_constraint))
            ("c-div", po::value<size_t>(&mod_divisor))
            ("file,F", po::value<std::string>(&outfile))
            ("number,N", po::value<size_t>(&N)->required());

    po::positional_options_description pos;
    pos.add("number", 1);

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

    std::vector<size_t> numbers(N);
    std::iota(numbers.begin(), numbers.end(), 0);

    std::random_device rnd_dev;
    std::mt19937 rnd_gen(rnd_dev());

    if (do_debug) {
        std::cerr << "#mt-state:  " << rnd_gen << std::endl;
    }

    size_t counter = 0;
    do {
        counter++;
        std::shuffle(numbers.begin(), numbers.end(), rnd_gen);
    } while(!mod_constraints_ok(numbers, mod_constraint) ||
            ! mod_divisor_ok(numbers, mod_divisor));

    if (do_debug) {
        std::cerr << "[D] " << counter <<  "# of rerolls" << std::endl;
    }

    std::stringstream outstr;
    outstr << "rnd";

    for (const size_t n : numbers) {
        outstr << std::endl << n;
    }

    if (outfile.empty()) {
        std::cout << outstr.str() << std::endl;
    } else {
        fs::file outfd(outfile);
        outfd.write_all(outstr.str());
    }


    return 0;
}