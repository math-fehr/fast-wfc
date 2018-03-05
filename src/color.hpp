#pragma once

#include <iostream>
#include <vector>

using namespace std;

struct Color {
  unsigned char r,g,b;

  bool operator==(const Color& c) const {
    return r == c.r && g == c.g && b == c.b;
  }

  bool operator!=(const Color& c) const {
    return !(c == *this);
  }
};

namespace std {
  template<>
  class hash<Color> {
  public:
    size_t operator()(const Color& c) const {
      return c.r + 256*c.g + 256*256*c.b;
    }
  };
}

ostream& operator<<(std::ostream& os, const Color& c) {
  os << "(" << (unsigned)c.r << "," << (unsigned)c.g << "," << (unsigned)c.b << ")";
  return os;
}
