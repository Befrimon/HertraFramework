#include "depth_buffer.hpp"

#include <iostream>
#include <vector>

static VkFormat findSupportedFormat(
  VkPhysicalDevice physicalDevice,
  const std::vector<VkFormat>& candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features
) {
  std::cout << "Finding supported depth format..." << std::endl;

  for (VkFormat format : candidates)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      return format;
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
      return format;
  }
  throw std::runtime_error("Failed to find supported format!");
}

static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
{
  return findSupportedFormat(
    physicalDevice,
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

DepthBuffer::DepthBuffer(VkPhysicalDevice physicalDevice, VkDevice dev, VkExtent2D extent)
  : device(dev), depthFormat(findDepthFormat(physicalDevice))
{
  std::cout << "Creating depth image " << extent.width << "x" << extent.height << std::endl;

  // Create image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = extent.width;
  imageInfo.extent.height = extent.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = depthFormat;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    throw std::runtime_error("Failed to create depth image!");
  std::cout << "Depth image created" << std::endl;

  // Allocate memory
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if (
      (memRequirements.memoryTypeBits & (1 << i)) &&
      ( memProperties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ) {
      allocInfo.memoryTypeIndex = i;
      break;
    }
  }

  if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate depth image memory!");
  std::cout << "Depth memory allocated" << std::endl;

  vkBindImageMemory(device, image, memory, 0);
  std::cout << "Depth memory bound" << std::endl;

  // Create view
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = depthFormat;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    throw std::runtime_error("Failed to create depth image view!");

  std::cout << "Depth buffer created: " << extent.width << "x" << extent.height << std::endl;
}

DepthBuffer::~DepthBuffer()
{
  if (imageView != VK_NULL_HANDLE)
    vkDestroyImageView(device, imageView, nullptr);
  if (image != VK_NULL_HANDLE)
    vkDestroyImage(device, image, nullptr);
  if (memory != VK_NULL_HANDLE)
    vkFreeMemory(device, memory, nullptr);
}
