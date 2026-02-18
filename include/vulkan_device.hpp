#ifndef VULKAN_DEVICE_HPP
#define VULKAN_DEVICE_HPP

#include <string>
#include <vector>

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
  uint32_t graphicsFamily = UINT32_MAX;
  uint32_t presentFamily = UINT32_MAX;

  bool isComplete() const
  {
    return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
  }
};

class VulkanDevice
{
private:
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  QueueFamilyIndices queueFamilies;

  void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
  void createLogicalDevice(VkInstance instance, VkSurfaceKHR surface);
  bool isDeviceSuitable(VkPhysicalDevice device, VkInstance instance, VkSurfaceKHR surface);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkInstance instance, VkSurfaceKHR surface);

public:
  VulkanDevice();
  ~VulkanDevice();

  void init(VkInstance instance, VkSurfaceKHR surface);
  void cleanup();

  VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
  VkDevice getDevice() const { return device; }
  VkQueue getGraphicsQueue() const { return graphicsQueue; }
  VkQueue getPresentQueue() const { return presentQueue; }
  QueueFamilyIndices getQueueFamilies() const { return queueFamilies; }
};

#endif
