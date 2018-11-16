#ifndef FAST_WFC_UTILS_UTILS_HPP_
#define FAST_WFC_UTILS_UTILS_HPP_

#include <string>
#include <vector>

/**
 * Get the directory containing the file file_path.
 */
std::string get_dir(const std::string& file_path) {
  for(unsigned i = file_path.size(); i > 0; --i) {
    if(file_path[i] == '/') {
      return std::string(file_path.begin(), file_path.begin() + i);
    }
  }
  if(file_path[0] == '/') {
    return "/";
  } else {
    return ".";
  }
}

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

#endif // FAST_WFC_UTILS_UTILS_HPP_
