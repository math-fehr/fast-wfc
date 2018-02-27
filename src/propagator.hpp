#pragma once

#include <vector>
#include "matrix.hpp"
#include "matrix3D.hpp"
#include "wave.hpp"

using namespace std;

class Propagator {
private:
  Matrix3D<vector<unsigned>> propagator;
  Matrix<bool> propagating;
  unsigned wave_width;
  unsigned wave_height;
  unsigned n_width;
  unsigned n_height;
  unsigned patterns_size = 0;
  bool periodic_output;
public:
  Propagator() {};

  Propagator(unsigned wave_width, unsigned wave_height, unsigned n_width, unsigned n_height, bool periodic_output) :
    propagating(wave_width, wave_height),
    wave_width(wave_width), wave_height(wave_height),
    n_width(n_width), n_height(n_height), periodic_output(periodic_output) {
  }

  void add_to_propagator(unsigned index) {
    propagating.data[index] = true;
  }

  template<typename T>
  void init(const vector<Matrix<T>>& patterns) {
    patterns_size = patterns.size();
    propagator = Matrix3D<vector<unsigned>>(2 * n_width - 1, 2 * n_height - 1, patterns.size());
    for(unsigned x = 0; x < 2 * n_width - 1; x++) {
      for(unsigned y = 0; y < 2 * n_height - 1; y++) {
        for(unsigned k1 = 0; k1 < patterns.size(); k1++) {
          for(unsigned k2 = 0; k2 < patterns.size(); k2++) {
            if(agrees(patterns[k1], patterns[k2], x - n_width + 1, y - n_height + 1)) {
              propagator.get(x,y,k1).push_back(k2);
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
    bool should_propagate = true;
    while(should_propagate) {
      should_propagate = false;
      for(unsigned x1 = 0; x1 < wave.get_width(); x1++) {
        for(unsigned y1 = 0; y1 < wave.get_height(); y1++) {
          if(!propagating.data[y1 * wave.get_width() + x1]) {
            continue;
          }
          propagating.set(y1, x1, false);

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
              for(unsigned k2 = 0; k2 < patterns_size; k2++) {
                if(wave.get(i2, k2)) {
                  bool b = false;
                  for(const unsigned& pattern : propagator.get(n_width - 1 - dx, n_height - 1 - dy, k2)) {
                    b = wave.get(y1, x1, pattern);
                    if(b) {
                      break;
                    }
                  }
                  if(!b) {
                    should_propagate = true;
                    add_to_propagator(i2);
                    wave.set(i2, k2, false);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
};
