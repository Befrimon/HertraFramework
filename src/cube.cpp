#include "cube.hpp"

#include <cstring>

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, pos);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(Vertex, normal);

  return attributeDescriptions;
}

Cube::Cube(VkPhysicalDevice physDev, VkDevice dev, VkCommandPool commandPool, VkQueue queue)
  : physicalDevice(physDev), device(dev)
{
  vertices =
  {
    // Front face (Z = +0.5) - normal = (0, 0, 1)
    {{-0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f,  1.0f}},  // 0
    {{ 0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f,  1.0f}},  // 1
    {{ 0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f,  1.0f}},  // 2
    {{-0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f,  1.0f}},  // 3

    // Back face (Z = -0.5) - normal = (0, 0, -1)
    {{ 0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},  // 4
    {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},  // 5
    {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},  // 6
    {{ 0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},  // 7

    // Top face (Y = +0.5) - normal = (0, 1, 0)
    {{-0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f,  1.0f, 0.0f}},  // 8
    {{ 0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f,  1.0f, 0.0f}},  // 9
    {{ 0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f,  1.0f, 0.0f}},  // 10
    {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f,  1.0f, 0.0f}},  // 11

    // Bottom face (Y = -0.5) - normal = (0, -1, 0)
    {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  // 12
    {{ 0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  // 13
    {{ 0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  // 14
    {{-0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  // 15

    // Right face (X = +0.5) - normal = (1, 0, 0)
    {{ 0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}},  // 16
    {{ 0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}},  // 17
    {{ 0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}},  // 18
    {{ 0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}},  // 19

    // Left face (X = -0.5) - normal = (-1, 0, 0)
    {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},  // 20
    {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},  // 21
    {{-0.5f,  0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},  // 22
    {{-0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}}   // 23
  };

  indices =
  {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    17, 16, 18, 18, 16, 19,
    21, 20, 22, 22, 20, 23,
  };

  createVertexBuffer(commandPool, queue);
  createIndexBuffer(commandPool, queue);
}

Cube::~Cube()
{
  if (indexBuffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    vkDestroyBuffer(device, indexBuffer, nullptr);
  if (indexBufferMemory != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    vkFreeMemory(device, indexBufferMemory, nullptr);
  if (vertexBuffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    vkDestroyBuffer(device, vertexBuffer, nullptr);
  if (vertexBufferMemory != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

VkBuffer Cube::createBuffer(
  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& memory
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

  if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate buffer memory!");

  vkBindBufferMemory(device, buffer, memory, 0);
  return buffer;
}

void Cube::copyBuffer(
  VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue
) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Cube::createVertexBuffer(VkCommandPool commandPool, VkQueue queue)
{
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  stagingBuffer = createBuffer(
    bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingBufferMemory
  );

  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  vertexBuffer = createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferMemory
  );

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize, commandPool, queue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Cube::createIndexBuffer(VkCommandPool commandPool, VkQueue queue)
{
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  stagingBuffer = createBuffer(
    bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingBufferMemory
  );

  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  indexBuffer = createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferMemory
  );

  copyBuffer(stagingBuffer, indexBuffer, bufferSize, commandPool, queue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}
