#ifndef FAST_WFC_OVERLAPPING_WFC_HPP_
#define FAST_WFC_OVERLAPPING_WFC_HPP_

#include <vector>
#include <algorithm>
#include <unordered_map>

#include "utils/array2D.hpp"
#include "wfc.hpp"

/**
 * Options needed to use the overlapping wfc.
 */
struct OverlappingWFCOptions {
  bool periodic_input;  // True if the input is toric.
  bool periodic_output; // True if the output is toric.
  unsigned out_height;  // The height of the output in pixels.
  unsigned out_width;   // The width of the output in pixels.
  unsigned symmetry; // The number of symmetries (the order is defined in wfc).
  bool ground;       // True if the ground needs to be set (see init_ground).
  unsigned pattern_size; // The width and height in pixel of the patterns.

  /**
   * Get the wave height given these options.
   */
  unsigned get_wave_height() const noexcept {
    return periodic_output ? out_height : out_height - pattern_size + 1;
  }

  /**
   * Get the wave width given these options.
   */
  unsigned get_wave_width() const noexcept {
    return periodic_output ? out_width : out_width - pattern_size + 1;
  }
};

/**
 * Class generating a new image with the overlapping WFC algorithm.
 */
template <typename T> class OverlappingWFC {

private:
  /**
   * The input image. T is usually a color.
   */
  Array2D<T> input;

  /**
   * Options needed by the algorithm.
   */
  OverlappingWFCOptions options;

  /**
   * The array of the different patterns extracted from the input.
   */
  std::vector<Array2D<T>> patterns;

  /**
   * The underlying generic WFC algorithm.
   */
  WFC wfc;

  /**
   * Constructor initializing the wfc.
   * This constructor is called by the other constructors.
   * This is necessary in order to initialize wfc only once.
   */
  OverlappingWFC(
      const Array2D<T> &input, const OverlappingWFCOptions &options,
      const int &seed,
      const std::pair<std::vector<Array2D<T>>, std::vector<double>> &patterns,
      const std::vector<std::array<std::vector<unsigned>, 4>>
          &propagator) noexcept
      : input(input), options(options), patterns(patterns.first),
        wfc(options.periodic_output, seed, patterns.second, propagator,
            options.get_wave_height(), options.get_wave_width()) {
    // If necessary, the ground is set.
    if (options.ground) {
      init_ground(wfc, input, patterns.first, options);
    }
  }

  /**
   * Constructor used only to call the other constructor with more computed
   * parameters.
   */
  OverlappingWFC(const Array2D<T> &input, const OverlappingWFCOptions &options,
                 const int &seed,
                 const std::pair<std::vector<Array2D<T>>, std::vector<double>>
                     &patterns) noexcept
      : OverlappingWFC(input, options, seed, patterns,
                       generate_compatible(patterns.first)) {}

  /**
   * Init the ground of the output image.
   * The lowest middle pattern is used as a floor (and ceiling when the input is
   * toric) and is placed at the lowest possible pattern position in the output
   * image, on all its width. The pattern cannot be used at any other place in
   * the output image.
   */
  void init_ground(WFC &wfc, const Array2D<T> &input,
                   const std::vector<Array2D<T>> &patterns,
                   const OverlappingWFCOptions &options) noexcept {
    unsigned ground_pattern_id =
        get_ground_pattern_id(input, patterns, options);

    // Place the pattern in the ground.
    for (unsigned j = 0; j < options.get_wave_width(); j++) {
      set_pattern(ground_pattern_id, options.get_wave_height() - 1, j);
    }

    // Remove the pattern from the other positions.
    for (unsigned i = 0; i < options.get_wave_height() - 1; i++) {
      for (unsigned j = 0; j < options.get_wave_width(); j++) {
        wfc.remove_wave_pattern(i, j, ground_pattern_id);
      }
    }

    // Propagate the information with wfc.
    wfc.propagate();
  }

  /**
   * Return the id of the lowest middle pattern.
   */
  static unsigned
  get_ground_pattern_id(const Array2D<T> &input,
                        const std::vector<Array2D<T>> &patterns,
                        const OverlappingWFCOptions &options) noexcept {
    // Get the pattern.
    Array2D<T> ground_pattern =
        input.get_sub_array(input.height - 1, input.width / 2,
                            options.pattern_size, options.pattern_size);

    // Retrieve the id of the pattern.
    for (unsigned i = 0; i < patterns.size(); i++) {
      if (ground_pattern == patterns[i]) {
        return i;
      }
    }

    // The pattern exists.
    assert(false);
    return 0;
  }

  /**
   * Return the list of patterns, as well as their probabilities of apparition.
   */
  static std::pair<std::vector<Array2D<T>>, std::vector<double>>
  get_patterns(const Array2D<T> &input,
               const OverlappingWFCOptions &options) noexcept {
    std::unordered_map<Array2D<T>, unsigned> patterns_id;
    std::vector<Array2D<T>> patterns;

    // The number of time a pattern is seen in the input image.
    std::vector<double> patterns_weight;

    std::vector<Array2D<T>> symmetries(
        8, Array2D<T>(options.pattern_size, options.pattern_size));
    unsigned max_i = options.periodic_input
                         ? input.height
                         : input.height - options.pattern_size + 1;
    unsigned max_j = options.periodic_input
                         ? input.width
                         : input.width - options.pattern_size + 1;

    for (unsigned i = 0; i < max_i; i++) {
      for (unsigned j = 0; j < max_j; j++) {
        // Compute the symmetries of every pattern in the image.
        symmetries[0].data =
            input
                .get_sub_array(i, j, options.pattern_size, options.pattern_size)
                .data;
        symmetries[1].data = symmetries[0].reflected().data;
        symmetries[2].data = symmetries[0].rotated().data;
        symmetries[3].data = symmetries[2].reflected().data;
        symmetries[4].data = symmetries[2].rotated().data;
        symmetries[5].data = symmetries[4].reflected().data;
        symmetries[6].data = symmetries[4].rotated().data;
        symmetries[7].data = symmetries[6].reflected().data;

        // The number of symmetries in the option class define which symetries
        // will be used.
        for (unsigned k = 0; k < options.symmetry; k++) {
          auto res = patterns_id.insert(
              std::make_pair(symmetries[k], patterns.size()));

          // If the pattern already exist, we just have to increase its number
          // of appearance.
          if (!res.second) {
            patterns_weight[res.first->second] += 1;
          } else {
            patterns.push_back(symmetries[k]);
            patterns_weight.push_back(1);
          }
        }
      }
    }

    return {patterns, patterns_weight};
  }

  /**
   * Return true if the pattern1 is compatible with pattern2
   * when pattern2 is at a distance (dy,dx) from pattern1.
   */
  static bool agrees(const Array2D<T> &pattern1, const Array2D<T> &pattern2,
                     int dy, int dx) noexcept {
    unsigned xmin = dx < 0 ? 0 : dx;
    unsigned xmax = dx < 0 ? dx + pattern2.width : pattern1.width;
    unsigned ymin = dy < 0 ? 0 : dy;
    unsigned ymax = dy < 0 ? dy + pattern2.height : pattern1.width;

    // Iterate on every pixel contained in the intersection of the two pattern.
    for (unsigned y = ymin; y < ymax; y++) {
      for (unsigned x = xmin; x < xmax; x++) {
        // Check if the color is the same in the two patterns in that pixel.
        if (pattern1.get(y, x) != pattern2.get(y - dy, x - dx)) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * Precompute the function agrees(pattern1, pattern2, dy, dx).
   * If agrees(pattern1, pattern2, dy, dx), then compatible[pattern1][direction]
   * contains pattern2, where direction is the direction defined by (dy, dx)
   * (see direction.hpp).
   */
  static std::vector<std::array<std::vector<unsigned>, 4>>
  generate_compatible(const std::vector<Array2D<T>> &patterns) noexcept {
    std::vector<std::array<std::vector<unsigned>, 4>> compatible =
        std::vector<std::array<std::vector<unsigned>, 4>>(patterns.size());

    // Iterate on every dy, dx, pattern1 and pattern2
    for (unsigned pattern1 = 0; pattern1 < patterns.size(); pattern1++) {
      for (unsigned direction = 0; direction < 4; direction++) {
        for (unsigned pattern2 = 0; pattern2 < patterns.size(); pattern2++) {
          if (agrees(patterns[pattern1], patterns[pattern2],
                     directions_y[direction], directions_x[direction])) {
            compatible[pattern1][direction].push_back(pattern2);
          }
        }
      }
    }

    return compatible;
  }

  /**
   * Transform a 2D array containing the patterns id to a 2D array containing
   * the pixels.
   */
  Array2D<T> to_image(const Array2D<unsigned> &output_patterns) const noexcept {
    Array2D<T> output = Array2D<T>(options.out_height, options.out_width);

    if (options.periodic_output) {
      for (unsigned y = 0; y < options.get_wave_height(); y++) {
        for (unsigned x = 0; x < options.get_wave_width(); x++) {
          output.get(y, x) = patterns[output_patterns.get(y, x)].get(0, 0);
        }
      }
    } else {
      for (unsigned y = 0; y < options.get_wave_height(); y++) {
        for (unsigned x = 0; x < options.get_wave_width(); x++) {
          output.get(y, x) = patterns[output_patterns.get(y, x)].get(0, 0);
        }
      }
      for (unsigned y = 0; y < options.get_wave_height(); y++) {
        const Array2D<T> &pattern =
            patterns[output_patterns.get(y, options.get_wave_width() - 1)];
        for (unsigned dx = 1; dx < options.pattern_size; dx++) {
          output.get(y, options.get_wave_width() - 1 + dx) = pattern.get(0, dx);
        }
      }
      for (unsigned x = 0; x < options.get_wave_width(); x++) {
        const Array2D<T> &pattern =
            patterns[output_patterns.get(options.get_wave_height() - 1, x)];
        for (unsigned dy = 1; dy < options.pattern_size; dy++) {
          output.get(options.get_wave_height() - 1 + dy, x) =
              pattern.get(dy, 0);
        }
      }
      const Array2D<T> &pattern = patterns[output_patterns.get(
          options.get_wave_height() - 1, options.get_wave_width() - 1)];
      for (unsigned dy = 1; dy < options.pattern_size; dy++) {
        for (unsigned dx = 1; dx < options.pattern_size; dx++) {
          output.get(options.get_wave_height() - 1 + dy,
                     options.get_wave_width() - 1 + dx) = pattern.get(dy, dx);
        }
      }
    }

    return output;
  }

  std::optional<unsigned> get_pattern_id(const Array2D<T> &pattern) {
    unsigned* pattern_id = std::find(patterns.begin(), patterns.end(), pattern);

    if (pattern_id != patterns.end()) {
      return *pattern_id;
    }

    return std::nullopt;
  }

  /**
   * Set the pattern at a specific position, given its pattern id
   * pattern_id needs to be a valid pattern id, and i and j needs to be in the wave range
   */
  void set_pattern(unsigned pattern_id, unsigned i, unsigned j) noexcept {
    for (unsigned p = 0; p < patterns.size(); p++) {
      if (pattern_id != p) {
        wfc.remove_wave_pattern(i, j, p);
      }
    }
  }

public:
  /**
   * The constructor used by the user.
   */
  OverlappingWFC(const Array2D<T> &input, const OverlappingWFCOptions &options,
                 int seed) noexcept
      : OverlappingWFC(input, options, seed, get_patterns(input, options)) {}

  /**
   * Set the pattern at a specific position.
   * Returns false if the given pattern does not exist, or if the
   * coordinates are not in the wave
   */
  bool set_pattern(const Array2D<T>& pattern, unsigned i, unsigned j) noexcept {
    auto pattern_id = get_pattern_id(pattern);

    if (pattern_id == std::nullopt || i >= options.get_wave_height() || j >= options.get_wave_width()) {
      return false;
    }

    set_pattern(pattern_id, i, j);
    return true;
  }

  /**
   * Run the WFC algorithm, and return the result if the algorithm succeeded.
   */
  std::optional<Array2D<T>> run() noexcept {
    std::optional<Array2D<unsigned>> result = wfc.run();
    if (result.has_value()) {
      return to_image(*result);
    }
    return std::nullopt;
  }
};

#endif // FAST_WFC_WFC_HPP_
