
#include <misc.h>
#include <iomanip>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace iris {


std::string make_timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream out;

    out << std::put_time(&tm, "%Y%m%eT%H%M");
    return out.str();
}

}