#pragma once

#include <unordered_map>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

#include "matrix.hpp"

template<typename T>
class WFC {
public:
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

  Matrix<T> input;
  Matrix<T> output;
  Matrix<unsigned> output_patterns;
  unordered_map<Matrix<T>, unsigned> patterns_frequencies;
  vector<unsigned> patterns_frequencies_by_id;
  vector<double> log_patterns_frequencies_by_id;
  vector<Matrix<T>> patterns;
  Matrix<vector<bool>> wave;
  vector<unsigned> to_propagate;
  vector<bool> is_propagating;
  vector<vector<vector<vector<unsigned>>>> propagator;

  unsigned n_width;
  unsigned n_height;

  WFC(const Matrix<T>& input, unsigned out_width, unsigned out_height, unsigned n_width, unsigned n_height)
    : n_width(n_width), n_height(n_height),
      input(input), output(out_width, out_height),
      wave(out_width - n_width + 1, out_height - n_height + 1),
      output_patterns(out_width - n_width + 1, out_height - n_height + 1),
      is_propagating((out_width - n_width + 1) * (out_height - n_height + 1), false), dis(0,1)
  {
    gen = mt19937(42);
    init_patterns();
    init_wave();
    init_propagator();
  }

  bool run() {
    int l = 0;
    while(true) {
      ObserveStatus result = observe();
      cout << l++ << endl;
      if(result == failure) {
        cout << "failed" << endl;
        return false;
      } else if(result == success) {
        return true;
      }
      propagate();
    }
  }

  void init_patterns() {
    Matrix<T> sub_matrix = Matrix<T>(n_width, n_height);
    for(unsigned i = 0; i<input.height - n_height + 1; i++) {
      for(unsigned j = 0; j<input.width - n_width + 1; j++) {
        for(unsigned ki = 0; ki<n_height; ki++) {
          for(unsigned kj = 0; kj<n_width; kj++) {
            sub_matrix.data[ki * n_width + kj] = input.data[(i+ki) * input.width + j+kj];
          }
        }
        patterns_frequencies[sub_matrix] += 1;
        if(patterns_frequencies[sub_matrix] == 1) {
          patterns.push_back(sub_matrix);
        }
      }
    }

    for (const Matrix<T>& pattern : patterns) {
      patterns_frequencies_by_id.push_back(patterns_frequencies[pattern]);
      log_patterns_frequencies_by_id.push_back(log(patterns_frequencies[pattern]));
    }
  }

  void init_wave() {
    unsigned nb_patterns = patterns.size();
    for(unsigned i = 0; i<wave.data.size(); i++) {
      wave[i] = vector<bool>(nb_patterns, true);
    }
  }

  void init_propagator() {
    propagator = vector<vector<vector<vector<unsigned>>>>(2 * n_width - 1);
    for(unsigned x = 0; x < 2 * n_width - 1; x++) {
      propagator[x] = vector<vector<vector<unsigned>>>(2 * n_height - 1);
      for(unsigned y = 0; y < 2 * n_height - 1; y++) {
        propagator[x][y] = vector<vector<unsigned>>(patterns.size());
        for(unsigned k1 = 0; k1 < patterns.size(); k1++) {
          propagator[x][y][k1] = vector<unsigned>();
          for(unsigned k2 = 0; k2 < patterns.size(); k2++) {
            if(agrees(patterns[k1], patterns[k2], x - n_width + 1, y - n_height + 1)) {
              propagator[x][y][k1].push_back(k2);
            }
          }
        }
      }
    }
  }

  bool agrees(Matrix<T> pattern1, Matrix<T> pattern2, int dx, int dy) {
    unsigned xmin = dx < 0 ? 0 : dx;
    unsigned xmax = dx < 0 ? dx + n_width : n_width;
    unsigned ymin = dy < 0 ? 0 : dy;
    unsigned ymax = dy < 0 ? dy + n_height : n_height;
    for(unsigned y = ymin; y < ymax; y++) {
      for(unsigned x = xmin; x < xmax; x++) {
        if(pattern1[x + n_width * y] != pattern2[x - dx + n_width * (y - dy)]) {
          return false;
        }
      }
    }
    return true;
  }

  enum ObserveStatus {
    success,
    failure,
    to_continue
  };

  ObserveStatus observe() {
    double min = std::numeric_limits<float>::infinity();
    int argmin = -1;

    for(unsigned i = 0; i < wave.data.size(); i++) {
      double sum = 0;
      for(unsigned k = 0; k < patterns.size(); k++) {
        if(wave[i][k]) {
          sum += patterns_frequencies_by_id[k];
        }
      }

      if(sum == 0) {
        return failure;
      }

      double noise = dis(gen) * 1e-5;
      double entropy = 0;
      double main_sum = 0;
      double log_sum = log(sum);
      for(unsigned k = 0; k<patterns.size(); k++) {
        if(wave[i][k]) {
          main_sum += patterns_frequencies_by_id[k] * log_patterns_frequencies_by_id[k];
        }
      }
      entropy = log_sum - main_sum / sum;

      if(entropy > 0 && entropy + noise < min) {
        min = entropy + noise;
        argmin = i;
      }
    }

    if(argmin == -1) {
      for(unsigned i = 0; i< wave.data.size(); i++) {
        for(unsigned k = 0; k < patterns.size(); k++) {
          if(wave[i][k]) {
            output_patterns[i] = k;
          }
        }
      }

      for(unsigned y = 0; y < wave.height; y++) {
        for(unsigned x = 0; x < wave.width; x++) {
          for(unsigned dy = 0; dy < n_height; dy++) {
            for(unsigned dx = 0; dx < n_width; dx++) {
              output[y + dy + output.width * (x + dx)] = patterns[output_patterns[y + x * output_patterns.width]][dy + dx * n_width];
            }
          }
        }
      }
      return success;
    }

    vector<double> distribution = vector<double>();
    double s = 0;
    double random_value = dis(gen);
    unsigned chosen_value;

    for(unsigned k = 0; k < patterns.size(); k++) {
      s+= wave[argmin][k] ? patterns_frequencies_by_id[k] : 0;
    }
    random_value *= s;

    for(unsigned k = 0; k < patterns.size(); k++) {
      random_value -= wave[argmin][k] ? patterns_frequencies_by_id[k] : 0;
      if(random_value <= 0 || k == patterns.size() - 1) {
        chosen_value = k;
        break;
      }
    }

    for(unsigned k = 0; k < patterns.size(); k++) {
      wave[argmin][k] = k == chosen_value;
    }

    change(argmin);

    return to_continue;
  }

  void propagate() {
    while(to_propagate.size() != 0) {
      unsigned i1 = to_propagate.back();
      to_propagate.pop_back();
      is_propagating[i1] = false;

      unsigned x1 = i1 % wave.width;
      unsigned y1 = i1 / wave.width;

      for(int dx = -int(n_width) + 1; dx < int(n_width); dx++) {
        for(int dy = -int(n_height) + 1; dy < int(n_height); dy++) {
          int x2 = x1 + dx;
          int y2 = y1 + dy;
          if(x2 < 0 || x2 >= wave.width) {
            continue;
          }
          if(y2 < 0 || y2 >= wave.height) {
            continue;
          }

          unsigned i2 = x2 + y2 * wave.width;
          const vector<vector<unsigned>>& prop = propagator[n_width - 1 - dx][n_height - 1 - dy];
          for(unsigned k2 = 0; k2 < patterns.size(); k2++) {
            if(wave[i2][k2]) {
              bool b = false;
              for(unsigned l = 0; l < prop[k2].size(); l++) {
                b = wave[i1][prop[k2][l]];
                if(b) {
                  break;
                }
              }
              if(!b) {
                change(i2);
                wave[i2][k2] = false;
              }
            }
          }
        }
      }
    }
  }

  void change(unsigned i) {
    if(is_propagating[i]) {
      return;
    }
    to_propagate.push_back(i);
    is_propagating[i] = true;
  }
};

