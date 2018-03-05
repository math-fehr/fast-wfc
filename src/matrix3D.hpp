#pragma once

#include <vector>
#include "assert.h"

using namespace std;

template<typename T>
class Matrix3D {
public:
  const unsigned height;
  const unsigned width;
  const unsigned depth;

  vector<T> data;

  Matrix3D(unsigned height, unsigned width, unsigned depth) noexcept :
    height(height), width(width), depth(depth), data(width * height * depth) {
  }

  const T& get(unsigned i, unsigned j, unsigned k) const noexcept {
    return data[i * width * depth + j * depth + k];
  }

  T& get(unsigned i, unsigned j, unsigned k) noexcept {
    return data[i * width * depth + j * depth + k];
  }

  void set(unsigned i, unsigned j, unsigned k, T value) noexcept {
    data[i * width * depth + j * depth + k] = value;
  }

  bool operator==(const Matrix3D& m) const noexcept {
    if(height != m.height) {
      return false;
    }
    if(width != m.width) {
      return false;
    }
    if(depth != m.depth) {
      return false;
    }
    for(unsigned i = 0; i<data.size(); i++) {
      if(m.data[i] != data[i]) {
        return false;
      }
    }
    return true;
  }
};
