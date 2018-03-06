#pragma once

#include <vector>
#include <stdint.h>
#include <limits>
#include <math.h>
#include <random>
#include <iostream>
#include "array2D.hpp"

using namespace std;

/**
 * Struct containing the values needed to compute the entropy of all the cells.
 * This struct is updated every time the wave is changed.
 * p'(pattern) is equal to patterns_frequencies[pattern] if wave.get(cell, pattern) is set to true, otherwise 0.
 */
struct EntropyMemoisation {
  vector<double> plogp_sum;     // The sum of p'(pattern) * log(p'(pattern)).
  vector<double> sum;           // The sum of p'(pattern).
  vector<double> log_sum;       // The log of sum.
  vector<unsigned> nb_patterns; // The number of patterns present in the wave in the cell.
  vector<double> entropy;       // The entropy of the cell.
};

/**
 * Contains the pattern possibilities in every cell.
 * Also contains information about cell entropy.
 */
class Wave {
private:

  /**
   * The patterns frequencies p given to wfc.
   */
  const vector<double> patterns_frequencies;

  /**
   * The precomputation of p * log(p).
   */
  const vector<double> plogp_patterns_frequencies;

  /**
   * The precomputation of min (p * log(p)) / 2.
   * This is used to define the maximum value of the noise.
   */
  const double half_min_plogp;

  /**
   * The memoisation of important values for the computation of entropy.
   */
  EntropyMemoisation memoisation;

  /**
   * This value is set to true if there is a contradiction in the wave (all elements set to false in a cell).
   */
  bool is_impossible;

  /**
   * The number of distinct patterns.
   */
  const unsigned nb_patterns;

  /**
   * The actual wave. data.get(index, pattern) is equal to 0 if the pattern can be placed in the cell index.
   */
  Array2D<uint8_t> data;

  /**
   * Return distribution * log(distribution).
   */
  static vector<double> get_plogp(const vector<double>& distribution) {
    vector<double> plogp;
    for(unsigned i = 0; i < distribution.size(); i++) {
      plogp.push_back(distribution[i] * log(distribution[i]));
    }
    return plogp;
  }

  /**
   * Return min(v) / 2.
   */
  static double get_half_min(const vector<double>& v) {
    double half_min = numeric_limits<double>::infinity();
    for(unsigned i = 0; i < v.size(); i++) {
      half_min = min(half_min, v[i] / 2.0);
    }
    return half_min;
  }

public:
  /**
   * The size of the wave.
   */
  const unsigned width;
  const unsigned height;
  const unsigned size;

  /**
   * Initialize the wave with every cell being able to have every pattern.
   */
  Wave(unsigned height, unsigned width, const vector<double>& patterns_frequencies) :
    patterns_frequencies(patterns_frequencies),
    plogp_patterns_frequencies(get_plogp(patterns_frequencies)),
    half_min_plogp(get_half_min(plogp_patterns_frequencies)),
    is_impossible(false),
    nb_patterns(patterns_frequencies.size()),
    data(width * height, nb_patterns, 1),
    width(width), height(height), size(height * width)
  {
    // Initialize the memoisation of entropy.
    double base_entropy = 0;
    double base_s = 0;
    double half_min_plogp = numeric_limits<double>::infinity();
    for(unsigned i = 0; i < nb_patterns; i++) {
      half_min_plogp = min(half_min_plogp, plogp_patterns_frequencies[i] / 2.0);
      base_entropy += plogp_patterns_frequencies[i];
      base_s += patterns_frequencies[i];
    }
    double log_base_s = log(base_s);
    double entropy_base = log_base_s - base_entropy / base_s;
    memoisation.plogp_sum = vector<double>(width * height, base_entropy);
    memoisation.sum = vector<double>(width * height, base_s);
    memoisation.log_sum = vector<double>(width * height, log_base_s);
    memoisation.nb_patterns = vector<unsigned>(width * height, nb_patterns);
    memoisation.entropy = vector<double>(width * height, entropy_base);
  }

  /**
   * Return true if pattern can be placed in cell index.
   */
  bool get(unsigned index, unsigned pattern) {
    return data.get(index, pattern);
  }

  /**
   * Return true if pattern can be placed in cell (i,j)
   */
  bool get(unsigned i, unsigned j, unsigned pattern) {
    return get(i * width + j, pattern);
  }

  /**
   * Set the value of pattern in cell index.
   */
  void set(unsigned index, unsigned pattern, bool value) {
    bool old_value = data.get(index, pattern);
    // If the value isn't changed, nothing needs to be done.
    if(old_value == value) {
      return;
    }
    // Otherwise, the memoisation should be updated.
    data.get(index, pattern) = value;
    memoisation.plogp_sum[index] -= plogp_patterns_frequencies[pattern];
    memoisation.sum[index] -= patterns_frequencies[pattern];
    memoisation.log_sum[index] = log(memoisation.sum[index]);
    memoisation.nb_patterns[index]--;
    memoisation.entropy[index] = memoisation.log_sum[index] - memoisation.plogp_sum[index] / memoisation.sum[index];
    // If there is no patterns possible in the cell, then there is a contradiction.
    if(memoisation.nb_patterns[index] == 0) {
      is_impossible = true;
    }
  }

  /**
   * Set the value of pattern in cell (i,j).
   */
  void set(unsigned i, unsigned j, unsigned pattern, bool value) {
    set(i * width + j, pattern, value);
  }

  /**
   * Return the index of the cell with lowest entropy different of 0.
   * If there is a contradiction in the wave, return -2.
   * If every cell is decided, return -1.
   */
  int get_min_entropy(minstd_rand& gen) {
    if(is_impossible) {
      return -2;
    }

    std::uniform_real_distribution<> dis(0,half_min_plogp);

    // The minimum entropy (plus a small noise)
    double min = numeric_limits<double>::infinity();
    int argmin = -1;

    for(unsigned i = 0; i < size; i++) {

      // If the cell is decided, we do not compute the entropy (which is equal to 0).
      double nb_patterns = memoisation.nb_patterns[i];
      if(nb_patterns == 1) {
        continue;
      }

      // Otherwise, we take the memoised entropy.
      double entropy = memoisation.entropy[i];

      // We first check if the entropy is less than the minimum.
      // This is important to reduce noise computation (which is not negligible).
      if(entropy <= min) {

        // Then, we add noise to decide randomly which will be chosen.
        // noise is smaller than the smallest p * log(p), so the minimum entropy will always be chosen.
        double noise = dis(gen);
        if(entropy + noise < min) {
          min = entropy + noise;
          argmin = i;
        }
      }
    }
    return argmin;
  }

};
