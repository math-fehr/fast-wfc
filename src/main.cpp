#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "matrix.hpp"
#include "wfc.hpp"
#include "overlapping_wfc.hpp"
#include "rapidxml.hpp"
#include <gperftools/profiler.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;
using namespace rapidxml;

struct Color {
  unsigned char r,g,b;
  bool operator==(const Color& c) const {
    return r == c.r && g == c.g && b == c.b;
  }
  bool operator!=(const Color& c) const {
    return !(c == *this);
  }
  explicit operator unsigned() const {
    return r + 256 * g + 256*256*b;
  }
};

ostream& operator<<(std::ostream& os, const Color& c) {
  os << "(" << (unsigned)c.r << "," << (unsigned)c.g << "," << (unsigned)c.b << ")";
  return os;
}


Matrix<Color> read_file(const string& file_path) {
  int width;
  int height;
  int num_components;
  // TODO check error
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 3);
  Matrix<Color> m = Matrix<Color>(width, height);
  for(unsigned i = 0; i < (unsigned)height; i++) {
    for(unsigned j = 0; j < (unsigned)width; j++) {
      unsigned index = 3 * (i * width + j);
      m.data[i * width + j] = {data[index], data[index + 1], data[index + 2]};
    }
  }
  free(data);
  return m;
}

void write_file(const string& file_path, const Matrix<Color>& m) {
  stbi_write_png(file_path.c_str(), m.width, m.height, 3, (const unsigned char*)m.data.data(),0);
}

string get_attribute(xml_node<>* node, const string& attribute, const string& default_value) {
  if(node->first_attribute(attribute.c_str()) != nullptr) {
    return node->first_attribute(attribute.c_str())->value();
  } else {
    return default_value;
  }
}

void read_config_file(const string& config_path) {
  xml_document<> doc;
  xml_node<>* root_node;
  ifstream config_file(config_path);
  vector<char> buffer((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  root_node = doc.first_node("samples");
  ProfilerStart("profile.log");
  for (xml_node<> * node = root_node->first_node("overlapping"); node; node = node->next_sibling("overlapping")) {
    string name = node->first_attribute("name")->value();
    string N = node->first_attribute("N")->value();
    string periodic_output = get_attribute(node, "periodic", "False");
    string periodic_input = get_attribute(node, "periodicInput", "True");
    string ground = get_attribute(node, "ground", "0");
    string symmetry = get_attribute(node, "symmetry", "8");
    string screenshots = get_attribute(node, "screenshots", "2");
    string width = get_attribute(node, "width", "96");
    string height = get_attribute(node, "width", "96");

    int N_value = stoi(N);
    int width_value = stoi(width);
    int height_value = stoi(height);
    int symmetry_value = stoi(symmetry);
    int ground_value = stoi(ground);
    int screenshots_value = stoi(screenshots);
    bool periodic_input_value = periodic_input == "True";
    bool periodic_output_value = periodic_output == "True";

    cout << name << " started!" << endl;
    Matrix<Color> m = read_file("samples/" + name + ".png");
    for(int i = 0; i < screenshots_value; i++) {
      for(unsigned test = 0; test < 10; test++) {
        pair<vector<unsigned>, vector<Matrix<Color>>> patterns = OverlappingWFC<Color>::get_patterns(m, N_value, periodic_input_value, symmetry_value);
        vector<array<vector<unsigned>, 4>> propagator = OverlappingWFC<Color>::generate_propagator(patterns.second);
        Wave wave = OverlappingWFC<Color>::generate_initial_wave(m, N_value, patterns.second, ground_value, periodic_output_value, width_value, height_value, patterns.first);
        WFC wfc = WFC(periodic_output_value, 6683 + test * screenshots_value + i, patterns.first, propagator, wave);
        bool success = wfc.run();
        if(success) {
          write_file("results/" + name + ".png", OverlappingWFC<Color>::get_output(wfc, patterns.second, N_value, width_value, height_value));
          cout << name << " finished!" << endl;
          break;
        }
      }
    }
  }
  ProfilerStop();
}

int main() {

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  read_config_file("samples2.xml");

  end = std::chrono::system_clock::now();
  int elapsed_s = std::chrono::duration_cast<std::chrono::seconds> (end-start).count();
  int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
  std::cout << "All samples done in " << elapsed_s << "s, " << elapsed_ms % 1000 << "ms.\n";
}
