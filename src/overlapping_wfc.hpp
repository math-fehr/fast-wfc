#pragma once

#include "wfc.hpp"
#include <iostream>

struct OverlappingWFCOptions {
  bool periodic_input;
  bool periodic_output;
  unsigned out_height;
  unsigned out_width;
  unsigned symmetry;
  bool ground;
  unsigned pattern_size;
  int seed;
};

template<typename T>
class OverlappingWFC {
private:
  Array2D<T> input;
  OverlappingWFCOptions options;
  vector<Array2D<T>> patterns;
  WFC wfc;

  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, const int& seed,
                 const pair<vector<unsigned>, vector<Array2D<T>>>& patterns,
                 const vector<array<vector<unsigned>, 4>>& propagator) :
    input(input),
    options(options),
    patterns(patterns.second),
    wfc(options.periodic_output, seed, patterns.first, propagator,
        options.periodic_output ? options.out_height : options.out_height - options.pattern_size + 1,
        options.periodic_output ? options.out_width : options.out_width - options.pattern_size + 1)
  {
    if(options.ground) {
      init_ground(wfc, input, patterns.second, options);
    }
  }

  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, const int& seed,
                 const pair<vector<unsigned>, vector<Array2D<T>>>& patterns) :
    OverlappingWFC(input, options, seed, patterns, generate_propagator(patterns.second))
  {}

public:
  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, int seed) :
    OverlappingWFC(input, options, seed, get_patterns(input, options))
  {}

  bool run() {
    return wfc.run();
  }

  Array2D<T> get_output() {
    Array2D<T> output = Array2D<T>(options.out_height, options.out_width);

    if(wfc.periodic_output) {
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          output.get(y,x) = patterns[wfc.output_patterns.get(y,x)].get(0,0);
        }
      }
    } else {
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          for(unsigned dy = 0; dy < options.pattern_size; dy++) {
            for(unsigned dx = 0; dx < options.pattern_size; dx++) {
              output.get(y + dy, x + dx) = patterns[wfc.output_patterns.get(y,x)].get(dy,dx);
            }
          }
        }
      }
    }

    return output;
  }


  static void init_ground(WFC& wfc, const Array2D<T>& input, const vector<Array2D<T>>& patterns, const OverlappingWFCOptions& options) {
    unsigned ground_pattern_id = get_ground_pattern_id(input, patterns, options);

    for(unsigned j = 0; j < wfc.wave.width; j++) {
      for(unsigned p = 0; p < patterns.size(); p++) {
        if(ground_pattern_id != p) {
          wfc.remove_wave_pattern(wfc.wave.height - 1, j, p);
        }
      }
    }

    for(unsigned i = 0; i < wfc.wave.height - 1; i++) {
      for(unsigned j = 0; j < wfc.wave.width; j++) {
        wfc.remove_wave_pattern(i, j, ground_pattern_id);
      }
    }

    wfc.propagate();
  }

  static unsigned get_ground_pattern_id(const Array2D<T>& input, const vector<Array2D<T>>& patterns, const OverlappingWFCOptions& options) {
    Array2D<T> ground_pattern = get_ground_pattern(input, options);
    for(unsigned i = 0; i < patterns.size(); i++) {
      if(ground_pattern == patterns[i]) {
        return i;
      }
    }
    assert(false);
    return 0;
  }

  static Array2D<T> get_ground_pattern(const Array2D<T>& input, const OverlappingWFCOptions& options) {
    return input.get_sub_array(input.height - 1, input.width / 2, options.pattern_size, options.pattern_size);
  }

  static pair<vector<unsigned>, vector<Array2D<T>>> get_patterns(const Array2D<T>& input, const OverlappingWFCOptions& options) {
    unordered_map<Array2D<T>, unsigned> matrix_frequencies;
    pair<vector<unsigned>, vector<Array2D<T>>> patterns;
    Array2D<T> sub_matrix = Array2D<T>(options.pattern_size, options.pattern_size);
    vector<Array2D<T>> sym(8, Array2D<T>(options.pattern_size, options.pattern_size));
    unsigned max_i = options.periodic_input ? input.height : input.height - options.pattern_size + 1;
    unsigned max_j = options.periodic_input ? input.width : input.width - options.pattern_size + 1;

    for(unsigned i = 0; i < max_i; i++) {
      for(unsigned j = 0; j < max_j; j++) {
        sym[0].data = input.get_sub_array(i, j, options.pattern_size, options.pattern_size).data;
        sym[1].data = sym[0].reflected().data;
        sym[2].data = sym[0].rotated().data;
        sym[3].data = sym[2].reflected().data;
        sym[4].data = sym[2].rotated().data;
        sym[5].data = sym[4].reflected().data;
        sym[6].data = sym[4].rotated().data;
        sym[7].data = sym[6].reflected().data;

        for(unsigned k = 0; k<options.symmetry; k++) {
          matrix_frequencies[sym[k]] += 1;
          if(matrix_frequencies[sym[k]] == 1) {
            patterns.second.push_back(sym[k]);
          }
        }
      }
    }

    for(const Array2D<T>& pattern : patterns.second) {
      patterns.first.push_back(matrix_frequencies[pattern]);
    }

    return patterns;
  }

  static bool agrees(const Array2D<T>& pattern1, const Array2D<T>& pattern2, int dx, int dy) {
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

  static vector<array<vector<unsigned>, 4>> generate_propagator(const vector<Array2D<T>>& patterns) {
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
