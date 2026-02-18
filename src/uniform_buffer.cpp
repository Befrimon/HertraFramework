#include "uniform_buffer.hpp"
#include <cstring>

static VkBuffer createBuffer(
  VkPhysicalDevice physicalDevice, VkDevice device,
  VkDeviceSize size, VkBufferUsageFlags usage,
  VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory
) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer buffer;
  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to create buffer!");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = 0;

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    if (
      (memRequirements.memoryTypeBits & (1 << i)) &&
      (memProperties.memoryTypes[i].propertyFlags & properties) == properties
    ) {
        allocInfo.memoryTypeIndex = i;
        break;
    }

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate buffer memory!");

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
  return buffer;
}

UniformBuffer::UniformBuffer(VkPhysicalDevice physicalDevice, VkDevice dev, uint32_t imageCount)
  : device(dev)
{
  uniformBuffers.resize(imageCount);
  uniformBuffersMemory.resize(imageCount);
  uniformBuffersMapped.resize(imageCount);

  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  for (size_t i = 0; i < imageCount; i++)
  {
    uniformBuffers[i] = createBuffer(
      physicalDevice, device, bufferSize,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      uniformBuffersMemory[i]
    );
    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

UniformBuffer::~UniformBuffer()
{
  for (size_t i = 0; i < uniformBuffers.size(); i++)
  {
    vkUnmapMemory(device, uniformBuffersMemory[i]);
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}

void UniformBuffer::update(uint32_t currentImage, const UniformBufferObject& ubo)
{
  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

VkDescriptorBufferInfo UniformBuffer::getDescriptorInfo(uint32_t currentImage) const
{
  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = uniformBuffers[currentImage];
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof(UniformBufferObject);
  return bufferInfo;
}
