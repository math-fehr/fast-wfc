#pragma once

#include <iostream>

using namespace std;

struct Color {
  unsigned char r,g,b;
  bool operator==(const Color& c) const {
    return r == c.r && g == c.g && b == c.b;
  }
  bool operator!=(const Color& c) const {
    return !(c == *this);
  }
  explicit operator unsigned() const {
    return r + 256 * g + 256*256*b;
  }
};

ostream& operator<<(std::ostream& os, const Color& c) {
  os << "(" << (unsigned)c.r << "," << (unsigned)c.g << "," << (unsigned)c.b << ")";
  return os;
}
