#pragma once

#include <string>
#include "../lib/rapidxml.hpp"


namespace rapidxml {
/**
 * Get an attribute from the xml node.
 * If the attribute does not exist, then return the default value given.
 */
  std::string get_attribute(xml_node<>* node, const std::string& attribute, const std::string& default_value) noexcept {
  if(node->first_attribute(attribute.c_str()) != nullptr) {
    return node->first_attribute(attribute.c_str())->value();
  } else {
    return default_value;
  }
}

/**
 * Get an attribute from the xml node.
 * If the attribute does not exist, this will result in undefined behavior.
 */
  std::string get_attribute(xml_node<>* node, const std::string& attribute) noexcept {
  return node->first_attribute(attribute.c_str())->value();
}

} // namespace rapidxml
