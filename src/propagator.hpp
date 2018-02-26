#pragma once

#include <vector>
#include "matrix.hpp"
#include "wave.hpp"

using namespace std;

class Propagator {
private:
  vector<vector<vector<vector<unsigned>>>> propagator;
  vector<unsigned> to_propagate;
  vector<bool> is_propagating;
  unsigned wave_width;
  unsigned wave_height;
  unsigned n_width;
  unsigned n_height;
  unsigned patterns_size = 0;
  bool periodic_output;
public:
  Propagator() {};

  Propagator(unsigned wave_width, unsigned wave_height, unsigned n_width, unsigned n_height, bool periodic_output) :
    is_propagating(wave_width * wave_height, false),
    wave_width(wave_width), wave_height(wave_height),
    n_width(n_width), n_height(n_height), periodic_output(periodic_output) {
  }

  void add_to_propagator(unsigned pattern) {
    if(is_propagating[pattern]) {
      return;
    }
    to_propagate.push_back(pattern);
    is_propagating[pattern] = true;
  }

  template<typename T>
  void init(const vector<Matrix<T>>& patterns) {
    patterns_size = patterns.size();
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
    while(to_propagate.size() != 0) {
      unsigned i1 = to_propagate.back();
      to_propagate.pop_back();
      is_propagating[i1] = false;

      unsigned x1 = i1 % wave.get_width();
      unsigned y1 = i1 / wave.get_width();

      for(int dx = -int(n_width) + 1; dx < int(n_width); dx++) {
        for(int dy = -int(n_height) + 1; dy < int(n_height); dy++) {
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
          const vector<vector<unsigned>>& prop = propagator[n_width - 1 - dx][n_height - 1 - dy];
          for(unsigned k2 = 0; k2 < patterns_size; k2++) {
            if(wave.get(i2, k2)) {
              bool b = false;
              for(unsigned pattern : prop[k2]) {
                b = wave.get(i1, pattern);
                if(b) {
                  break;
                }
              }
              if(!b) {
                add_to_propagator(i2);
                wave.set(i2, k2, false);
              }
            }
          }
        }
      }
    }
  }


};
