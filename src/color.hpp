#pragma once

#include <iostream>
#include <vector>

using namespace std;

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

namespace std {
  template<>
  class hash<Color> {
  public:
    size_t operator()(const Color& c) const {
      return (size_t)c.r + (size_t)256*(size_t)c.g + (size_t)256*(size_t)256*(size_t)c.b;
    }
  };
}
