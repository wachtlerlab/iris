#ifndef GLUE_TEXT_H
#define GLUE_TEXT_H

#include <glue/basic.h>
#include <glue/shader.h>
#include <glue/buffer.h>
#include <glue/arrays.h>
#include <glue/texture.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <string>
#include <map>
#include <memory>

namespace glue {

//tf stands for texture font

struct glyph {
    char32_t ch;

    //advance;
    float ax;
    float ay;

    //the dimensions
    float left;
    float top;
    float width;
    float height;
};

struct glyph_tf : glyph {

    glyph_tf(const glyph &src, float u, float v)
            : glyph(src), u(u), v(v) {
    }

    float u;
    float v;
};

struct glyph_bmp : glyph {
    std::vector<uint8_t> bytes;
};


class tf_atlas {
public:
    tf_atlas() : font_size(0) { };
    tf_atlas(size_t font_size);

    const glyph_tf &operator[](char32_t ch) const;

    void bind() {
        font_tex.bind(GL_TEXTURE_2D);
    }

    void add_glyphs(const std::vector<glyph_bmp> &glyphs);

private:
    size_t font_size;
    point pos;
    extent size;
    float rowh;
    std::vector<glyph_tf> index;
    texture font_tex;
};

class tf_font {
private:
    tf_font(FT_Face the_face) : face(the_face), atlases(std::make_shared<atlas_map>()) { }

public:
    tf_font() : face(nullptr), atlases(nullptr) { }
    tf_font(const tf_font &other) : face(other.face), atlases(other.atlases) {
        if (face) {
            FT_Reference_Face(face);
        }
    }

    static tf_font make(const std::string &path) {
        FT_Face face;
        if (FT_New_Face(ft_library(), path.c_str(), 0, &face)) {
            throw std::runtime_error("Could not open font %s");
        }
        return tf_font(face);
    }

    ~tf_font() {
        if (face) {
            FT_Done_Face(face);
        }
    }

    tf_atlas  make_atlas(size_t size, const std::string &characters);
    tf_atlas& atlas_for_size(size_t size);

    std::vector<glyph_bmp> glyphs_for(size_t size, const std::string &u8str);

private:
    static FT_Library ft_library();

private:
    FT_Face face;
    typedef std::map<size_t, tf_atlas> atlas_map;
    std::shared_ptr<atlas_map> atlases;
};


class text {
public:
    text() {}

    static text make(tf_atlas &ch_atlas, const std::string &str);
    void draw(glm::mat4 transform);

private:
    std::string body;

    program prg;
    buffer vbuffer;
    vertex_array varray;

    GLsizei ntriag;
    tf_atlas atlas;
};

std::u32string u8to32(const std::string &istr);

}

#endif //IRIS_TEXT_H
