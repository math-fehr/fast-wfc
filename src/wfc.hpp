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
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

  Matrix<T> input;
  Matrix<T> output;
  Matrix<unsigned> output_patterns;
  vector<unsigned> patterns_frequencies;
  vector<double> plogp_patterns_frequencies;
  vector<Matrix<T>> patterns;
  Wave wave;
  Propagator propagator;

  unsigned symmetry;
  bool ground;
  bool periodic_input;
  bool periodic_output;
  unsigned n_width;
  unsigned n_height;

  WFC(const Matrix<T>& input, unsigned out_width, unsigned out_height, unsigned n_width, unsigned n_height,
      unsigned symmetry, bool periodic_input, bool periodic_output, int ground, int seed = 6683)
    : gen(seed), dis(0,1), input(input), output(out_width, out_height),
      symmetry(symmetry), ground(ground),
      periodic_input(periodic_input), periodic_output(periodic_output), n_width(n_width), n_height(n_height)
  {
    unsigned wave_width = periodic_output ? out_width : out_width - n_width + 1;
    unsigned wave_height = periodic_output ? out_height : out_height - n_height + 1;
    wave = Wave(wave_width, wave_height);
    propagator = Propagator(wave_width, wave_height, n_width, n_height, periodic_output);
    output_patterns = Matrix<unsigned>(wave_width, wave_height);
  }

  bool run() {
    init_patterns();
    propagator.init(patterns);
    wave.init(patterns.size());
    init_ground();
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

  unsigned get_id_of_matrix(const Matrix<T>& matrix) {
    for(unsigned i = 0; i<patterns.size(); i++) {
      if(matrix == patterns[i]) {
        return i;
      }
    }
    assert(false);
  }

  void add_pattern(const Matrix<T>& sub_matrix, unordered_map<Matrix<T>, unsigned>& matrix_frequencies) {
    vector<Matrix<T>> sym(8);
    sym[0] = sub_matrix;
    sym[1] = sym[0].reflected();
    sym[2] = sym[0].rotated();
    sym[3] = sym[2].reflected();
    sym[4] = sym[2].rotated();
    sym[5] = sym[4].reflected();
    sym[6] = sym[4].rotated();
    sym[7] = sym[6].reflected();

    for(unsigned k = 0; k<symmetry; k++) {
      matrix_frequencies[sym[k]] += 1;
      if(matrix_frequencies[sym[k]] == 1) {
        patterns.push_back(sym[k]);
      }
    }
  }

  void init_patterns() {
    unordered_map<Matrix<T>, unsigned> matrix_frequencies;
    Matrix<T> sub_matrix = Matrix<T>(n_width, n_height);
    unsigned max_i = periodic_input ? input.height : input.height - n_height + 1;
    unsigned max_j = periodic_input ? input.width : input.width - n_width + 1;
    for(unsigned i = 0; i < max_i; i++) {
      for(unsigned j = 0; j < max_j; j++) {
        add_pattern(input.get_sub_matrix(i,j,n_width,n_height), matrix_frequencies);
      }
    }

    for (const Matrix<T>& pattern : patterns) {
      patterns_frequencies.push_back(matrix_frequencies[pattern]);
      plogp_patterns_frequencies.push_back((double)matrix_frequencies[pattern] * log(matrix_frequencies[pattern]));
    }
  }

  void init_ground() {
    if(!ground) {
      return;
    }
    Matrix<T> ground_matrix = input.get_sub_matrix(input.height - 1, input.width / 2, n_width, n_height);
    unsigned ground_matrix_id = get_id_of_matrix(ground_matrix);

    for(unsigned j = 0; j < wave.get_width(); j++) {
      for(unsigned k = 0; k < patterns.size(); k++) {
        if(ground_matrix_id != k) {
          wave.set(wave.get_height() - 1, j, k, false);
          change(wave.get_height() - 1, j);
        }
      }
    }

    for(unsigned i = 0; i < wave.get_height() - 1; i++) {
      for(unsigned j = 0; j < wave.get_width(); j++) {
        wave.set(i, j, ground_matrix_id, false);
        change(i,j);
      }
    }

    propagator.propagate(wave);
  }

  void wave_to_output() {
    for(unsigned i = 0; i< wave.get_size(); i++) {
      for(unsigned k = 0; k < patterns.size(); k++) {
        if(wave.get(i, k)) {
          output_patterns.data[i] = k;
        }
      }
    }

    if(periodic_output) {
      for(unsigned y = 0; y < wave.get_height(); y++) {
        for(unsigned x = 0; x < wave.get_width(); x++) {
          output.get(y,x) = patterns[output_patterns.get(y,x)].get(0,0);
        }
      }
    } else {
      for(unsigned y = 0; y < wave.get_height(); y++) {
        for(unsigned x = 0; x < wave.get_width(); x++) {
          for(unsigned dy = 0; dy < n_height; dy++) {
            for(unsigned dx = 0; dx < n_width; dx++) {
              output.get(y + dy, x + dx) = patterns[output_patterns.get(y,x)].get(dy,dx);
            }
          }
        }
      }
    }
  }

  int get_min_entropy() {
    double min = std::numeric_limits<float>::infinity();
    int argmin = -1;
    for(unsigned i = 0; i < wave.get_size(); i++) {
      double sum = 0;
      int nb_possibilities = 0;
      for(unsigned k = 0; k < patterns.size(); k++) {
        if(wave.get(i,k)) {
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
        for(unsigned k = 0; k<patterns.size(); k++) {
          if(wave.get(i,k)) {
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

  enum ObserveStatus {
    success,
    failure,
    to_continue
  };

  ObserveStatus observe() {
    int argmin = get_min_entropy();
    if(argmin == -2) {
      return failure;
    }

    if(argmin == -1) {
      wave_to_output();
      return success;
    }

    double s = 0;
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
      wave.set(argmin, k, k == chosen_value);
    }

    change(argmin);

    return to_continue;
  }

  void change(unsigned i) {
    propagator.add_to_propagator(i);
  }

  void change(unsigned i, unsigned j) {
    change(j + wave.get_height() * i);
  }
};

