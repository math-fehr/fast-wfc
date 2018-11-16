#ifndef FAST_WFC_WFC_HPP_
#define FAST_WFC_WFC_HPP_

#include <cmath>
#include <limits>
#include <random>
#include <unordered_map>

#include "utils/array2D.hpp"
#include "utils/utils.hpp"
#include "propagator.hpp"
#include "wave.hpp"

/**
 * Class containing the generic WFC algorithm.
 */
class WFC {
private:
  /**
   * The random number generator.
   */
  std::minstd_rand gen;

  /**
   * The distribution of the patterns as given in input.
   */
  const std::vector<double> patterns_frequencies;

  /**
   * The wave, indicating which patterns can be put in which cell.
   */
  Wave wave;

  /**
   * The number of distinct patterns.
   */
  const unsigned nb_patterns;

  /**
   * The propagator, used to propagate the information in the wave.
   */
  Propagator propagator;

  /**
   * Transform the wave to a valid output (a 2d array of patterns that aren't in
   * contradiction). This function should be used only when all cell of the wave
   * are defined.
   */
  Array2D<unsigned> wave_to_output() const noexcept {
    Array2D<unsigned> output_patterns(wave.height, wave.width);
    for (unsigned i = 0; i < wave.size; i++) {
      for (unsigned k = 0; k < nb_patterns; k++) {
        if (wave.get(i, k)) {
          output_patterns.data[i] = k;
        }
      }
    }
    return output_patterns;
  }

public:
  /**
   * Basic constructor initializing the algorithm.
   */
  WFC(bool periodic_output, int seed, std::vector<double> patterns_frequencies,
      Propagator::PropagatorState propagator, unsigned wave_height,
      unsigned wave_width)
  noexcept
    : gen(seed), patterns_frequencies(normalize(patterns_frequencies)),
        wave(wave_height, wave_width, patterns_frequencies),
        nb_patterns(propagator.size()),
        propagator(wave.height, wave.width, periodic_output, propagator) {}

  /**
   * Run the algorithm, and return a result if it succeeded.
   */
  std::optional<Array2D<unsigned>> run() noexcept {
    while (true) {

      // Define the value of an undefined cell.
      ObserveStatus result = observe();

      // Check if the algorithm has terminated.
      if (result == failure) {
        return std::nullopt;
      } else if (result == success) {
        return wave_to_output();
      }

      // Propagate the information.
      propagator.propagate(wave);
    }
  }

  /**
   * Return value of observe.
   */
  enum ObserveStatus {
    success,    // WFC has finished and has succeeded.
    failure,    // WFC has finished and failed.
    to_continue // WFC isn't finished.
  };

  /**
   * Define the value of the cell with lowest entropy.
   */
  ObserveStatus observe() noexcept {
    // Get the cell with lowest entropy.
    int argmin = wave.get_min_entropy(gen);

    // If there is a contradiction, the algorithm has failed.
    if (argmin == -2) {
      return failure;
    }

    // If the lowest entropy is 0, then the algorithm has succeeded and
    // finished.
    if (argmin == -1) {
      wave_to_output();
      return success;
    }

    // Choose an element according to the pattern distribution
    double s = 0;
    for (unsigned k = 0; k < nb_patterns; k++) {
      s += wave.get(argmin, k) ? patterns_frequencies[k] : 0;
    }

    std::uniform_real_distribution<> dis(0, s);
    double random_value = dis(gen);
    unsigned chosen_value = nb_patterns - 1;

    for (unsigned k = 0; k < nb_patterns; k++) {
      random_value -= wave.get(argmin, k) ? patterns_frequencies[k] : 0;
      if (random_value <= 0) {
        chosen_value = k;
        break;
      }
    }

    // And define the cell with the pattern.
    for (unsigned k = 0; k < nb_patterns; k++) {
      if (wave.get(argmin, k) != (k == chosen_value)) {
        propagator.add_to_propagator(argmin / wave.width, argmin % wave.width,
                                     k);
        wave.set(argmin, k, false);
      }
    }

    return to_continue;
  }

  /**
   * Propagate the information of the wave.
   */
  void propagate() noexcept { propagator.propagate(wave); }

  /**
   * Remove pattern from cell (i,j).
   */
  void remove_wave_pattern(unsigned i, unsigned j, unsigned pattern) noexcept {
    if (wave.get(i, j, pattern)) {
      wave.set(i, j, pattern, false);
      propagator.add_to_propagator(i, j, pattern);
    }
  }
};

#endif // FAST_WFC_WFC_HPP_
