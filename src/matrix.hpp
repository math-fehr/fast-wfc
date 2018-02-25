#pragma once

#include <vector>
#include <ostream>
#include "assert.h"

using namespace std;

template<typename T>
class Matrix {
public:
  unsigned height;
  unsigned width;
  vector<T> data;

  Matrix() {
    width = 0;
    height = 0;
    data = vector<T>();
  }

  Matrix(unsigned width, unsigned height) {
    this->width = width;
    this->height = height;
    this->data = vector<T>(width * height);
  }

  Matrix(const Matrix& m) {
    this->width = m.width;
    this->height = m.height;
    this->data = m.data;
  }

  const T& get(unsigned i, unsigned j) const {
    return data[j + i * width];
  }

  T& get(unsigned i, unsigned j) {
    return data[j + i * width];
  }

  void set(unsigned i, unsigned j, T value) {
    data[j + i * width] = value;
  }

  Matrix<T> reflected() {
    Matrix<T> result = Matrix<T>(width, height);
    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width; x++) {
        result.set(y, x, get(y, width - 1 - x));
      }
    }
    return result;
  }

  Matrix<T> rotated() {
    Matrix<T> result = Matrix<T>(height, width);
    for(unsigned y = 0; y < width; y++) {
      for(unsigned x = 0; x < height; x++) {
        result.set(y,x, get(x, width - 1 - y));
      }
    }
    return result;
  }

  Matrix<T> get_sub_matrix(unsigned y, unsigned x, unsigned sub_width, unsigned sub_height) {
    Matrix<T> sub_matrix = Matrix<T>(sub_width, sub_height);
    for(unsigned ki = 0; ki < sub_height; ki++) {
      for(unsigned kj = 0; kj < sub_width; kj++) {
        sub_matrix.get(ki, kj) = get((y+ki) % height, (x+kj) % width);
      }
    }
    return sub_matrix;
  }

  bool operator==(const Matrix& m) const {
    if(height != m.height) {
      return false;
    }
    if(width != m.width) {
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


template <typename T>
ostream& operator<<(std::ostream& os, const Matrix<T>& m)
{
  os << "[";
  for(unsigned i = 0; i<m.height; i++) {
    os << "[";
    for(unsigned j = 0; j<m.width; j++) {
      os << m.get(i,j) << ",";
    }
    os << "],";
  }
  os << "]";
  return os;
}

namespace std {
  template<typename T>
  class hash<Matrix<T>> {
  public:
    size_t operator()(const Matrix<T> &m) const
    {
        std::size_t seed = m.data.size();
        for(const T& i: m.data) {
          seed ^= unsigned(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
  };
}
