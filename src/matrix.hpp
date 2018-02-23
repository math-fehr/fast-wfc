#pragma once

#include <vector>
#include <ostream>

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

  const T& operator[](const unsigned& i) const {
    return data[i];
  }

  T& operator[](const unsigned& i) {
    return data[i];
  }
};


template <typename T>
ostream& operator<<(std::ostream& os, const Matrix<T>& m)
{
  os << "[";
  for(unsigned i = 0; i<m.height; i++) {
    os << "[";
    for(unsigned j = 0; j<m.width; j++) {
      os << m.data[i * m.width + j] << ",";
    }
    os << "],\n";
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
