#pragma once

using namespace std;
#include <vector>
#include <stdint.h>

class Wave {
private:
  vector<uint8_t> data;
  unsigned width;
  unsigned height;
  unsigned pattern_size;
public:
  Wave() {}


  Wave(unsigned width, unsigned height) : width(width), height(height), pattern_size(0) {
  }

  void init(unsigned pattern_size) {
    data = vector<uint8_t>(width * height * pattern_size, 1);
    this->pattern_size = pattern_size;
  }

  bool get(unsigned index, unsigned pattern) {
    return data[index * pattern_size + pattern];
  }

  void set(unsigned index, unsigned pattern, bool value) {
    data[index * pattern_size + pattern] = value;
  }

  bool get(unsigned i, unsigned j, unsigned pattern) {
    return get(i * width + j, pattern);
  }

  void set(unsigned i, unsigned j, unsigned pattern, bool value) {
    set(i * width + j, pattern, value);
  }

  unsigned get_width() {
    return width;
  }

  unsigned get_height() {
    return height;
  }

  unsigned get_size() {
    return width * height;
  }
};

/*
class Wave {
private:
  vector<uint8_t> data;
  unsigned width;
  unsigned height;
  unsigned pattern_size;
public:
  Wave() {}

  Wave(unsigned width, unsigned height) : width(width), height(height), pattern_size(0) {
  }

  void init(unsigned pattern_size) {
    data = vector<uint8_t>(width * height * ((pattern_size + 7) / 8), 0b11111111);
    this->pattern_size = (pattern_size + 7) / 8;
  }

  bool get(unsigned index, unsigned pattern) {
    return data[index * pattern_size + pattern / 8] & (1 << (pattern % 8));
  }

  void set(unsigned index, unsigned pattern, bool value) {
    if(!value) {
      data[index * pattern_size + pattern / 8] &= ~(1 << (pattern % 8));
    } else {
      data[index * pattern_size + pattern / 8] |= 1 << (pattern % 8);
    }
  }

  bool get(unsigned i, unsigned j, unsigned pattern) {
    return get(i * width + j, pattern);
  }

  void set(unsigned i, unsigned j, unsigned pattern, bool value) {
    set(i * width + j, pattern, value);
  }

  unsigned get_width() {
    return width;
  }

  unsigned get_height() {
    return height;
  }

  unsigned get_size() {
    return width * height;
  }
};*/
