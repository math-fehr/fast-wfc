#pragma once

#include <vector>
#include <tuple>
#include "array2D.hpp"
#include "array3D.hpp"
#include "wave.hpp"
#include "direction.hpp"

using namespace std;

class Propagator {
private:
  const unsigned patterns_size;
  const vector<array<vector<unsigned>, 4>> propagator;
  const unsigned wave_width;
  const unsigned wave_height;
  const bool periodic_output;
  vector<tuple<unsigned, unsigned, unsigned>> propagating;
  Array3D<array<int, 4>> compatible;
public:

  Propagator(unsigned wave_height, unsigned wave_width, bool periodic_output,
             vector<array<vector<unsigned>, 4>> propagator) :
    patterns_size(propagator.size()), propagator(propagator), wave_width(wave_width),
    wave_height(wave_height), periodic_output(periodic_output),
    compatible(wave_height, wave_width, patterns_size)
  {
    for(unsigned y = 0; y < wave_height; y++) {
      for(unsigned x = 0; x < wave_width; x++) {
        for(unsigned pattern = 0; pattern < patterns_size; pattern++) {
          array<int, 4> value;
          for(int direction = 0; direction < 4; direction++) {
            value[direction] = propagator[pattern][get_opposite_direction(direction)].size();
          }
          compatible.get(y, x, pattern) = value;
        }
      }
    }
  }

  void add_to_propagator(unsigned y, unsigned x, unsigned pattern) {
    array<int, 4> temp = {};
    compatible.get(y,x,pattern) = temp;
    propagating.emplace_back(y, x, pattern);
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
          x2 = ((int)x1 + dx + (int)wave.width) % wave.width;
          y2 = ((int)y1 + dy + (int)wave.height) % wave.height;
        } else {
          x2 = x1 + dx;
          y2 = y1 + dy;
          if(x2 < 0 || x2 >= (int)wave.width) {
            continue;
          }
          if(y2 < 0 || y2 >= (int)wave.height) {
            continue;
          }
        }

        unsigned i2 = x2 + y2 * wave.width;
        const vector<unsigned>& patterns = propagator[pattern][direction];
        for(auto it = patterns.begin(), it_end = patterns.end(); it < it_end; ++it) {
          array<int, 4>& value = compatible.get(y2, x2, *it);
          value[direction]--;
          if(value[direction] == 0) {
            add_to_propagator(y2, x2, *it);
            wave.set(i2, *it, false);
          }
        }
      }
    }
  }
};
