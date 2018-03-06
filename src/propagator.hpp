#pragma once

#include <vector>
#include <tuple>
#include "array2D.hpp"
#include "array3D.hpp"
#include "wave.hpp"
#include "direction.hpp"

using namespace std;

/**
 * Propagate information about patterns in the wave.
 */
class Propagator {

private:

  /**
   * The size of the patterns.
   */
  const unsigned patterns_size;

  /**
   * propagator[pattern1][direction] contains all the patterns that can be placed in
   * next to pattern1 in the direction direction.
   */
  const vector<array<vector<unsigned>, 4>> propagator;

  /**
   * The wave width and height.
   */
  const unsigned wave_width;
  const unsigned wave_height;

  /**
   * True if the wave and the output is toric.
   */
  const bool periodic_output;

  /**
   * All the tuples (y, x, pattern) that should be propagated.
   * The tuple should be propagated when wave.get(y, x, pattern) is set to false.
   */
  vector<tuple<unsigned, unsigned, unsigned>> propagating;

  /**
   * compatible.get(y, x, pattern)[direction] contains the number of patterns present in the wave
   * that can be placed in the cell next to (y,x) in the opposite direction of direction without being
   * in contradiction with pattern placed in (y,x).
   * If wave.get(y, x, pattern) is set to false, then compatible.get(y, x, pattern) has every element negative or null
   */
  Array3D<array<int, 4>> compatible;

  /**
   * Initialize compatible.
   */
  void init_compatible() {
    array<int, 4> value;
    // We compute the number of pattern compatible in all directions.
    for(unsigned y = 0; y < wave_height; y++) {
      for(unsigned x = 0; x < wave_width; x++) {
        for(unsigned pattern = 0; pattern < patterns_size; pattern++) {
          for(int direction = 0; direction < 4; direction++) {
            value[direction] = propagator[pattern][get_opposite_direction(direction)].size();
          }
          compatible.get(y, x, pattern) = value;
        }
      }
    }
  }

public:

  /**
   * Constructor building the propagator and initializing compatible.
   */
  Propagator(unsigned wave_height, unsigned wave_width, bool periodic_output,
             vector<array<vector<unsigned>, 4>> propagator) :
    patterns_size(propagator.size()), propagator(propagator), wave_width(wave_width),
    wave_height(wave_height), periodic_output(periodic_output),
    compatible(wave_height, wave_width, patterns_size)
  {
    init_compatible();
  }

  /**
   * Add an element to the propagator.
   * This function is called when wave.get(y, x, pattern) is set to false.
   */
  void add_to_propagator(unsigned y, unsigned x, unsigned pattern) {
    // All the direction are set to 0, since the pattern cannot be set in (y,x).
    array<int, 4> temp = {};
    compatible.get(y,x,pattern) = temp;
    propagating.emplace_back(y, x, pattern);
  }

  /**
   * Propagate the information given with add_to_propagator.
   */
  void propagate(Wave& wave) {

    // We propagate every element while there is element to propagate.
    while(propagating.size() != 0) {

      // The cell and pattern that has been set to false.
      unsigned y1, x1, pattern;
      tie(y1, x1, pattern) = propagating.back();
      propagating.pop_back();

      // We propagate the information in all 4 directions.
      for(unsigned direction = 0; direction < 4; direction++) {

        // We get the next cell in the direction direction.
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

        // The index of the second cell, and the patterns compatible
        unsigned i2 = x2 + y2 * wave.width;
        const vector<unsigned>& patterns = propagator[pattern][direction];

        // For every pattern that could be placed in that cell without being in contradiction with pattern1
        for(auto it = patterns.begin(), it_end = patterns.end(); it < it_end; ++it) {

          // We decrease the number of compatible patterns in the opposite direction
          // If the pattern was discarded from the wave, the element is still negative, which is not a problem
          array<int, 4>& value = compatible.get(y2, x2, *it);
          value[direction]--;

          // If the element was set to 0 with this operation, we need to remove the pattern
          // from the wave, and propagate the information
          if(value[direction] == 0) {
            add_to_propagator(y2, x2, *it);
            wave.set(i2, *it, false);
          }
        }
      }
    }
  }
};
