
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

tf_atlas::tf_atlas(size_t font_size)
        : font_size(font_size), pos(), size(), rowh(0), index(), font_tex() {
  int w = 1024;
  int h = static_cast<int>(font_size) * 2;

  glActiveTexture(GL_TEXTURE0);

  font_tex = texture::make();
  font_tex.bind(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
  texture::pixel_store(GL_UNPACK_ALIGNMENT, 1);

  texture::parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  texture::parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  texture::parameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture::parameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  size.width = w;
  size.height = h;
}

const glyph_tf& tf_atlas::operator[](char ch) const {
  for (const glyph_tf &info : index) {
    if (info.ch == ch) {
      return info;
    }
  }

  throw std::runtime_error("Could not find char");
}

void tf_atlas::add_glyphs(const std::vector<glyph_bmp> &glyphs) {

  glActiveTexture(GL_TEXTURE0);
  font_tex.bind(GL_TEXTURE_2D);

  for(const glyph_bmp &bmp : glyphs) {

    //"line wrap"
    if (pos.x + bmp.width + 1 >= size.width) {
      pos.y += rowh;
      rowh = bmp.height;
      pos.x = 0;
    } else {
      rowh = std::max(rowh, bmp.height);
    }

    if ((pos.y + rowh) > size.height) {
      std::vector<uint8_t> data(size.height * size.width);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

      size.height += (pos.y + rowh);
      data.resize((size.height * size.width));

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size.width, size.height,
                   0, GL_RED, GL_UNSIGNED_BYTE, data.data());
      //std::cerr << "Increasing tf atlas size: " << size.width << " " << size.height << std::endl;
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    static_cast<int>(pos.x),
                    static_cast<int>(pos.y),
                    static_cast<int>(bmp.width),
                    static_cast<int>(bmp.height),
                    GL_RED, GL_UNSIGNED_BYTE,
                    bmp.bytes.data());

    index.emplace_back(bmp, pos.x, pos.y);

    pos.x += bmp.width + 1.0f;
  }

}


tf_atlas tf_font::make_atlas(size_t size, const std::string &characters) {
  FT_Set_Pixel_Sizes(face, 0, size);
  FT_GlyphSlot g = face->glyph;

  tf_atlas the_atlas(size);

  std::vector<glyph_bmp> glyphs;
  for (const auto &ch : characters) {
    if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) {
      fprintf(stderr, "Loading character %c failed!\n", ch);
      continue;
    }

    glyph_bmp bmp;
    bmp.ch = ch;
    bmp.ax = g->advance.x >> 6;
    bmp.ay = g->advance.y >> 6;

    bmp.width = g->bitmap.width;
    bmp.height = g->bitmap.rows;
    bmp.left = g->bitmap_left;
    bmp.top = g->bitmap_top;

    size_t nbytes = static_cast<size_t>(bmp.width * bmp.height);
    bmp.bytes.resize(nbytes, 0);
    memcpy(bmp.bytes.data(), g->bitmap.buffer, nbytes);

    glyphs.emplace_back(std::move(bmp));
  }

  the_atlas.add_glyphs(glyphs);
  return the_atlas;
}


tf_atlas& tf_font::atlas_for_size(size_t size) {
  auto iter = atlases->find(size);

  if (iter == atlases->end()) {
    tf_atlas atlas = make_atlas(size, "abcdefghijklmnopqrstuvwxyz");
    auto res = atlases->insert(std::make_pair(size, atlas));
    iter = res.first;
  }

  return iter->second;
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
    const glyph_tf ci = ch_atlas[ch];

    float x2 = x + ci.left;
    float y2 = -y - ci.top;

    x += ci.ax;
    y += ci.ay;

    float w = ci.width;
    float h = ci.height;

    if (!w || !h)
      continue;

    coords[c++] = {x2,     -y2,       ci.u,     ci.v};
    coords[c++] = {x2 + w, -y2,       ci.u + w, ci.v};
    coords[c++] = {x2,     -y2 - h,   ci.u,     ci.v + h};
    coords[c++] = {x2 + w, -y2,       ci.u + w, ci.v};
    coords[c++] = {x2,     -y2 - h,   ci.u,     ci.v + h};
    coords[c++] = {x2 + w, -y2 - h,   ci.u + w, ci.v + h};
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
  txt.atlas = ch_atlas;

  return txt;
}

void text::draw(glm::mat4 transform) {
  prg.use();
  varray.bind();

  atlas.bind();
  prg.uniform("transform", transform);
  glDrawArrays(GL_TRIANGLES, 0, ntriag);

  varray.unbind();
  prg.unuse();
}

} // glue::