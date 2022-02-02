#ifndef FAST_WFC_UTILS_ARRAY3D_HPP_
#define FAST_WFC_UTILS_ARRAY3D_HPP_

#include "assert.h"
#include <vector>

/**
 * Represent a 3D array.
 * The 3D array is stored in a single array, to improve cache usage.
 */
template <typename T> class Array3D {

public:
  /**
   * The dimensions of the 3D array.
   */
  std::size_t height;
  std::size_t width;
  std::size_t depth;

  /**
   * The array containing the data of the 3D array.
   */
  std::vector<T> data;

  /**
   * Build a 2D array given its height, width and depth.
   * All the arrays elements are initialized to default value.
   */
  Array3D(std::size_t height, std::size_t width, std::size_t depth) noexcept
      : height(height), width(width), depth(depth),
        data(width * height * depth) {}

  /**
   * Build a 2D array given its height, width and depth.
   * All the arrays elements are initialized to value
   */
  Array3D(std::size_t height, std::size_t width, std::size_t depth,
          T value) noexcept
      : height(height), width(width), depth(depth),
        data(width * height * depth, value) {}

  /**
   * Return a const reference to the element in the i-th line, j-th column, and
   * k-th depth. i must be lower than height, j lower than width, and k lower
   * than depth.
   */
  const T &get(std::size_t i, std::size_t j, std::size_t k) const noexcept {
    assert(i < height && j < width && k < depth);
    return data[i * width * depth + j * depth + k];
  }

  /**
   * Return a reference to the element in the i-th line, j-th column, and k-th
   * depth. i must be lower than height, j lower than width, and k lower than
   * depth.
   */
  T &get(std::size_t i, std::size_t j, std::size_t k) noexcept {
    return data[i * width * depth + j * depth + k];
  }

  /**
   * Check if two 3D arrays are equals.
   */
  bool operator==(const Array3D &a) const noexcept {
    if (height != a.height || width != a.width || depth != a.depth) {
      return false;
    }

    for (std::size_t i = 0; i < data.size(); i++) {
      if (a.data[i] != data[i]) {
        return false;
      }
    }
    return true;
  }
};

#endif // FAST_WFC_UTILS_ARRAY3D_HPP_
