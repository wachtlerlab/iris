
#include <glue/monitor.h>
#include <cmath>

namespace glue {


monitor monitor::primary() {
    GLFWmonitor *h = glfwGetPrimaryMonitor();
    return glue::monitor(h);
}

std::vector <monitor> monitor::monitors() {
    int n;
    GLFWmonitor** monitors = glfwGetMonitors(&n);
    std::vector<monitor> result(static_cast<size_t>(n));

    std::transform(monitors, monitors + n, result.begin(), [](GLFWmonitor *m) {
        return monitor(m);
    });

    return result;
};

static monitor::mode make(const GLFWvidmode *vm) {
    monitor::mode m;
    m.red   = vm->redBits;
    m.green = vm->greenBits;
    m.blue  = vm->blueBits;
    m.rate  = vm->refreshRate;

    m.size = extent(vm->width, vm->height);
    return m;
}

std::vector<monitor::mode> monitor::modes() const {

    int c;
    const GLFWvidmode* modes = glfwGetVideoModes(handle, &c);

    std::vector<mode> mm(static_cast<size_t>(c));
    std::transform(modes, modes + c, mm.begin(), [](const GLFWvidmode &m) {
        return make(&m);
    });

    return mm;
}

monitor::mode monitor::current_mode() const {
    const GLFWvidmode* m = glfwGetVideoMode(handle);
    return make(m);
}

static float diag(const extent &s) {
    return std::sqrt(std::pow(s.width, 2.0f) + std::pow(s.height, 2.0f));
}

float monitor::ppi() const {
    // take the last one, which is hopefully the one with the highest resolution
    std::vector<mode> mms = modes();
    mode cur = mms.back();
    extent phy = physical_size();

    #define MM_TO_INCH 0.0393700787f
    float d_pxl = diag(cur.size);
    float d_phy = diag(phy) * MM_TO_INCH;

    return d_pxl / d_phy;
}
}
