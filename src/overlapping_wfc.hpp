#pragma once

#include "wfc.hpp"
#include <iostream>

/**
 * Options needed to use the overlapping wfc.
 */
struct OverlappingWFCOptions {
  bool periodic_input;     // True if the input is toric.
  bool periodic_output;    // True if the output is toric.
  unsigned out_height;     // The height of the output in pixels.
  unsigned out_width;      // The width of the output in pixels.
  unsigned symmetry;       // The number of symmetries (the order is defined in wfc).
  bool ground;             // True if the ground needs to be set (see init_ground).
  unsigned pattern_size;   // The width and height in pixel of the patterns.
};

/**
 * Class generating a new image with the overlapping WFC algorithm.
 */
template<typename T>
class OverlappingWFC {

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
  vector<Array2D<T>> patterns;

  /**
   * The underlying generic WFC algorithm.
   */
  WFC wfc;

  /**
   * Constructor initializing the wfc.
   * This constructor is called by the other constructors.
   * This is necessary in order to initialize wfc only once.
   */
  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, const int& seed,
                 const pair<vector<Array2D<T>>, vector<double>>& patterns,
                 const vector<array<vector<unsigned>, 4>>& propagator) :
    input(input),
    options(options),
    patterns(patterns.first),
    wfc(options.periodic_output, seed, patterns.second, propagator,
        options.periodic_output ? options.out_height : options.out_height - options.pattern_size + 1,
        options.periodic_output ? options.out_width : options.out_width - options.pattern_size + 1)
  {
    // If necessary, the ground is set.
    if(options.ground) {
      init_ground(wfc, input, patterns.first, options);
    }
  }

  /**
   * Constructor used only to call the other constructor with more computed parameters.
   */
  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, const int& seed,
                 const pair<vector<Array2D<T>>, vector<double>>& patterns) :
    OverlappingWFC(input, options, seed, patterns, generate_compatible(patterns.first))
  {}


  /**
   * Init the ground of the output image.
   * The lowest middle pattern is used as a floor (and ceiling when the input is toric)
   * and is placed at the lowest possible pattern position in the output image, on all its width.
   * The pattern cannot be used at any other place in the output image.
   */
  static void init_ground(WFC& wfc, const Array2D<T>& input, const vector<Array2D<T>>& patterns, const OverlappingWFCOptions& options) {
    unsigned ground_pattern_id = get_ground_pattern_id(input, patterns, options);

    // Place the pattern in the ground.
    for(unsigned j = 0; j < wfc.wave.width; j++) {
      for(unsigned p = 0; p < patterns.size(); p++) {
        if(ground_pattern_id != p) {
          wfc.remove_wave_pattern(wfc.wave.height - 1, j, p);
        }
      }
    }

    // Remove the pattern from the other positions.
    for(unsigned i = 0; i < wfc.wave.height - 1; i++) {
      for(unsigned j = 0; j < wfc.wave.width; j++) {
        wfc.remove_wave_pattern(i, j, ground_pattern_id);
      }
    }

    // Propagate the information with wfc.
    wfc.propagate();
  }

  /**
   * Return the id of the lowest middle pattern.
   */
  static unsigned get_ground_pattern_id(const Array2D<T>& input, const vector<Array2D<T>>& patterns, const OverlappingWFCOptions& options) {
    // Get the pattern.
    Array2D<T> ground_pattern = input.get_sub_array(input.height - 1, input.width / 2, options.pattern_size, options.pattern_size);

    // Retrieve the id of the pattern.
    for(unsigned i = 0; i < patterns.size(); i++) {
      if(ground_pattern == patterns[i]) {
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
  static pair<vector<Array2D<T>>, vector<double>> get_patterns(const Array2D<T>& input, const OverlappingWFCOptions& options) {
    unordered_map<Array2D<T>, unsigned> patterns_id;
    vector<Array2D<T>> patterns;

    // The number of time a pattern is seen in the input image.
    vector<double> patterns_frequency;

    vector<Array2D<T>> symmetries(8, Array2D<T>(options.pattern_size, options.pattern_size));
    unsigned max_i = options.periodic_input ? input.height : input.height - options.pattern_size + 1;
    unsigned max_j = options.periodic_input ? input.width : input.width - options.pattern_size + 1;

    for(unsigned i = 0; i < max_i; i++) {
      for(unsigned j = 0; j < max_j; j++) {
        // Compute the symmetries of every pattern in the image.
        symmetries[0].data = input.get_sub_array(i, j, options.pattern_size, options.pattern_size).data;
        symmetries[1].data = symmetries[0].reflected().data;
        symmetries[2].data = symmetries[0].rotated().data;
        symmetries[3].data = symmetries[2].reflected().data;
        symmetries[4].data = symmetries[2].rotated().data;
        symmetries[5].data = symmetries[4].reflected().data;
        symmetries[6].data = symmetries[4].rotated().data;
        symmetries[7].data = symmetries[6].reflected().data;

        // The number of symmetries in the option class define which symetries will be used.
        for(unsigned k = 0; k<options.symmetry; k++) {
          auto res = patterns_id.insert(make_pair(symmetries[k],patterns.size()));

          // If the pattern already exist, we just have to increase its number of appearance.
          if(!res.second) {
            patterns_frequency[res.first->second] += 1;
          } else {
            patterns.push_back(symmetries[k]);
            patterns_frequency.push_back(1);
          }
        }
      }
    }

    return {patterns, patterns_frequency};
  }

  /**
   * Return true if the pattern1 is compatible with pattern2
   * when pattern2 is at a distance (dy,dx) from pattern1.
   */
  static bool agrees(const Array2D<T>& pattern1, const Array2D<T>& pattern2, int dy, int dx) {
    unsigned xmin = dx < 0 ? 0 : dx;
    unsigned xmax = dx < 0 ? dx + pattern2.width : pattern1.width;
    unsigned ymin = dy < 0 ? 0 : dy;
    unsigned ymax = dy < 0 ? dy + pattern2.height : pattern1.width;

    // Iterate on every pixel contained in the intersection of the two pattern.
    for(unsigned y = ymin; y < ymax; y++) {
      for(unsigned x = xmin; x < xmax; x++) {
        // Check if the color is the same in the two patterns in that pixel.
        if(pattern1.get(y,x) != pattern2.get(y-dy,x-dx)) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * Precompute the function agrees(pattern1, pattern2, dy, dx).
   * If agrees(pattern1, pattern2, dy, dx), then compatible[pattern1][direction] contains pattern2,
   * where direction is the direction defined by (dy, dx) (see direction.hpp).
   */
  //TODO rename compatible ?
  static vector<array<vector<unsigned>, 4>> generate_compatible(const vector<Array2D<T>>& patterns) {
    vector<array<vector<unsigned>, 4>> compatible = vector<array<vector<unsigned>, 4>>(patterns.size());

    // Iterate on every dy, dx, pattern1 and pattern2
    for(unsigned pattern1 = 0; pattern1 < patterns.size(); pattern1++) {
      for(unsigned direction = 0; direction < 4; direction++) {
        for(unsigned pattern2 = 0; pattern2 < patterns.size(); pattern2++) {
          if(agrees(patterns[pattern1], patterns[pattern2], directions_y[direction], directions_x[direction])) {
            compatible[pattern1][direction].push_back(pattern2);
          }
        }
      }
    }

    return compatible;
  }


  /**
   * Transform a 2D array containing the patterns id to a 2D array containing the pixels.
   */
  Array2D<T> to_image(const Array2D<unsigned>& output_patterns) {
    Array2D<T> output = Array2D<T>(options.out_height, options.out_width);

    if(wfc.periodic_output) {
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          output.get(y,x) = patterns[output_patterns.get(y,x)].get(0,0);
        }
      }
    } else { //TODO change bad code
      for(unsigned y = 0; y < wfc.wave.height; y++) {
        for(unsigned x = 0; x < wfc.wave.width; x++) {
          for(unsigned dy = 0; dy < options.pattern_size; dy++) {
            for(unsigned dx = 0; dx < options.pattern_size; dx++) {
              output.get(y + dy, x + dx) = patterns[output_patterns.get(y,x)].get(dy,dx);
            }
          }
        }
      }
    }

    return output;
  }

public:

  /**
   * The constructor used by the user.
   */
  OverlappingWFC(const Array2D<T>& input, const OverlappingWFCOptions& options, int seed) :
    OverlappingWFC(input, options, seed, get_patterns(input, options))
  {}

  /**
   * Run the WFC algorithm, and return the result if the algorithm succeeded.
   */
  std::optional<Array2D<T>> run() {
    std::optional<Array2D<unsigned>> result = wfc.run();
    if(result.has_value()) {
      return to_image(*result);
    }
    return nullopt;
  }
};
