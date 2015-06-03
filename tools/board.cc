#include <iris.h>

#include <glue/basic.h>
#include <glue/window.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <dkl.h>
#include <misc.h>

#include <numeric>
#include <thread>
#include <chrono>

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

class board : public gl::window {
public:
    board(const iris::data::display &display, iris::dkl &cspace)
            : window(display, "IRIS Board"), colorspace(cspace),
              rd(), gen(rd()), dis(0, 15)  {
        make_current_context();
        glfwSwapInterval(1);
        disable_cursor();

        circ_phi = iris::linspace(0.0, 2*M_PI, 16);
        circ_rgb.resize(circ_phi.size());

        update_colors();

        the_box.init();
        stamp = -100.0;
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

    virtual void key_event(int key, int scancode, int action, int mods) override;

    iris::dkl &colorspace;
    double phi = 0.0;
    double c = 0.1;

    std::vector<double> circ_phi;
    std::vector<iris::rgb> circ_rgb;

    box the_box;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<size_t> dis;

    double stamp;
};


void board::key_event(int key, int scancode, int action, int mods) {
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
    }
}


void board::render() {
    gl::extent fb = framebuffer_size();

    double now = glfwGetTime();

    if (now - stamp < 3.0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return;
    }

    //move to fb changed event
    glm::mat4 vp;
    if (fb.height > fb.width) {
        float scale = fb.width / fb.height;
        vp = glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0f));
    } else {
        float scale = fb.height / fb.width;
        vp = glm::scale(glm::mat4(1), glm::vec3(scale, 1.0f, 1.0f));
    }

    glm::mat4 vpi = glm::inverse(vp);

    glm::vec4 ll_gl(-1.0f, -1.0f, 0.0f, 0.0f);
    glm::vec4 ur_gl(1.0f, 1.0f, 0.0f, 0.0f);

    glm::vec4 ll_sc = vpi * ll_gl;
    glm::vec4 ur_sc = vpi * ur_gl;

    glm::vec4 sz_sc = ur_sc - ll_sc;

    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float box_size = sz_sc.x / (dis(gen) + 1.0f);

    for (float x = -0.5f * sz_sc.x; x <= 0.5 * sz_sc.x; x += box_size) {
        for (float y = -0.5f * sz_sc.y; y <= 0.5 * sz_sc.y; y += box_size) {
            glm::mat4 ttrans = glm::translate(glm::mat4(1), glm::vec3(x, y, 0.0f));
            glm::mat4 tscale = glm::scale(glm::mat4(1), glm::vec3(box_size, box_size, 0));
            glm::mat4 t = ttrans * tscale;
            glm::mat4 fin = vp * t;

            the_box.render(fin, circ_rgb[dis(gen)]);
        }
    }

    swap_buffers();
    stamp = now;
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    bool grab_mouse = false;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("grab-mouse,m", po::value<bool>(&grab_mouse));

    po::positional_options_description pos;

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
    iris::data::store store = iris::data::store::default_store();
    std::string mdev = store.default_monitor();

    iris::data::monitor moni = store.load_monitor(mdev);
    iris::data::monitor::mode mode = moni.default_mode;
    iris::data::display display = store.make_display(moni, mode, "gl");
    iris::data::rgb2lms rgb2lms = store.load_rgb2lms(display);

    iris::dkl::parameter params = rgb2lms.dkl_params;
    std::cerr << "Using rgb2sml calibration:" << std::endl;
    params.print(std::cerr);

    std::cerr << "[I] gray level: " << rgb2lms.gray_level << std::endl;
    iris::rgb refpoint = iris::rgb::gray(rgb2lms.gray_level);

    iris::dkl cspace(params, refpoint);

    gl::glue_start();

    board wnd(display, cspace);

    if (grab_mouse) {
        wnd.disable_cursor();
    }

    while (! wnd.should_close()) {

        wnd.render();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}