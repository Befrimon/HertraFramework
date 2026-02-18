#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include "uniform_buffer.hpp"

class Descriptor
{
private:
  VkDevice device;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  VkPipelineLayout pipelineLayout;

public:
  Descriptor(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t imageCount);
  ~Descriptor();

  void update(uint32_t currentImage, const UniformBuffer& uniformBuffer);
  VkDescriptorSet getDescriptorSet(uint32_t currentImage) const { return descriptorSets[currentImage]; }
  VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
};

#endif
