#ifndef UNIFORM_BUFFER_HPP
#define UNIFORM_BUFFER_HPP

#include <glm/glm.hpp>
#include <vector>

struct UniformBufferObject
{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
  alignas(16) glm::vec3 lightPos;
  alignas(16) glm::vec3 viewPos;
  alignas(16) glm::vec3 lightColor;
};

class UniformBuffer
{
private:
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;
  VkDevice device;

public:
  UniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t imageCount);
  ~UniformBuffer();

  void update(uint32_t currentImage, const UniformBufferObject& ubo);
  VkDescriptorBufferInfo getDescriptorInfo(uint32_t currentImage) const;
};

#endif
