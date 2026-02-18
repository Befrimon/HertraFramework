#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec3 normal;

  static VkVertexInputBindingDescription getBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

#endif
