
#include <glue/text.h>

#include <array>
#include <thread>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace glue {

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


tf_atlas::char_info::char_info(const FT_GlyphSlot &g, char ch, float tx, float ty) {
  this->ch = ch;

  ax = g->advance.x >> 6;
  ay = g->advance.y >> 6;

  bw = g->bitmap.width;
  bh = g->bitmap.rows;

  bl = g->bitmap_left;
  bt = g->bitmap_top;

  this->tx = tx;
  this->ty = ty;
}


const tf_atlas::char_info& tf_atlas::operator[](char ch) const {
  for (const char_info &info : index) {
    if (info.ch == ch) {
      return info;
    }
  }

  throw std::runtime_error("Could not find char");
}


tf_atlas tf_font::make_atlas(size_t size, const std::string &characters) {
  FT_Set_Pixel_Sizes(face, 0, size);
  FT_GlyphSlot g = face->glyph;

  unsigned h, w;
  h = w = 0;

  unsigned int roww = 0;
  unsigned int rowh = 0;

  for (const auto &ch : characters) {
    if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) {
      std::cerr << "Loading character [" << ch << "] failed!" << std::endl;
      continue;
    }


    if (roww + g->bitmap.width + 1 >= 1024) {
      w = std::max(w, roww);
      h += rowh;
      roww = 0;
      rowh = 0;
    }

    roww += g->bitmap.width + 1;
    rowh = std::max(rowh, g->bitmap.rows);
  }

  w = std::max(w, roww);
  h += rowh;

  glActiveTexture(GL_TEXTURE0);

  texture font_tex = texture::make();
  font_tex.bind(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int ox = 0;
  int oy = 0;

  rowh = 0;

  std::vector<tf_atlas::char_info> ci;
  for (const auto &ch : characters) {
    if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) {
      fprintf(stderr, "Loading character %c failed!\n", ch);
      continue;
    }

    if (ox + g->bitmap.width + 1 >= 1024) {
      oy += rowh;
      rowh = 0;
      ox = 0;
    }

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    ox, oy,
                    g->bitmap.width, g->bitmap.rows,
                    GL_RED, GL_UNSIGNED_BYTE,
                    g->bitmap.buffer);

    ci.emplace_back(g, ch, ox, oy);

    rowh = std::max(rowh, g->bitmap.rows);
    ox += g->bitmap.width + 1;
  }

  fprintf(stderr, "Generated a %d x %d (%f kb) texture font atlas\n", w, h, w * h / 1024.0);
  tf_atlas the_atlas(ci, font_tex);

  the_atlas.w = w;
  the_atlas.h = h;

  return the_atlas;
}


FT_Library tf_font::ft_library() {
  static FT_Library lib;
  static std::once_flag flag;

  std::call_once(flag, [](){
      std::cout << "Init freetype!" << std::endl;
      if (FT_Init_FreeType(&lib)) {
        throw std::runtime_error("Could not init freetype library");
      }
  });

  return lib;
}



text text::make(tf_atlas &ch_atlas, const std::string &str) {


  struct coords_point {
    float x;
    float y;
    float s;
    float t;
  };

  program prg = program::make();

  shader vs = shader::make(vs_text, GL_VERTEX_SHADER);
  shader fs = shader::make(fs_text, GL_FRAGMENT_SHADER);

  vs.compile();
  fs.compile();

  prg.attach({vs, fs});
  prg.link();

  prg.use();

  ch_atlas.bind();

  prg.uniform("tex", 0); //texture unit
  prg.uniform("color", color::rgba(0.0f, 0.0f, 0.0f, 1.0f));

  buffer vbuffer = buffer::make();
  vertex_array varray = vertex_array::make();

  vbuffer.bind();
  varray.bind();

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  float x = 0;
  float y = 0;
  std::vector<coords_point> coords(6*str.size());

  int c = 0;
  for (char ch : str) {
    const tf_atlas::char_info ci = ch_atlas[ch];

    float x2 = x + ci.bl;
    float y2 = -y - ci.bt;

    x += ci.ax;
    y += ci.ay;

    float w = ci.bw;
    float h = ci.bh;

    if (!w || !h)
      continue;

    coords[c++] = {x2,     -y2,       ci.tx,         ci.ty};
    coords[c++] = {x2 + w, -y2,       ci.tx + ci.bw, ci.ty};
    coords[c++] = {x2,     -y2 - h,   ci.tx,         ci.ty + ci.bh};
    coords[c++] = {x2 + w, -y2,       ci.tx + ci.bw, ci.ty};
    coords[c++] = {x2,     -y2 - h,   ci.tx,         ci.ty + ci.bh};
    coords[c++] = {x2 + w, -y2 - h,   ci.tx + ci.bw, ci.ty + ci.bh};
  }

  vbuffer.data(coords);

  varray.unbind();
  vbuffer.unbind();

  prg.unuse();

  text txt;
  txt.prg = prg;
  txt.varray = varray;
  txt.vbuffer = vbuffer;
  txt.body = str;
  txt.ntriag = c;

  return txt;
}

void text::draw(glm::mat4 transform) {
  prg.use();
  varray.bind();

  prg.uniform("transform", transform);
  glDrawArrays(GL_TRIANGLES, 0, ntriag);

  varray.unbind();
  prg.unuse();
}

} // glue::