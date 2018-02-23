#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "matrix.hpp"
#include "wfc.hpp"
#include "rapidxml.hpp"

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
  operator unsigned() const {
    return r + 256 * g + 256*256*b;
  }
};


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
  for (xml_node<> * node = root_node->first_node("overlapping"); node; node = node->next_sibling("overlapping")) {
    string name = node->first_attribute("name")->value();
    string N = node->first_attribute("N")->value();
    string periodic = get_attribute(node, "periodic", "False");         //TODO add
    string periodicInput = get_attribute(node, "periodic", "");         //TODO ad
    string ground = get_attribute(node, "ground", "");                  //TODO add
    string symmetry = get_attribute(node, "symmetry", "8");             //TODO add
    string screenshots = get_attribute(node, "screenshots", "");        //TODO add
    string width = get_attribute(node, "width", "48");
    string height = get_attribute(node, "width", "48");

    int N_value = stoi(N);
    int width_value = stoi(width);
    int height_value = stoi(height);
    int symmetry_value = stoi(symmetry);

    cout << name << " started!" << endl;
    Matrix<Color> m = read_file("samples/" + name + ".png");
    WFC<Color> wfc = WFC<Color>(m, width_value, height_value, N_value, N_value, symmetry_value);
    bool success = wfc.run();
    if(success) {
      write_file("results/" + name + ".png", wfc.output);
      cout << name << " finished!" << endl;
    }
  }
}

int main() {
  read_config_file("samples.xml");
}
