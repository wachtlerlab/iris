
#include <misc.h>
#include <iomanip>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace iris {


std::string make_timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    char buffer[1024];

    size_t res = strftime(buffer, sizeof(buffer), "%Y%m%eT%H%M", &tm);

    if (res == 0) {
    	throw std::runtime_error("Could not format time string");
    }

    return std::string(buffer, res);
}

}