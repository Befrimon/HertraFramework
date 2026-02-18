#ifndef DEPTH_BUFFER_HPP
#define DEPTH_BUFFER_HPP

class DepthBuffer
{
private:
  VkDevice device;
  VkImage image;
  VkDeviceMemory memory;
  VkImageView imageView;
  VkFormat depthFormat;

public:
  DepthBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkExtent2D extent);
  ~DepthBuffer();

  VkImageView getImageView() const { return imageView; }
  VkImage getImage() const { return image; }
  VkDeviceMemory getMemory() const { return memory; }
  VkFormat getFormat() const { return depthFormat; }
};

#endif
