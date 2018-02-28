#pragma once

#include <vector>
#include <stdint.h>
#include <limits>
#include <math.h>
#include <random>
#include <iostream>

using namespace std;

class Wave {
private:
  vector<uint8_t> data;
  vector<unsigned> patterns_frequencies;
  vector<double> plogp_patterns_frequencies;
  vector<double> plogp_memoisation;
  vector<unsigned> s_memoisation;
  vector<double> logs_memoisation;
  vector<unsigned> nb_possibilities_memoisation;
  vector<double> entropy_memoisation;
  bool is_impossible;
  double half_min_plogp;
  unsigned width;
  unsigned height;
  unsigned pattern_size;
public:
  Wave() {}


  Wave(unsigned width, unsigned height, const vector<unsigned>& patterns_frequencies) :
    patterns_frequencies(patterns_frequencies), width(width), height(height),
    pattern_size(patterns_frequencies.size())
  {
    is_impossible = false;
    double base_entropy = 0;
    unsigned base_s = 0;
    double half_min_plogp = numeric_limits<double>::infinity();
    for(unsigned i = 0; i < pattern_size; i++) {
      plogp_patterns_frequencies.push_back((double)patterns_frequencies[i] * log(patterns_frequencies[i]));
      half_min_plogp = min(half_min_plogp, plogp_patterns_frequencies[i] / 2.0);
      base_entropy += plogp_patterns_frequencies[i];
      base_s += patterns_frequencies[i];
    }
    double log_base_s = log(base_s);
    for(unsigned i = 0; i < width * height; i++) {
      plogp_memoisation.push_back(base_entropy);
      s_memoisation.push_back(base_s);
      logs_memoisation.push_back(log_base_s);
      nb_possibilities_memoisation.push_back(pattern_size);
      entropy_memoisation.push_back(logs_memoisation[i] - plogp_memoisation[i] / s_memoisation[i]);
    }
    data = vector<uint8_t>(width * height * pattern_size, 1);
  }

  bool get(unsigned index, unsigned pattern) {
    return data[index * pattern_size + pattern];
  }

  void set(unsigned index, unsigned pattern, bool value) {
    bool old_value = data[index * pattern_size + pattern];
    if(old_value == value) {
      return;
    }
    data[index * pattern_size + pattern] = value;
    plogp_memoisation[index] -= plogp_patterns_frequencies[pattern];
    s_memoisation[index] -= patterns_frequencies[pattern];
    logs_memoisation[index] = log(s_memoisation[index]);
    nb_possibilities_memoisation[index]--;
    entropy_memoisation[index] = logs_memoisation[index] - plogp_memoisation[index] / s_memoisation[index];
    if(nb_possibilities_memoisation[index] == 0) {
      is_impossible = true;
    }
  }

  bool get(unsigned i, unsigned j, unsigned pattern) {
    return get(i * width + j, pattern);
  }

  void set(unsigned i, unsigned j, unsigned pattern, bool value) {
    set(i * width + j, pattern, value);
  }

  int get_min_entropy(minstd_rand& gen) {
    if(is_impossible) {
      return -2;
    }
    std::uniform_real_distribution<> dis(0,1);
    double min = numeric_limits<double>::infinity();
    int argmin = -1;
    for(unsigned i = 0; i < get_size(); i++) {
      double nb_possibilities = nb_possibilities_memoisation[i];
      if(nb_possibilities == 1) {
        continue;
      }

      double entropy = entropy_memoisation[i];

      if(entropy <= min + half_min_plogp) {
        double noise = dis(gen) * half_min_plogp;
        if(entropy + noise < min) {
          min = entropy + noise;
          argmin = i;
        }
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
