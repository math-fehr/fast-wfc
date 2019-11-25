#include "wfc.hpp"
#include <limits>

namespace {
  /**
   * Normalize a vector so the sum of its elements is equal to 1.0f
   */
  std::vector<double>& normalize(std::vector<double>& v) {
    double sum_weights = 0.0;
    for(double weight: v) {
      sum_weights += weight;
    }

    double inv_sum_weights = 1.0/sum_weights;
    for(double& weight: v) {
      weight *= inv_sum_weights;
    }

    return v;
  }
}


Array2D<unsigned> WFC::wave_to_output() const noexcept {
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

WFC::WFC(bool periodic_output, int seed,
         std::vector<double> patterns_frequencies,
         Propagator::PropagatorState propagator, unsigned wave_height,
         unsigned wave_width)
  noexcept
  : gen(seed), patterns_frequencies(normalize(patterns_frequencies)),
    wave(wave_height, wave_width, patterns_frequencies),
    nb_patterns(propagator.size()),
    propagator(wave.height, wave.width, periodic_output, propagator) {}

std::optional<Array2D<unsigned>> WFC::run() noexcept {
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


WFC::ObserveStatus WFC::observe() noexcept {
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
    size_t chosen_value = nb_patterns - 1;

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
