#include <unordered_map>
#include <vector>
#include <iostream>
#include "matrix.hpp"
#include "wfc.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;

struct Color {
  unsigned char r,g,b;
  bool operator==(const Color& c) const {
    return r == c.r && g == c.g && b == c.b;
  }
  bool operator!=(const Color& c) const {
    return !(c == *this);
  }
  operator unsigned() const {
    return r + 256 * g + 256*256*b;
  }
};

Matrix<Color> read_file(const string& file_path) {
  int width;
  int height;
  int num_components;
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 3);
  Matrix<Color> m = Matrix<Color>(width, height);
  for(unsigned i = 0; i < (unsigned)height; i++) {
    for(unsigned j = 0; j < (unsigned)width; j++) {
      unsigned index = 3 * (i * width + j);
      m.data[i * width + j] = {data[index], data[index + 1], data[index + 2]};
    }
  }
  free(data);
  return m;
}

void write_file(const string& file_path, const Matrix<Color>& m) {
  stbi_write_bmp(file_path.c_str(), m.width, m.height, 3, (const unsigned char*)m.data.data());
}

int main() {
  Matrix<Color> m = read_file("samples/cave.bmp");
  WFC<Color> wfc = WFC<Color>(m, 48, 48, 3, 3);
  wfc.run();
  write_file("out.bmp", wfc.output);
}
