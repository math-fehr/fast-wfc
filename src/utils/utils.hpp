#include <string>

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
