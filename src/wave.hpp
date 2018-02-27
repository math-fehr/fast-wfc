#pragma once

#include <vector>
#include <stdint.h>
#include <limits>
#include <math.h>
#include <random>

using namespace std;

class Wave {
private:
  vector<uint8_t> data;
  vector<unsigned> patterns_frequencies;
  vector<double> plogp_patterns_frequencies;
  unsigned width;
  unsigned height;
  unsigned pattern_size;
public:
  Wave() {}


  Wave(unsigned width, unsigned height, const vector<unsigned>& patterns_frequencies) :
    patterns_frequencies(patterns_frequencies), width(width), height(height),
    pattern_size(patterns_frequencies.size())
  {
    for(unsigned i = 0; i < pattern_size; i++) {
      plogp_patterns_frequencies.push_back((double)patterns_frequencies[i] * log(patterns_frequencies[i]));
    }
    data = vector<uint8_t>(width * height * pattern_size, 1);
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

  int get_min_entropy(minstd_rand& gen) {
    std::uniform_real_distribution<> dis(0,1);
    double min = numeric_limits<double>::infinity();
    int argmin = -1;
    for(unsigned i = 0; i < get_size(); i++) {
      double sum = 0;
      int nb_possibilities = 0;
      for(unsigned k = 0; k < pattern_size; k++) {
        if(get(i,k)) {
          sum += patterns_frequencies[k];
          nb_possibilities++;
        }
      }

      if(sum == 0) {
        return -2;
      }

      double noise = dis(gen) * 1e-5;
      double entropy;
      double main_sum = 0;
      double log_sum = log(sum);

      if(nb_possibilities == 1) {
        entropy = 0;
      } else {
        for(unsigned k = 0; k<pattern_size; k++) {
          if(get(i,k)) {
            main_sum += plogp_patterns_frequencies[k];
          }
        }
        entropy = log_sum - main_sum / sum;
      }

      if(entropy > 0 && entropy + noise < min) {
        min = entropy + noise;
        argmin = i;
      }
    }
    return argmin;
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
