#include "vulkan_device.hpp"

#include <iostream>
#include <set>

VulkanDevice::VulkanDevice()
  :physicalDevice(VK_NULL_HANDLE), device(VK_NULL_HANDLE) {}

VulkanDevice::~VulkanDevice()
{
  cleanup();
}

void VulkanDevice::init(VkInstance instance, VkSurfaceKHR surface)
{
  pickPhysicalDevice(instance, surface);
  createLogicalDevice(instance, surface);
}

void VulkanDevice::cleanup()
{
  if (device != VK_NULL_HANDLE)
  {
    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;
  }
}

void VulkanDevice::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0)
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto& dev : devices)
    if (isDeviceSuitable(dev, instance, surface))
    {
      physicalDevice = dev;
      break;
    }

  if (physicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("Failed to find a suitable GPU!");

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
  std::cout << "Using GPU: " << deviceProperties.deviceName << std::endl;
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device, VkInstance instance, VkSurfaceKHR surface)
{
  QueueFamilyIndices indices = findQueueFamilies(device, instance, surface);
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions =
  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  for (const auto& extension : availableExtensions)
    requiredExtensions.erase(extension.extensionName);

  return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device, VkInstance instance, VkSurfaceKHR surface)
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphicsFamily = i;

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport)
      indices.presentFamily = i;

    if (indices.isComplete())
        break;
    i++;
  }
  return indices;
}

void VulkanDevice::createLogicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
  queueFamilies = findQueueFamilies(physicalDevice, instance, surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies =
  {
    queueFamilies.graphicsFamily,
    queueFamilies.presentFamily
  };

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  std::vector<const char*> deviceExtensions =
  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("Failed to create logical device!");

  vkGetDeviceQueue(device, queueFamilies.graphicsFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(device, queueFamilies.presentFamily, 0, &presentQueue);

  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

}
