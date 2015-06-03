
#include <iris.h>

#include <glue/basic.h>
#include <glue/window.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <dkl.h>
#include <misc.h>

#include <numeric>

#include <boost/program_options.hpp>

namespace gl = glue;

static const char vs_simple[] = R"SHDR(
#version 140

in vec2 pos;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(pos, 0.0, 1.0);
}
)SHDR";

static const char fs_simple[] = R"SHDR(
#version 140

out vec4 finalColor;
uniform vec4 plot_color;

void main() {
    finalColor = plot_color;
}
)SHDR";


std::vector<double> make_phi_circle(size_t n) {
    double step = 2 * M_PI / n;
    std::vector<double> phi(n, 0);
    std::iota(phi.begin(), phi.end(), 0.0);
    std::transform(phi.begin(), phi.end(), phi.begin(), [step](const double v) {
        return v*step;
    });

    return phi;
}

struct box {

    void init() {
        vs = gl::shader::make(vs_simple, GL_VERTEX_SHADER);
        fs = gl::shader::make(fs_simple, GL_FRAGMENT_SHADER);

        vs.compile();
        fs.compile();

        prg = gl::program::make();
        prg.attach({vs, fs});
        prg.link();

        std::vector<float> box = { -1.0f,  1.0f,
                -1.0f, -1.0f,
                1.0f, -1.0f,

                1.0f,  1.0f,
                -1.0f,  1.0f,
                1.0f, -1.0f};

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
    }

    void render(glm::mat4 mvp, gl::color::rgba color) {
        prg.use();
        prg.uniform("plot_color", color);
        prg.uniform("mvp", mvp);

        va.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        va.unbind();
        prg.unuse();
    }

    //

private:
    gl::shader vs;
    gl::shader fs;

    gl::program prg;

    gl::buffer bb;
    gl::vertex_array va;
};

class colorcircle : public gl::window {
public:
    colorcircle(int height, int width, const std::string &title, iris::dkl &cspace)
            : window(height, width, title, gl::monitor{}), colorspace(cspace) {
        make_current_context();
        glfwSwapInterval(1);

        circ_phi = iris::linspace(0.0, 2*M_PI, 16);
        circ_rgb.resize(circ_phi.size());

        update_colors();

        the_box.init();
        debug = false;
    }

    void update_colors() {
        std::transform(circ_phi.cbegin(), circ_phi.cend(), circ_rgb.begin(), [&](const double p){
            iris::rgb crgb = colorspace.iso_lum(p, c);
            uint8_t creport;
            iris::rgb res = crgb.clamp(&creport);
            if (creport != 0) {
                std::cerr << "[W] color clamped: " << crgb << " → " << res << " @ c: " << c << std::endl;
            }
            return res;

        });

    }

    void render();

    virtual void pointer_moved(glue::point pos) override;
    virtual void key_event(int key, int scancode, int action, int mods) override;

    iris::rgb fg = iris::rgb::gray(0.65f);
    iris::dkl &colorspace;
    gl::point cursor;
    float gain = 0.0001;
    float stimsize = 0.05f;
    double phi = 0.0;
    double c = 0.1;

    std::vector<double> circ_phi;
    std::vector<iris::rgb> circ_rgb;

    box the_box;
    bool debug;
};


void colorcircle::pointer_moved(gl::point pos) {
    gl::window::pointer_moved(pos);

    float x = cursor.x - pos.x;

    cursor = pos;
    phi += x * gain;
    phi = fmod(phi + (2.0f * M_PI), (2.0f * M_PI));

    fg = colorspace.iso_lum(phi, c);


    if (debug) {
        std::cerr << phi << ", " << fg << std::endl;
    }
}

void colorcircle::key_event(int key, int scancode, int action, int mods) {
    window::key_event(key, scancode, action, mods);

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        c -= 0.01;
        std::cout << "↓ " << c << std::endl;
        update_colors();
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        c += 0.01;
        std::cout << "↑ " << c << std::endl;
        update_colors();
    } else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        std::cout << "c: " << c << std::endl;
        for(size_t i = 0; i < circ_phi.size(); i++) {
            std::cout << circ_phi[i] << " → " << circ_rgb[i] << std::endl;
        }
    } else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        stimsize *= 0.5f;
    } else if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        stimsize *= 2.0f;
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        debug = true;
    }
}


void colorcircle::render() {
    gl::extent fb = framebuffer_size();

    phi += M_PI/180;
    phi = fmod(phi + (2.0f * M_PI), (2.0f * M_PI));
    fg = colorspace.iso_lum(phi, c);


    glm::mat4 vp;
    if (fb.height > fb.width) {
        float scale = fb.width / fb.height;
        vp = glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0f));
    } else {
        float scale = fb.height / fb.width;
        vp = glm::scale(glm::mat4(1), glm::vec3(scale, 1.0f, 1.0f));
    }

    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const std::vector<double> &phi = circ_phi;
    const std::vector<iris::rgb> &pc = circ_rgb;

    for (size_t i = 0; i < phi.size(); i++) {

        float x = (float) std::cos(phi[i]) * 0.9f;
        float y = (float) std::sin(phi[i]) * 0.9f;

        glm::mat4 tscale = glm::scale(glm::mat4(1), glm::vec3(0.05f, 0.05f, 0));
        glm::mat4 ttrans = glm::translate(glm::mat4(1), glm::vec3(x, y, 0.0f));
        glm::mat4 t = ttrans * tscale;

        glm::mat4 fin = vp * t;

        gl::color::rgba color(pc[i]);

        the_box.render(fin, color);
    }

    {
        glm::mat4 tscale = glm::scale(glm::mat4(1), glm::vec3(stimsize, stimsize, 0));
        glm::mat4 fin = vp * tscale;

        gl::color::rgba color(fg);
        the_box.render(fin, color);
    }
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string sid;
    bool grab_mouse = false;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("subject,S", po::value<std::string>(&sid))
            ("grab-mouse,m", po::value<bool>(&grab_mouse));

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
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

    // if we are running in windowed mode with > 1 monitors
    // attached this is all not right ...
    iris::data::store store = iris::data::store::default_store();
    std::string mdev = store.default_monitor();

    iris::data::monitor moni = store.load_monitor(mdev);
    iris::data::monitor::mode mode = moni.default_mode;
    iris::data::display display = store.make_display(moni, mode, "gl");
    iris::data::rgb2lms rgb2lms = store.load_rgb2lms(display);

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

    if (!glfwInit()) {
        return -1;
    }

    colorcircle wnd = colorcircle(800, 1200, "Colorcircle", cspace);

    if (grab_mouse) {
        wnd.disable_cursor();
    }

    while (! wnd.should_close()) {

        wnd.render();

        wnd.swap_buffers();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

