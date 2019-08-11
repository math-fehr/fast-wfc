#ifndef FAST_WFC_DIRECTION_HPP_
#define FAST_WFC_DIRECTION_HPP_

/**
 * A direction is represented by an unsigned integer in the range [0; 3].
 * The x and y values of the direction can be retrieved in these tables.
 */
constexpr int directions_x[4] = {0, -1, 1, 0};
constexpr int directions_y[4] = {-1, 0, 0, 1};

/**
 * Return the opposite direction of direction.
 */
constexpr unsigned get_opposite_direction(unsigned direction) noexcept {
  return 3 - direction;
}

#endif // FAST_WFC_DIRECTION_HPP_
