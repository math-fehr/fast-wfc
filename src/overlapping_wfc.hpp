#pragma once

#include "wfc.hpp"
#include <iostream>

template<typename T>
class OverlappingWFC {
public:

  static Matrix<T> get_output(const WFC& wfc, const vector<Matrix<T>>& patterns, const unsigned& pattern_size, const unsigned& output_width, const unsigned& output_height) {
    Matrix<T> output = Matrix<T>(output_width, output_height);

    if(wfc.periodic_output) {
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          output.get(y,x) = patterns[wfc.output_patterns.get(y,x)].get(0,0);
        }
      }
    } else {
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          for(unsigned dy = 0; dy < pattern_size; dy++) {
            for(unsigned dx = 0; dx < pattern_size; dx++) {
              output.get(y + dy, x + dx) = patterns[wfc.output_patterns.get(y,x)].get(dy,dx);
            }
          }
        }
      }
    }

    return output;
  }

  static Wave generate_initial_wave(const Matrix<T>& input, const unsigned& pattern_size, const vector<Matrix<T>>& patterns, const bool& ground, const bool& periodic_output, const unsigned& out_width, const unsigned& out_height, const vector<unsigned>& patterns_frequencies) {
    unsigned wave_width = periodic_output ? out_width : out_width - pattern_size + 1;
    unsigned wave_height = periodic_output ? out_height : out_height - pattern_size + 1;
    Wave wave(wave_width, wave_height, patterns_frequencies);

    if(ground) {
      init_ground(wave, input, patterns, pattern_size);
    }

    return wave;
  }

  static void init_ground(Wave& wave, const Matrix<T>& input, const vector<Matrix<T>>& patterns, const unsigned& pattern_size) {
    unsigned ground_pattern_id = get_ground_pattern_id(input, pattern_size, patterns);

    for(unsigned j = 0; j < wave.width; j++) {
      for(unsigned p = 0; p < patterns.size(); p++) {
        wave.set(wave.height - 1, j, p, ground_pattern_id == p);
      }
    }

    for(unsigned i = 0; i < wave.height - 1; i++) {
      for(unsigned j = 0; j < wave.width; j++) {
        wave.set(i, j, ground_pattern_id, false);
      }
    }
  }

  static unsigned get_ground_pattern_id(const Matrix<T>& input, const unsigned& pattern_size, const vector<Matrix<T>>& patterns) {
    Matrix<T> ground_pattern = get_ground_pattern(input, pattern_size);
    for(unsigned i = 0; i < patterns.size(); i++) {
      if(ground_pattern == patterns[i]) {
        return i;
      }
    }
    assert(false);
    return 0;
  }

  static Matrix<T> get_ground_pattern(const Matrix<T>& input, const unsigned& pattern_size) {
    return input.get_sub_matrix(input.height - 1, input.width / 2, pattern_size, pattern_size);
  }

  static pair<vector<unsigned>, vector<Matrix<T>>> get_patterns(const Matrix<T>& input, const unsigned& pattern_size, const bool& periodic_input, const unsigned& symmetry) {
    unordered_map<Matrix<T>, unsigned> matrix_frequencies;
    pair<vector<unsigned>, vector<Matrix<T>>> patterns;
    Matrix<T> sub_matrix = Matrix<T>(pattern_size, pattern_size);
    vector<Matrix<T>> sym(8, Matrix<T>(pattern_size, pattern_size));
    unsigned max_i = periodic_input ? input.height : input.height - pattern_size + 1;
    unsigned max_j = periodic_input ? input.width : input.width - pattern_size + 1;

    for(unsigned i = 0; i < max_i; i++) {
      for(unsigned j = 0; j < max_j; j++) {
        sym[0].data = input.get_sub_matrix(i, j, pattern_size, pattern_size).data;
        sym[1].data = sym[0].reflected().data;
        sym[2].data = sym[0].rotated().data;
        sym[3].data = sym[2].reflected().data;
        sym[4].data = sym[2].rotated().data;
        sym[5].data = sym[4].reflected().data;
        sym[6].data = sym[4].rotated().data;
        sym[7].data = sym[6].reflected().data;

        for(unsigned k = 0; k<symmetry; k++) {
          matrix_frequencies[sym[k]] += 1;
          if(matrix_frequencies[sym[k]] == 1) {
            patterns.second.push_back(sym[k]);
          }
        }
      }
    }

    for(const Matrix<T>& pattern : patterns.second) {
      patterns.first.push_back(matrix_frequencies[pattern]);
    }

    return patterns;
  }

  static bool agrees(const Matrix<T>& pattern1, const Matrix<T>& pattern2, int dx, int dy) {
    unsigned xmin = dx < 0 ? 0 : dx;
    unsigned xmax = dx < 0 ? dx + pattern2.width : pattern1.width;
    unsigned ymin = dy < 0 ? 0 : dy;
    unsigned ymax = dy < 0 ? dy + pattern2.height : pattern1.width;
    for(unsigned y = ymin; y < ymax; y++) {
      for(unsigned x = xmin; x < xmax; x++) {
        if(pattern1.get(y,x) != pattern2.get(y-dy,x-dx)) {
          return false;
        }
      }
    }
    return true;
  }

  static vector<array<vector<unsigned>, 4>> generate_propagator(const vector<Matrix<T>>& patterns) {
    unsigned patterns_size = patterns.size();
    vector<array<vector<unsigned>, 4>> propagator = vector<array<vector<unsigned>, 4>>(patterns_size);
    for(unsigned k1 = 0; k1 < patterns_size; k1++) {
      for(unsigned direction = 0; direction < 4; direction++) {
        for(unsigned k2 = 0; k2 < patterns_size; k2++) {
          if(agrees(patterns[k1], patterns[k2], directions_x[direction], directions_y[direction])) {
            propagator[k1][direction].push_back(k2);
          }
        }
      }
    }

    return propagator;
  }
};
