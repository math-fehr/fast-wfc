#pragma once

#include <vector>
#include <stdint.h>
#include <limits>
#include <math.h>
#include <random>
#include <iostream>

using namespace std;

struct EntropyMemoisation {
  vector<double> plogp;
  vector<double> sum;
  vector<double> log_sum;
  vector<unsigned> nb_possibilities;
  vector<double> entropy;
};

class Wave {
private:
  vector<uint8_t> data;
  const vector<double> patterns_frequencies;
  vector<double> plogp_patterns_frequencies;
  EntropyMemoisation memoisation;
  bool is_impossible;
  double half_min_plogp;
  unsigned nb_patterns;

public:
  const unsigned width;
  const unsigned height;
  const unsigned size;

  Wave(unsigned height, unsigned width, const vector<double>& patterns_frequencies) :
    patterns_frequencies(patterns_frequencies), nb_patterns(patterns_frequencies.size()),
    width(width), height(height), size(width * height)
  {
    is_impossible = false;
    double base_entropy = 0;
    double base_s = 0;
    double half_min_plogp = numeric_limits<double>::infinity();
    for(unsigned i = 0; i < nb_patterns; i++) {
      plogp_patterns_frequencies.push_back(patterns_frequencies[i] * log(patterns_frequencies[i]));
      half_min_plogp = min(half_min_plogp, plogp_patterns_frequencies[i] / 2.0);
      base_entropy += plogp_patterns_frequencies[i];
      base_s += patterns_frequencies[i];
    }
    double log_base_s = log(base_s);
    double entropy_base = log_base_s - base_entropy / base_s;
    memoisation.plogp = vector<double>(width * height, base_entropy);
    memoisation.sum = vector<double>(width * height, base_s);
    memoisation.log_sum = vector<double>(width * height, log_base_s);
    memoisation.nb_possibilities = vector<unsigned>(width * height, nb_patterns);
    memoisation.entropy = vector<double>(width * height, entropy_base);
    data = vector<uint8_t>(width * height * nb_patterns, 1);
  }

  bool get(unsigned index, unsigned pattern) {
    return data[index * nb_patterns + pattern];
  }

  void set(unsigned index, unsigned pattern, bool value) {
    bool old_value = data[index * nb_patterns + pattern];
    if(old_value == value) {
      return;
    }
    data[index * nb_patterns + pattern] = value;
    memoisation.plogp[index] -= plogp_patterns_frequencies[pattern];
    memoisation.sum[index] -= patterns_frequencies[pattern];
    memoisation.log_sum[index] = log(memoisation.sum[index]);
    memoisation.nb_possibilities[index]--;
    memoisation.entropy[index] = memoisation.log_sum[index] - memoisation.plogp[index] / memoisation.sum[index];
    if(memoisation.nb_possibilities[index] == 0) {
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
    for(unsigned i = 0; i < size; i++) {
      double nb_possibilities = memoisation.nb_possibilities[i];
      if(nb_possibilities == 1) {
        continue;
      }

      double entropy = memoisation.entropy[i];
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

};
