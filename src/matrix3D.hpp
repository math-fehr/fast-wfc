#pragma once

#include <vector>

using namespace std;

template<typename T>
class Matrix3D {
public:
  unsigned height;
  unsigned width;
  unsigned depth;

  vector<T> data;

  Matrix3D() {
    width = 0;
    height = 0;
    depth = 0;
    data = vector<T>();
  }

  Matrix3D(unsigned width, unsigned height, unsigned depth) {
    this->width = width;
    this->height = height;
    this->depth = depth;
    this->data = vector<T>(width * height * depth);
  }

  Matrix3D(const Matrix3D& m) {
    this->width = m.width;
    this->height = m.height;
    this->depth = m.depth;
    this->data = m.data;
  }

  const T& get(unsigned i, unsigned j, unsigned k) const {
    return data[i * width * depth + j * depth + k];
  }

  T& get(unsigned i, unsigned j, unsigned k) {
    return data[i * width * depth + j * depth + k];
  }

  void set(unsigned i, unsigned j, unsigned k, T value) {
    data[i * width * depth + j * depth + k] = value;
  }

  bool operator==(const Matrix3D& m) const {
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
