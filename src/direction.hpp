#pragma once

constexpr int directions_x[4] = {0, -1, 1, 0};
constexpr int directions_y[4] = {-1, 0, 0, 1};

unsigned get_opposite_direction(unsigned direction) {
  return 3 - direction;
}
