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

template<typename T>
class WFC {
public:
  minstd_rand gen;

  Matrix<T> input;
  Matrix<T> output;
  Wave wave;
  Matrix<unsigned> output_patterns;
  const vector<unsigned> patterns_frequencies;
  const vector<Matrix<T>> patterns;
  Propagator propagator;

  unsigned symmetry;
  bool ground;
  bool periodic_input;
  bool periodic_output;
  unsigned n_width;
  unsigned n_height;

  WFC(const Matrix<T>& input, unsigned out_width, unsigned out_height, unsigned n_width, unsigned n_height,
      unsigned symmetry, bool periodic_input, bool periodic_output, int ground, int seed,
      vector<unsigned> patterns_frequencies, vector<Matrix<T>> patterns,
      vector<array<vector<unsigned>, 4>> propagator, Wave wave)
    : gen(seed) , input(input), output(out_width, out_height),
      wave(wave),
      output_patterns(wave.width, wave.height),
      patterns_frequencies(patterns_frequencies), patterns(patterns),
      propagator(wave.width, wave.height, periodic_output, propagator),
      symmetry(symmetry), ground(ground),
      periodic_input(periodic_input), periodic_output(periodic_output), n_width(n_width), n_height(n_height)
  {
    for(unsigned i = 0; i < wave.height; i++) {
      for(unsigned j = 0; j < wave.width; j++) {
        for(unsigned p = 0; p < patterns.size(); p++) {
          if(!wave.get(i,j,p)) {
            change(i, j, p);
          }
        }
      }
    }
    this->propagator.propagate(this->wave);
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
      for(unsigned k = 0; k < patterns.size(); k++) {
        if(wave.get(i, k)) {
          output_patterns.data[i] = k;
        }
      }
    }

    if(periodic_output) {
      for(unsigned y = 0; y < wave.height; y++) {
        for(unsigned x = 0; x < wave.width; x++) {
          output.get(y,x) = patterns[output_patterns.get(y,x)].get(0,0);
        }
      }
    } else {
      for(unsigned y = 0; y < wave.height; y++) {
        for(unsigned x = 0; x < wave.width; x++) {
          for(unsigned dy = 0; dy < n_height; dy++) {
            for(unsigned dx = 0; dx < n_width; dx++) {
              output.get(y + dy, x + dx) = patterns[output_patterns.get(y,x)].get(dy,dx);
            }
          }
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
    unsigned chosen_value = patterns.size() - 1;

    for(unsigned k = 0; k < patterns.size(); k++) {
      s+= wave.get(argmin,k) ? patterns_frequencies[k] : 0;
    }
    random_value *= s;

    for(unsigned k = 0; k < patterns.size(); k++) {
      random_value -= wave.get(argmin,k) ? patterns_frequencies[k] : 0;
      if(random_value <= 0) {
        chosen_value = k;
        break;
      }
    }

    for(unsigned k = 0; k < patterns.size(); k++) {
      if(wave.get(argmin, k) != (k == chosen_value)) {
        change(argmin / wave.width, argmin % wave.width, k);
        wave.set(argmin, k, false);
      }
    }

    return to_continue;
  }

  void change(unsigned i, unsigned j, unsigned pattern) {
    propagator.add_to_propagator(i, j, pattern);
  }
};

