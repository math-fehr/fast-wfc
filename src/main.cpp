#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "array2D.hpp"
#include "wfc.hpp"
#include "overlapping_wfc.hpp"
#include "lib/rapidxml.hpp"
#include "color.hpp"
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "lib/stb_image_write.h"

using namespace std;
using namespace rapidxml;

/**
 * Read an image.
 */
Array2D<Color> read_image(const string& file_path) noexcept {
  int width;
  int height;
  int num_components;
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 3);
  assert(data != nullptr);
  Array2D<Color> m = Array2D<Color>(height, width);
  for(unsigned i = 0; i < (unsigned)height; i++) {
    for(unsigned j = 0; j < (unsigned)width; j++) {
      unsigned index = 3 * (i * width + j);
      m.data[i * width + j] = {data[index], data[index + 1], data[index + 2]};
    }
  }
  free(data);
  return m;
}

/**
 * Write an image.
 */
void write_image(const string& file_path, const Array2D<Color>& m) noexcept {
  stbi_write_png(file_path.c_str(), m.width, m.height, 3, (const unsigned char*)m.data.data(),0);
}

/**
 * Get an attribute from the xml node.
 * If the attribute does not exist, then return the default value given.
 */
string get_attribute(xml_node<>* node, const string& attribute, const string& default_value) noexcept {
  if(node->first_attribute(attribute.c_str()) != nullptr) {
    return node->first_attribute(attribute.c_str())->value();
  } else {
    return default_value;
  }
}

/**
 * Read the overlapping wfc problem from the xml node.
 */
void read_overlapping_element(xml_node<>* node) noexcept {
  string name = node->first_attribute("name")->value();
  unsigned N = stoi(node->first_attribute("N")->value());
  bool periodic_output = (get_attribute(node, "periodic", "False") == "True");
  bool periodic_input = (get_attribute(node, "periodicInput", "True") == "True");
  bool ground = (stoi(get_attribute(node, "ground", "0")) != 0);
  unsigned symmetry = stoi(get_attribute(node, "symmetry", "8"));
  unsigned screenshots = stoi(get_attribute(node, "screenshots", "2"));
  unsigned width = stoi(get_attribute(node, "width", "48"));
  unsigned height = stoi(get_attribute(node, "height", "48"));

  cout << name << " started!" << endl;
  Array2D<Color> m = read_image("samples/" + name + ".png");
  OverlappingWFCOptions options = {periodic_input, periodic_output, height, width, symmetry, ground, N};
  for(unsigned i = 0; i < screenshots; i++) {
    for(unsigned test = 0; test < 10; test++) {
      int seed = random_device()();
      OverlappingWFC<Color> wfc(m, options, seed);
      std::optional<Array2D<Color>> success = wfc.run();
      if(success.has_value()) {
        write_image("results/" + name + to_string(i) + ".png", *success);
        cout << name << " finished!" << endl;
        break;
      } else {
        cout << "failed!" << endl;
      }
    }
  }
}


void read_config_file(const string& config_path) noexcept {
  xml_document<> doc;
  xml_node<>* root_node;
  ifstream config_file(config_path);
  vector<char> buffer((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  root_node = doc.first_node("samples");
  for (xml_node<> * node = root_node->first_node("overlapping"); node; node = node->next_sibling("overlapping")) {
    read_overlapping_element(node);
  }
}

int main() {

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  read_config_file("samples.xml");

  end = std::chrono::system_clock::now();
  int elapsed_s = std::chrono::duration_cast<std::chrono::seconds> (end-start).count();
  int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
  std::cout << "All samples done in " << elapsed_s << "s, " << elapsed_ms % 1000 << "ms.\n";
}
