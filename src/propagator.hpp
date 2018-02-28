#pragma once

#include <vector>
#include <tuple>
#include "matrix.hpp"
#include "matrix3D.hpp"
#include "wave.hpp"
#include "direction.hpp"

using namespace std;

struct S {
  int data[4] = {};
};

class Propagator {
private:
  vector<array<vector<unsigned>, 4>> propagator;
  Matrix3D<S> compatible;
  vector<tuple<unsigned, unsigned, unsigned>> propagating;
  unsigned wave_width;
  unsigned wave_height;
  unsigned n_width;
  unsigned n_height;
  unsigned patterns_size = 0;
  bool periodic_output;
public:
  Propagator() {};

  Propagator(unsigned wave_width, unsigned wave_height, unsigned n_width, unsigned n_height, bool periodic_output) :
    wave_width(wave_width), wave_height(wave_height),
    n_width(n_width), n_height(n_height), periodic_output(periodic_output) {
  }

  void add_to_propagator(unsigned y, unsigned x, unsigned pattern) {
    S temp;
    compatible.set(y,x,pattern,temp);
    propagating.emplace_back(y, x, pattern);
  }

  template<typename T>
  void init(const vector<Matrix<T>>& patterns) {
    patterns_size = patterns.size();
    compatible = Matrix3D<S>(wave_width, wave_height, patterns_size);
    propagator = vector<array<vector<unsigned>, 4>>(patterns_size);
    for(unsigned k1 = 0; k1 < patterns_size; k1++) {
      for(unsigned direction = 0; direction < 4; direction++) {
        for(unsigned k2 = 0; k2 < patterns_size; k2++) {
          if(agrees(patterns[k1], patterns[k2], directions_x[direction], directions_y[direction])) {
            propagator[k1][direction].push_back(k2);
          }
        }
      }
    }

    for(unsigned y = 0; y < wave_height; y++) {
      for(unsigned x = 0; x < wave_width; x++) {
        for(unsigned pattern = 0; pattern < patterns_size; pattern++) {
          S value;
          for(int direction = 0; direction < 4; direction++) {
            value.data[direction] = propagator[pattern][inv_direction[direction]].size();
          }
          compatible.set(y, x, pattern, value);
        }
      }
    }
  }

  template<typename T>
  bool agrees(Matrix<T> pattern1, Matrix<T> pattern2, int dx, int dy) {
    unsigned xmin = dx < 0 ? 0 : dx;
    unsigned xmax = dx < 0 ? dx + n_width : n_width;
    unsigned ymin = dy < 0 ? 0 : dy;
    unsigned ymax = dy < 0 ? dy + n_height : n_height;
    for(unsigned y = ymin; y < ymax; y++) {
      for(unsigned x = xmin; x < xmax; x++) {
        if(pattern1.get(y,x) != pattern2.get(y-dy,x-dx)) {
          return false;
        }
      }
    }
    return true;
  }


  void propagate(Wave& wave) {
    while(propagating.size() != 0) {
      unsigned y1, x1, pattern;
      tie(y1, x1, pattern) = propagating.back();
      propagating.pop_back();

      for(unsigned direction = 0; direction < 4; direction++) {
        int dx = directions_x[direction];
        int dy = directions_y[direction];
        int x2, y2;
        if(periodic_output) {
          x2 = ((int)x1 + dx + (int)wave.get_width()) % wave.get_width();
          y2 = ((int)y1 + dy + (int)wave.get_height()) % wave.get_height();
        } else {
          x2 = x1 + dx;
          y2 = y1 + dy;
          if(x2 < 0 || x2 >= (int)wave.get_width()) {
            continue;
          }
          if(y2 < 0 || y2 >= (int)wave.get_height()) {
            continue;
          }
        }

        unsigned i2 = x2 + y2 * wave.get_width();
        const vector<unsigned>& patterns = propagator[pattern][direction];
        for(auto it = patterns.begin(), it_end = patterns.end(); it < it_end; ++it) {
          S& value = compatible.get(y2, x2, *it);
          value.data[direction]--;
          if(value.data[direction] == 0) {
            add_to_propagator(y2, x2, *it);
            wave.set(i2, *it, false);
          }
        }
      }
    }
  }


  /*
  template<unsigned direction, bool periodic_output>
  void propagate_aux2(Wave& wave, unsigned x1, unsigned y1, unsigned pattern) {
    constexpr int dx = directions_x[direction];
    constexpr int dy = directions_y[direction];
    int x2, y2;

    if constexpr(periodic_output) {
      if constexpr(dx == 1) {
        x2 = x1 + 1 == wave.get_width() ? 0 : x1 + 1;
      } else if constexpr(dx == -1) {
        x2 = x1 ? x1 - 1 : wave.get_width() - 1;
      } else {
        x2 = x1;
      }

      if constexpr(dy == 1) {
        y2 = y1 + 1 == wave.get_height() ? 0 : y1 + 1;
      } else if constexpr(dy == -1) {
        y2 = y1 ? y1 - 1 : wave.get_height() - 1;
      } else {
        y2 = y1;
      }
    } else {
      if constexpr(dx == 1) {
        if(x1 + 1 == wave.get_width()) {
          return;
        }
        x2 = x1 + 1;
      } else if constexpr(dx == -1) {
        if(x1 == 0) {
          return;
        }
        x2 = x1 - 1;
      } else {
        x2 = x1;
      }

      if constexpr(dy == 1) {
        if(y1 + 1 == wave.get_height()) {
          return;
        }
        y2 = y1 + 1;
      } else if constexpr(dy == -1) {
        if(y1 == 0) {
          return;
        }
        y2 = y1 - 1;
      } else {
        y2 = y1;
      }
    }

    propagate_aux(wave, x2, y2, direction, pattern);
  }

  void propagate(Wave& wave) {
    while(propagating.size() != 0) {
      unsigned y1, x1, pattern;
      tie(y1, x1, pattern) = propagating.back();
      propagating.pop_back();

      if(periodic_output) {
        propagate_aux2<0,true>(wave, x1, y1, pattern);
        propagate_aux2<1,true>(wave, x1, y1, pattern);
        propagate_aux2<2,true>(wave, x1, y1, pattern);
        propagate_aux2<3,true>(wave, x1, y1, pattern);
      } else {
        propagate_aux2<0,false>(wave, x1, y1, pattern);
        propagate_aux2<1,false>(wave, x1, y1, pattern);
        propagate_aux2<2,false>(wave, x1, y1, pattern);
        propagate_aux2<3,false>(wave, x1, y1, pattern);
      }
    }
  }

  void propagate_aux(Wave& wave, unsigned x2, unsigned y2, unsigned direction, unsigned pattern) {
    unsigned i2 = x2 + y2 * wave.get_width();
    const vector<unsigned>& patterns = propagator[pattern][direction];
    for(auto it = patterns.begin(), it_end = patterns.end(); it < it_end; ++it) {
      S& value = compatible.get(y2, x2, *it);
      value.data[direction]--;
      if(value.data[direction] == 0) {
        add_to_propagator(y2, x2, *it);
        wave.set(i2, *it, false);
      }
    }
  }
  */
};
