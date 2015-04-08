#ifndef GLUE_TEXT_H
#define GLUE_TEXT_H

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

class tf_atlas {
public:
    struct char_info {

        char ch;     // the char

        float ax;    // advance.x
        float ay;    // advance.y

        float bw;    // bitmap.width;
        float bh;    // bitmap.height;

        float bl;    // bitmap_left;
        float bt;    // bitmap_top;

        float tx;    // x offset of glyph in texture coordinates
        float ty;    // y offset of glyph in texture coord

        char_info(const FT_GlyphSlot &g, char ch, float tx, float ty);
    };

    tf_atlas() : index(), font_tex() { }
    tf_atlas(std::vector<char_info> nfo, texture ft_tex) : index(nfo), font_tex(ft_tex) { }

    const char_info &operator[](char ch) const;

    void bind() {
        font_tex.bind(GL_TEXTURE_2D);
    }

    int w, h;

private:
    std::vector<char_info> index;
    texture font_tex;
};

class tf_font {
private:
    tf_font(FT_Face the_face) : face(the_face), atlases(std::make_shared<atlas_map>()) { }

public:
    tf_font() : face(nullptr), atlases(nullptr) { }
    tf_font(const tf_font &other) : face(other.face) {
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
};



}

#endif //IRIS_TEXT_H
