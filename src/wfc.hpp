#pragma once

#include <unordered_map>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

#include "matrix.hpp"
#include "wave.hpp"
#include "propagator.hpp"

using namespace std;

class WFC {
public:
  minstd_rand gen;

  Wave wave;
  Matrix<unsigned> output_patterns;
  const vector<unsigned> patterns_frequencies;
  const unsigned nb_patterns;
  Propagator propagator;

  const bool periodic_output;

  WFC(bool periodic_output, int seed, vector<unsigned> patterns_frequencies,
      vector<array<vector<unsigned>, 4>> propagator, unsigned wave_width, unsigned wave_height)
    : gen(seed), wave(wave_width, wave_height, patterns_frequencies),
      output_patterns(wave.width, wave.height),
      patterns_frequencies(patterns_frequencies), nb_patterns(propagator.size()),
      propagator(wave.width, wave.height, periodic_output, propagator),
      periodic_output(periodic_output)
  {
  }

  bool run() {
    while(true) {
      ObserveStatus result = observe();
      if(result == failure) {
        cout << "failed" << endl;
        return false;
      } else if(result == success) {
        return true;
      }
      propagator.propagate(wave);
    }
  }

  void wave_to_output() {
    for(unsigned i = 0; i< wave.size; i++) {
      for(unsigned k = 0; k < nb_patterns; k++) {
        if(wave.get(i, k)) {
          output_patterns.data[i] = k;
        }
      }
    }
  }

  enum ObserveStatus {
    success,
    failure,
    to_continue
  };

  ObserveStatus observe() {
    int argmin = wave.get_min_entropy(gen);
    if(argmin == -2) {
      return failure;
    }

    if(argmin == -1) {
      wave_to_output();
      return success;
    }

    double s = 0;

    std::uniform_real_distribution<> dis(0,1);
    double random_value = dis(gen);
    unsigned chosen_value = nb_patterns - 1;

    for(unsigned k = 0; k < nb_patterns; k++) {
      s+= wave.get(argmin,k) ? patterns_frequencies[k] : 0;
    }
    random_value *= s;

    for(unsigned k = 0; k < nb_patterns; k++) {
      random_value -= wave.get(argmin,k) ? patterns_frequencies[k] : 0;
      if(random_value <= 0) {
        chosen_value = k;
        break;
      }
    }

    for(unsigned k = 0; k < nb_patterns; k++) {
      if(wave.get(argmin, k) != (k == chosen_value)) {
        propagator.add_to_propagator(argmin / wave.width, argmin % wave.width, k);
        wave.set(argmin, k, false);
      }
    }

    return to_continue;
  }

  void propagate() {
    propagator.propagate(wave);
  }

  void remove_wave_pattern(unsigned i, unsigned j, unsigned pattern) {
    if(wave.get(i, j, pattern)) {
      wave.set(i, j, pattern, false);
      propagator.add_to_propagator(i, j, pattern);
    }
  }
};

