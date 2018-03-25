#pragma once

#include <functional>
#include "array2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"


/**
 * Represent a 24-bit rgb color.
 */
struct Color {
  unsigned char r,g,b;

  bool operator==(const Color& c) const noexcept {
    return r == c.r && g == c.g && b == c.b;
  }

  bool operator!=(const Color& c) const noexcept {
    return !(c == *this);
  }
};


/**
 * Hash function for color.
 */
namespace std {
  template<>
  class hash<Color> {
  public:
    size_t operator()(const Color& c) const {
      return (size_t)c.r + (size_t)256*(size_t)c.g + (size_t)256*(size_t)256*(size_t)c.b;
    }
  };
}


/**
 * Read an image.
 */
Array2D<Color> read_image(const string& file_path) noexcept {
  int width;
  int height;
  int num_components;
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 3);
  assert(data != nullptr);
  Array2D<Color> m = Array2D<Color>(height, width);
  for(unsigned i = 0; i < (unsigned)height; i++) {
    for(unsigned j = 0; j < (unsigned)width; j++) {
      unsigned index = 3 * (i * width + j);
      m.data[i * width + j] = {data[index], data[index + 1], data[index + 2]};
    }
  }
  free(data);
  return m;
}

/**
 * Write an image in the png format.
 */
void write_image_png(const string& file_path, const Array2D<Color>& m) noexcept {
  stbi_write_png(file_path.c_str(), m.width, m.height, 3, (const unsigned char*)m.data.data(),0);
}
