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

// label

static const char vs_text[] = R"SHDR(
#version 140

in vec4 coord;
out vec2 texpos;
uniform sampler2D tex;
uniform mat4 transform;

void main(void) {
  gl_Position = transform * vec4(coord.xy, 0, 1);
  texpos = coord.zw / textureSize(tex, 0);
}
)SHDR";

static const char fs_text[] = R"SHDR(
#version 140

in vec2 texpos;
uniform sampler2D tex;
uniform vec4 color;

out vec4 finalColor;

void main(void) {
  finalColor = vec4(1, 1, 1, texture(tex, texpos).r) * color;
}
)SHDR";


label::label(const glue::tf_font &font, const std::string &str, size_t font_size)
        : font(font), str(str), fsize(font_size) {

}

void label::init() {
    std::u32string u32str = glue::u8to32(str);

    struct coords_point {
        float x;
        float y;
        float s;
        float t;
    };

    prg = glue::program::make();

    glue::shader vs = glue::shader::make(vs_text, GL_VERTEX_SHADER);
    glue::shader fs = glue::shader::make(fs_text, GL_FRAGMENT_SHADER);

    vs.compile();
    fs.compile();

    prg.attach({vs, fs});
    prg.link();

    prg.use();

    glue::tf_atlas &ch_atlas = font.atlas_for_size(fsize);

    ch_atlas.bind();

    vbuffer = glue::buffer::make();
    varray = glue::vertex_array::make();

    vbuffer.bind();
    varray.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    float x = 0;
    float y = 0;
    std::vector<coords_point> coords(6*u32str.size());

    int &c = ntriag = 0;

    for (char32_t ch : u32str) {
        const glue::glyph_tf ci = ch_atlas[ch];

        float x2 = x + ci.left;
        float y2 = y - ci.top;

        x += ci.ax;
        y += ci.ay;

        float w = ci.width;
        float h = ci.height;

        if (!w || !h)
            continue;

        coords[c++] = {x2,     y2,       ci.u,     ci.v};
        coords[c++] = {x2 + w, y2,       ci.u + w, ci.v};
        coords[c++] = {x2,     y2 + h,   ci.u,     ci.v + h};
        coords[c++] = {x2 + w, y2,       ci.u + w, ci.v};
        coords[c++] = {x2,     y2 + h,   ci.u,     ci.v + h};
        coords[c++] = {x2 + w, y2 + h,   ci.u + w, ci.v + h};
    }

    mysize = glue::extent(0.0f, 0.0f);
    float y_min, y_max, x_min, x_max;
    y_min = y_max = x_min = x_max = 0.0f;

    for (coords_point p : coords) {
        mysize.width = std::max(mysize.width, p.x);
        y_min = std::min(y_min, p.y);
        y_max = std::max(y_max, p.y);
        x_min = std::min(x_min, p.x);
        x_max = std::max(x_max, p.x);
    }

    mysize.height = y_max + -y_min;
    mysize.width = x_max - x_min;

    for (coords_point &p : coords) {
        p.x -= x_min;
        p.y -= y_min;
    }

    vbuffer.data(coords);

    varray.unbind();
    vbuffer.unbind();

    prg.unuse();
}

void label::draw(const glm::mat4 &vp) {

    if (!prg) {
        return;
    }

    prg.use();
    varray.bind();

    glue::tf_atlas &atlas = font.atlas_for_size(fsize);
    atlas.bind();
    prg.uniform("transform", vp);
    prg.uniform("tex", 0); //texture unit
    prg.uniform("color", glue::color::rgba(0.0f, 0.0f, 0.0f, 1.0f));

    glDrawArrays(GL_TRIANGLES, 0, ntriag);

    varray.unbind();
    prg.unuse();
}

} // iris::scene::
} // iris::