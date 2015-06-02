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
#include <stdexcept>

#include <boost/program_options.hpp>
#include <misc.h>
#include <random>
#include <scene.h>
#include <fit.h>

namespace gl = glue;

class flicker_wnd : public gl::window {
public:
    flicker_wnd(const iris::data::rgb2lms &rgb2lms, const std::vector<double> &stimuli);
    void render();

    virtual void key_event(int key, int scancode, int action, int mods) override;
    virtual void pointer_moved(glue::point pos) override;

    void lum_change(float delta, float gain) {
        iris::rgb gray = dkl.reference_gray();
        gray = iris::rgb::gray(gray.r + delta * gain).clamp();
        dkl.reference_gray(gray);
        fg_angle(phi[stim_index]);
    }

    void fg_angle(double angle) {
        iris::rgb fg_color = dkl.iso_lum(phi[stim_index], contrast);
        fg.fg_color(fg_color);
    }

    bool success() const {
        return completed;
    }

    const std::vector<double>& response() const {
        return resp;
    }

    void update_label();

private:
    iris::dkl dkl;
    std::vector<double> phi;
    size_t stim_index;

    glm::mat4 vp;
    iris::scene::rectangle fg;
    float gray_level = 0.7;

    iris::scene::label progress;
    glm::mat4 px2gl;

    double clock;
    bool draw_stim;

    gl::point cursor;
    float mouse_gain;
    double contrast;

    bool completed;
    std::vector<double> resp;
};

flicker_wnd::flicker_wnd(const iris::data::rgb2lms &rgb2lms, const std::vector<double> &stimuli)
        : window(rgb2lms.dsy, "iris - isoslant"),
          dkl(rgb2lms.dkl_params, iris::rgb::gray(rgb2lms.gray_level)),
          phi(stimuli) {

    make_current_context();
    glfwSwapInterval(1);
    disable_cursor();

    gray_level = rgb2lms.gray_level;

    const float width = rgb2lms.width;
    const float height = rgb2lms.height;
    float stim_size = 40;

    vp = glm::ortho(0.f, width, height, 0.f);

    const float center_x = width * .5f - (stim_size * .5f);
    const float center_y = height * .5f - (stim_size * .5f);

    gl::rect bg_rect(center_x, center_y, stim_size, stim_size);

    fg = iris::scene::rectangle(bg_rect, gl::color::rgba::gray(1.0f));
    fg.init();

    clock = -std::numeric_limits<double>::infinity();
    draw_stim = false;

    stim_index = 0;
    contrast = 0.2;
    mouse_gain = 0.00001;
    completed = false;

    resp.resize(phi.size());

    fg_angle(phi[stim_index]);

    //setup the label
    iris::data::store store = iris::data::store::default_store();
    fs::file base = store.location();
    fs::file default_font = base.child("default.font").readlink();
    std::cerr << default_font.path() << " " << default_font.exists() << std::endl;
    if (default_font.exists()) {
        gl::tf_font font = gl::tf_font::make(default_font.path());
        gl::extent wsize = framebuffer_size();
        px2gl = glm::ortho(0.f, wsize.width, wsize.height, 0.f);
        progress = iris::scene::label(font, "isoslant", 16);
        progress.init();
        update_label();
    }

}


void flicker_wnd::update_label() {
    std::stringstream sstr;
    sstr << stim_index + 1 << " of " << phi.size();
    progress.text(sstr.str());
}

void flicker_wnd::render() {
    double now = glfwGetTime();
    double delta = now - clock;
    bool flip = delta > 0.05;

    if (flip) {
        draw_stim = !draw_stim;
        clock = now;

        glClearColor(gray_level, gray_level, gray_level, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (draw_stim) {
            fg.draw(vp);
        }

        progress.draw(px2gl);

        swap_buffers();
    }

    glfwPollEvents();
}


void flicker_wnd::key_event(int key, int scancode, int action, int mods) {
    window::key_event(key, scancode, action, mods);

    if (action != GLFW_PRESS) {
        return;
    }

    float gain = mods == GLFW_MOD_SHIFT ? .5f : 0.01f;
    if (key == GLFW_KEY_SPACE) {
        const double phi_adjusted = dkl.reference_gray().r;
        const double idx = stim_index++;

        resp[idx] = phi_adjusted;

        std::cerr << phi[idx] << ", " << phi_adjusted << std::endl;

        //reset reference point
        dkl.reference_gray(iris::rgb::gray(gray_level));

        if (stim_index < phi.size()) {
            fg_angle(phi[stim_index]);
        } else {
            completed = true;
            should_close(true);
        }

        update_label();

    } else if (key == GLFW_KEY_RIGHT) {
        lum_change(.1f, gain);
    } else if (key == GLFW_KEY_LEFT) {
        lum_change(-.1f, gain);
    }
}


void flicker_wnd::pointer_moved(glue::point pos) {
    float x = cursor.x - pos.x;
    float gain = mouse_gain;
    lum_change(x, gain);
    cursor = pos;
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    int exit_code = 0;

    try {
        size_t N = 16;
        size_t R = 4;
        double contrast = 0.16;
        float width = 0.0f;
        float height = 0.0f;
        bool use_stdout = false;

        po::options_description opts("colortilt experiment");
        opts.add_options()
                ("help", "produce help message")
                ("number,n", po::value<size_t>(&N), "number of colors to sample [default=16")
                ("repetition,r", po::value<size_t>(&R), "number of repetitions [default=4]")
                ("contrast,c", po::value<double>(&contrast)->required())
                ("stdout", po::value<bool>(&use_stdout));

        po::positional_options_description pos;
        po::variables_map vm;

        po::store(po::command_line_parser(argc, argv).options(opts).positional(pos).run(), vm);
        po::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << opts << std::endl;
            return 0;
        }

        gl::glue_start();

        iris::data::store store = iris::data::store::default_store();
        std::string mdev = store.default_monitor();

        iris::data::monitor moni = store.load_monitor(mdev);
        iris::data::monitor::mode mode = moni.default_mode;
        iris::data::display display = store.make_display(moni, mode, "gl");
        iris::data::rgb2lms rgb2lms = store.load_rgb2lms(display);

        std::vector<double> phi = iris::linspace(0.0, 2 * M_PI, N);
        std::vector<double> stim = iris::repvec(phi, R);

        std::random_device rnd_dev;
        std::mt19937 rnd_gen(rnd_dev());
        iris::block_shuffle(stim.begin(), stim.end(), N, rnd_gen);

        flicker_wnd wnd(rgb2lms, stim);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_MULTISAMPLE);

        const float gray_level = rgb2lms.gray_level;
        std::cerr << store.rgb2lms2yaml(rgb2lms) << std::endl;

        while (!wnd.should_close()) {
           wnd.render();
        }

        if (wnd.success()) {
            const std::vector<double> y = wnd.response();

            std::string tstamp = iris::make_timestamp();
            iris::data::isodata iso(iris::make_timestamp());
            iso.samples.resize(y.size());

            for (size_t i = 0; i < y.size(); i++) {
                iso.samples[i].stimulus = static_cast<float>(stim[i]);
                iso.samples[i].response = static_cast<float>(y[i]);
            }

            iso.display = display;

            std::string outstr = store.isodata2yaml(iso);
            if (use_stdout) {
                std::cout << outstr << std::endl;
            } else {
                fs::file outfile(iso.identifier() + ".isodata");
                outfile.write_all(outstr);
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "[E] Fail: " << e.what() << std::endl;
        exit_code = -2;
    } catch (...) {
        std::cerr << "very error, unkown fail, bad programmer, much sad :-(" << std::endl;
        exit_code = -1;
    }

    glfwTerminate();

    return exit_code;
}