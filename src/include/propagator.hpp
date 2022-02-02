#ifndef FAST_WFC_PROPAGATOR_HPP_
#define FAST_WFC_PROPAGATOR_HPP_

#include "direction.hpp"
#include "utils/array3D.hpp"
#include <tuple>
#include <vector>
#include <array>

class Wave;

/**
 * Propagate information about patterns in the wave.
 */
class Propagator {
public:
  using PropagatorState = std::vector<std::array<std::vector<unsigned>, 4>>;

private:
  /**
   * The size of the patterns.
   */
  const std::size_t patterns_size;

  /**
   * propagator[pattern1][direction] contains all the patterns that can
   * be placed in next to pattern1 in the direction direction.
   */
  PropagatorState propagator_state;

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
   * The tuple should be propagated when wave.get(y, x, pattern) is set to
   * false.
   */
  std::vector<std::tuple<unsigned, unsigned, unsigned>> propagating;

  /**
   * compatible.get(y, x, pattern)[direction] contains the number of patterns
   * present in the wave that can be placed in the cell next to (y,x) in the
   * opposite direction of direction without being in contradiction with pattern
   * placed in (y,x). If wave.get(y, x, pattern) is set to false, then
   * compatible.get(y, x, pattern) has every element negative or null
   */
  Array3D<std::array<int, 4>> compatible;

  /**
   * Initialize compatible.
   */
  void init_compatible() noexcept;

public:
  /**
   * Constructor building the propagator and initializing compatible.
   */
  Propagator(unsigned wave_height, unsigned wave_width, bool periodic_output,
             PropagatorState propagator_state) noexcept
      : patterns_size(propagator_state.size()),
        propagator_state(propagator_state), wave_width(wave_width),
        wave_height(wave_height), periodic_output(periodic_output),
        compatible(wave_height, wave_width, patterns_size) {
    init_compatible();
  }

  /**
   * Add an element to the propagator.
   * This function is called when wave.get(y, x, pattern) is set to false.
   */
  void add_to_propagator(unsigned y, unsigned x, unsigned pattern) noexcept {
    // All the direction are set to 0, since the pattern cannot be set in (y,x).
    std::array<int, 4> temp = {};
    compatible.get(y, x, pattern) = temp;
    propagating.emplace_back(y, x, pattern);
  }

  /**
   * Propagate the information given with add_to_propagator.
   */
  void propagate(Wave &wave) noexcept;
};

#endif // FAST_WFC_PROPAGATOR_HPP_
