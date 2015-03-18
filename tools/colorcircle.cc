
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
#version 330

layout(location = 0) in vec2 pos;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(pos, 0.0, 1.0);
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

        circ_phi = iris::linspace(0.0, 2*M_PI, 16);
        circ_rgb.resize(circ_phi.size());

        update_colors();

        the_box.init();
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
    float gain = 0.005;
    double phi = 0.0;
    double c = 0.1;

    std::vector<double> circ_phi;
    std::vector<iris::rgb> circ_rgb;

    box the_box;
};


void colorcircle::pointer_moved(gl::point pos) {
    gl::window::pointer_moved(pos);

    float x = cursor.x - pos.x;
    float y = cursor.y - pos.y;

    bool s = std::signbit(x*y);

    float length = hypot(x, y);

    cursor = pos;
    phi += length * gain * (s ? -1.0 : 1.0);

    fg = colorspace.iso_lum(phi, c);
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
    }
}


void colorcircle::render() {
    gl::extent fb = framebuffer_size();

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
        glm::mat4 tscale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0));
        glm::mat4 fin = vp * tscale;

        gl::color::rgba color(fg);
        the_box.render(fin, color);
    }
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string ca_path;

    po::options_description opts("calibration tool");
    opts.add_options()
            ("help", "produce help message")
            ("calibration,c", po::value<std::string>(&ca_path)->required());

    po::positional_options_description pos;
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

    iris::dkl::parameter params = iris::dkl::parameter::from_csv(ca_path);
    std::cerr << "Using rgb2sml calibration:" << std::endl;
    params.print(std::cerr);

    iris::rgb refpoint(0.65f, 0.65f, 0.65f);
    iris::dkl cspace(params, refpoint);

    if (!glfwInit()) {
        return -1;
    }

    colorcircle wnd = colorcircle(800, 1200, "Test", cspace);

    while (! wnd.should_close()) {

        wnd.render();

        wnd.swap_buffers();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

