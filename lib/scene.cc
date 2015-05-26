#include <scene.h>

#include <glm/gtc/matrix_transform.hpp>

namespace gl = glue;

namespace iris {
namespace scene {

static const char vs_s2d[] = R"SHDR(
#version 140

in vec2 pos;

uniform mat4 viewport;

void main()
{
    gl_Position = viewport * vec4(pos, 0.0, 1.0);
}
)SHDR";

static const char fs_s2d[] = R"SHDR(
#version 140

out vec4 finalColor;
uniform vec4 plot_color;

void main() {
    finalColor = plot_color;
}
)SHDR";


//rectangle

rectangle::rectangle(const glue::rect &r, const glue::color::rgba &paint)
        : rect(r), paint(paint) {
}


void rectangle::fg_color(const glue::color::rgba &color) {
    paint = color;
}

void rectangle::init() {
    vs = gl::shader::make(vs_s2d, GL_VERTEX_SHADER);
    fs = gl::shader::make(fs_s2d, GL_FRAGMENT_SHADER);

    vs.compile();
    fs.compile();

    prg = gl::program::make();
    prg.attach({vs, fs});
    prg.link();

    std::vector<float> box = {
            0.0f,  1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,

            1.0f,  1.0f,
            0.0f,  1.0f,
            1.0f, 0.0f};

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

void rectangle::draw(glm::mat4 vp) {
    glm::mat4 S = glm::scale(glm::mat4(1), glm::vec3(rect.width, rect.height, 0.0f));
    glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(rect.x, rect.y, 0.0f));

    glm::mat4 mvp = vp * T * S;

    prg.use();
    prg.uniform("plot_color", paint);
    prg.uniform("viewport", mvp);

    va.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    va.unbind();
    prg.unuse();
}



} // iris::scene::
} // iris::