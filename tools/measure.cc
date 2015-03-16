
#include <h5x/File.hpp>

#include <pr655.h>

#include <iris.h>

#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>
#include <glue/colors.h>
#include <glue/monitor.h>
#include <glue/window.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <ios>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#include <boost/program_options.hpp>

#include <iomanip>
#include <fstream>

static const char vs_simple[] = R"SHDR(
#version 330

layout(location = 0) in vec2 pos;

uniform mat4 viewport;

void main()
{
    gl_Position = viewport * vec4(pos, 0.0, 1.0);
}
)SHDR";

static const char fs_simple[] = R"SHDR(
#version 330

out vec4 finalColor;
uniform vec4 plot_color;

void main() {
    finalColor = plot_color;
}
)SHDR";


// ***
namespace gl = glue;

void print_color_hex(const gl::color::rgba &c) {

    int r = static_cast<int>(c.r * 255.0f);
    int g = static_cast<int>(c.g * 255.0f);
    int b = static_cast<int>(c.b * 255.0f);

    std::cout << std::hex << std::setw(2) << std::setfill('0') << r;
    std::cout << std::hex << std::setw(2) << std::setfill('0') << g;
    std::cout << std::hex << std::setw(2) << std::setfill('0') << b;
}

//***

class looper {
public:

    enum class state : int {
        Waiting = 0,
        NextDisplay = 1,
        NextMeasure = 2,
        Stop = 3
    };

protected:
    virtual bool display() = 0;
    virtual void refresh() = 0;
    virtual void measure() = 0;

public:
    looper() : news(state::Waiting) {}

    bool render() {

        state cur = news;
        if (cur == state::Stop) {
            return false;
        }

        bool do_run = true;
        if (cur == state::NextDisplay) {
            do_run = display();
            state next = do_run ? state::NextMeasure : state::Stop;
            news.compare_exchange_strong(cur, next);
            cur = news;
        }

        if (do_run) {
            refresh();
        }

        return do_run;
    }

    void loop() {
        news = state::NextDisplay;

        state cur_state;
        while ((cur_state = news) != state::Stop) {

            if (cur_state != state::NextMeasure) {
                std::chrono::milliseconds tsleep(100);
                std::this_thread::sleep_for(tsleep);
                continue;
            }

            measure();
            news.compare_exchange_strong(cur_state, state::NextDisplay);
        }
    }

    void start() {

        if (news != state::Waiting) {
            throw std::invalid_argument("wrong state");
        }

        thd = std::thread(&looper::loop, this);

        while(news != state::NextDisplay) {
            std::chrono::milliseconds tsleep(50);
            std::this_thread::sleep_for(tsleep);
        }
    }

    void stop() {
        news = state::Stop;
        if (thd.joinable()) {
            thd.join();
        }
    }

    virtual ~looper() {
        if (thd.joinable()) {
            thd.join();
        }
    }

    bool is_running() const {
        state cur = news;
        return cur != state::Waiting && cur != state::Stop;
    }

private:
    std::thread        thd;
    std::atomic<state> news;
};

static void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}

static void check_gl_error(const std::string &prefix = "") {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << prefix << "OpenGL error: " << err << std::endl;
    }
}

class robot : public looper, public gl::window {
public:

    robot(gl::monitor m, device::pr655 &meter, std::vector<iris::rgb> &stim, float gray_level)
            : gl::window("iris - measure", m), meter(meter), stim(stim), gray_level(gray_level) {
        make_current_context();
        setup();
    }

    bool display() override {

        if (pos == stim.size()) {
            return false;
        }

        color = stim[pos++];
        return true;
    }

    void refresh() override {
        gl::extent fb = framebuffer_size();

        glm::mat4 vp;
        if (fb.height > fb.width) {
            float scale = fb.width / fb.height;
            vp = glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0f));
        } else {
            float scale = fb.height / fb.width;
            vp = glm::scale(glm::mat4(1), glm::vec3(scale, 1.0f, 1.0f));
        }

        glClearColor(gray_level, gray_level, gray_level, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        prg.use();
        prg.uniform("plot_color", color);
        prg.uniform("viewport", vp);

        va.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        va.unbind();
        prg.unuse();
    }

    void measure() override {
        std::cerr << " Measuring ..." << std::endl;

        meter.measure();
        spectral_data data = meter.spectral();

        resp.push_back(data);
        std::cerr << " done" << std::endl;
    }

    void setup() {

        vs = gl::shader::make(vs_simple, GL_VERTEX_SHADER);
        fs = gl::shader::make(fs_simple, GL_FRAGMENT_SHADER);

        vs.compile();
        fs.compile();

        prg = gl::program::make();
        prg.attach({vs, fs});
        prg.link();

        std::vector<float> box = { -0.5f,  0.5f,
                                   -0.5f, -0.5f,
                                   0.5f, -0.5f,

                                   0.5f,  0.5f,
                                   -0.5f,  0.5f,
                                   0.5f, -0.5f};

        bb = gl::buffer::make();
        va = gl::vertex_array::make();

        bb.bind();
        va.bind();

        bb.data(box);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        bb.unbind();
        va.unbind();

        pos = 0;

        stim.reserve(stim.size());
        color = iris::rgb::cyan();
    }

    const std::vector<iris::rgb> & stimulation() const {
        return stim;
    }

    const std::vector<spectral_data> &spectra() const {
        return resp;
    }

    virtual void key_event(int key, int scancode, int action, int mods) override {
        gl::window::key_event(key, scancode, action, mods);

        if (! is_running() && key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            start();
        }
    }

    robot& operator=(std::nullptr_t) {
        cleanup();
        return *this;
    }


private:
    gl::shader vs;
    gl::shader fs;

    gl::program prg;

    gl::buffer bb;
    gl::vertex_array va;

    device::pr655 &meter;

    // state
    gl::color::rgba color;
    size_t pos;

    // data
    std::vector<iris::rgb> stim;
    float gray_level;
    std::vector<spectral_data> resp;
};

void dump_stdout(const robot &r) {

    using namespace gl::color;

    const std::vector<iris::rgb> &stim = r.stimulation();
    const std::vector<spectral_data> &resp = r.spectra();

    if (resp.empty()) {
        std::cout << "No data!" << std::endl;
    }

    uint16_t wave = resp[0].wl_start;
    uint16_t step = resp[0].wl_step;
    size_t nwaves = resp[0].data.size();

    const std::string prefix = "# ";

    std::cout << prefix << "spectral data" << std::endl;
    std::cout << prefix << "λ    \t ";

    for (size_t i = 0; i < stim.size(); i ++) {
        std::cout << std::hex << stim[i];
        std::cout << "  \t  ";
    }

    std::cout << std::endl;
    std::cout << std::scientific;

    for (size_t i = 0; i < nwaves; i++) {
        std::cout << std::dec << wave + i * step << " \t ";

        for (size_t k = 0; k < resp.size(); k++) {
            std::cout << "  " << resp[k].data[i] << "\t";
        }

        std::cout  << std::endl;
    }

    std::cout.unsetf(std::ios_base::floatfield);
}

void save_data_h5(const std::string &path, const robot &r) {

    const std::vector<iris::rgb> &stim = r.stimulation();
    const std::vector<spectral_data> &resp = r.spectra();

    if (resp.empty()) {
        std::cout << "[W] No data!" << std::endl;
        return;
    }

    size_t nwl = resp[0].data.size();

    h5x::NDSize dims = {resp.size(), nwl};
    h5x::File fd = h5x::File::open(path, "a");
    h5x::DataSet ds;
    if (!fd.hasData("spectra")) {
        ds = fd.createData("spectra", h5x::TypeId::Float, dims);
    } else {
        ds = fd.openData("spectra");
    }

    for(size_t i = 0; i < resp.size(); i++) {
        const std::vector<float> data = resp[i].data;
        h5x::Selection memSel(data);
        h5x::Selection fileSel(ds.getSpace());
        h5x::NDSize count = {static_cast<size_t>(1), nwl};
        h5x::NDSize offset = {i, static_cast<size_t>(0)};
        fileSel.select(count, offset);
        ds.write(h5x::TypeId::Float, data.data(), fileSel, memSel);
    }

    ds.setAttr("wl_start", resp[0].wl_start);
    ds.setAttr("wl_step", resp[0].wl_step);

    if (! fd.hasData("patches")) {
        h5x::NDSize dc = {stim.size(), static_cast<size_t>(3)};
        h5x::DataSet patches;
        patches = fd.createData("patches", h5x::TypeId::Float, dc);
        patches.setExtent(dc);
        patches.write(h5x::TypeId::Float, dims, static_cast<const void *>(stim.data()));
    }

    ds.close();
    fd.close();
}

std::vector<iris::rgb> read_color_list(std::string path) {
    if (path == "-") {
        path = "/dev/stdin";
    }

    char buf[8] = {0, };

    std::ifstream fd(path, std::ios::in);

    std::vector<iris::rgb> result;
    while(fd.getline(buf, sizeof(buf) - 1, '\n')) {
        std::string s(buf, static_cast<size_t>(fd.gcount()));
        iris::rgb color;
        for (size_t i = 0; i < 3; i++) {
            std::string sub = s.substr(2*i, 2);
            size_t pos = 0;
            unsigned long c = std::stoul(sub, &pos, 16);
            if (pos == 0) {
                throw std::runtime_error("Error while parsing color list");
            }
            color.raw[i] = static_cast<float>(c / 255.0f);
        }
        result.push_back(color);
    }

    return result;
}

int main(int argc, char **argv)
{
    namespace po = boost::program_options;

    std::string device;
    std::string mdev;
    std::string input;
    float       gray_level = 0.66f;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("device,d", po::value<std::string>(&device))
            ("monitor", po::value<std::string>(&mdev))
            ("gray", po::value<float>(&gray_level), "reference gray [default=0.66]")
            ("input", po::value<std::string>(&input)->required());

    po::positional_options_description pos;
    pos.add("input", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << opts << std::endl;
        return 0;
    }

    // ****
    std::cerr << "List of colors to measure:" << std::endl;
    std::vector<iris::rgb> colors = read_color_list(input);
    for (auto c : colors) {
        std::cerr << std::hex << c << std::endl;
    }

    // ****

    device::pr655 meter;

    try {
        meter = device::pr655::open(device);

        meter.start();
        std::cerr << meter.model_number();
        std::cerr << " is ready!" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "E: " << e.what() << std::endl;
        meter.stop();
        return -1;
    }

    // ****

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);

    gl::monitor mtarget = gl::monitor::primary();

    if (!mdev.empty()) {

        std::vector<gl::monitor> mm = gl::monitor::monitors();

        const char *cstr = mdev.c_str();
        char *cend;
        unsigned long k = strtoul(cstr, &cend, 10);
        if (cstr != cend) {
            if (k >= mm.size()) {
                std::cerr << "monitor index out of range" << std::endl;
                return -1;
            }

            mtarget = mm[k];
        } else {
            auto pos = std::find_if(mm.begin(), mm.end(), [mdev](const gl::monitor &m) {
                return m.name() == mdev;
            });

            if (pos == mm.end()) {
                std::cerr << "could not find monitor" << std::endl;
                return 0;
            }

            mtarget = *pos;
        }
    }

    std::cout << "Monitor: " << mtarget.name() << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 2);

    // *****

    robot bender(mtarget, meter, colors, gray_level);

    // **
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);

    // **** and so it begins ...

    std::cerr << "All ready! Press <space> to start measurement..." << std::endl;


    bool keep_looping = true;
    while (keep_looping && !bender.should_close()) {

        keep_looping = bender.render();

        bender.swap_buffers();
        glfwPollEvents();
    }

    meter.stop();
    dump_stdout(bender);
    save_data_h5("spectra.h5", bender);
    bender.stop();
    bender = nullptr;

    std::cerr << "Goodbay. Have a nice day!" << std::endl;

    glfwTerminate();
    return 0;
}
