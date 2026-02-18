#ifndef CUBE_HPP
#define CUBE_HPP

#include "vertex.hpp"
#include <vector>
#include <glm/glm.hpp>

class Cube
{
private:
  VkDevice device;
  VkPhysicalDevice physicalDevice;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  void createVertexBuffer(VkCommandPool commandPool, VkQueue queue);
  void createIndexBuffer(VkCommandPool commandPool, VkQueue queue);
  VkBuffer createBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& memory
  );
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue);

public:
  Cube(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue);
  ~Cube();

  VkBuffer getVertexBuffer() const { return vertexBuffer; }
  VkBuffer getIndexBuffer() const { return indexBuffer; }
  uint32_t getIndexCount() const { return static_cast<uint32_t>(indices.size()); }
};

#endif
