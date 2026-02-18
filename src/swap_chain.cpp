#include "swap_chain.hpp"

#include <iostream>
#include <algorithm>

SwapChain::SwapChain(VulkanDevice& dev, VkSurfaceKHR surf, GLFWwindow* win)
  : device(dev), surface(surf), window(win), swapChain(VK_NULL_HANDLE), imageFormat(VK_FORMAT_UNDEFINED)
{
  extent = {0, 0};
}

SwapChain::~SwapChain()
{
  cleanup();
}

void SwapChain::init()
{
  createSwapChain();
  createImageViews();
}

void SwapChain::cleanup()
{
  for (auto imageView : imageViews)
    vkDestroyImageView(device.getDevice(), imageView, nullptr);
  imageViews.clear();

  if (swapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device.getDevice(), swapChain, nullptr);
    swapChain = VK_NULL_HANDLE;
  }
}

void SwapChain::recreate()
{
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0)
  {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(device.getDevice());
  cleanup();
  createSwapChain();
  createImageViews();
}

SwapChainSupportDetails SwapChain::querySwapChainSupport()
{
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
  for (const auto& format : formats)
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return format;
  return formats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
  for (const auto& mode : presentModes)
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return mode;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
  if (capabilities.currentExtent.width != UINT32_MAX)
    return capabilities.currentExtent;

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D actualExtent =
  {
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height)
  };

  actualExtent.width = std::clamp(
    actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
  );
  actualExtent.height = std::clamp(
    actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
  );

  return actualExtent;
}

void SwapChain::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    imageCount = swapChainSupport.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = device.getQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    throw std::runtime_error("Failed to create swap chain!");

  this->imageFormat = surfaceFormat.format;
  this->extent = extent;

  vkGetSwapchainImagesKHR(device.getDevice(), swapChain, &imageCount, nullptr);
  images.resize(imageCount);
  vkGetSwapchainImagesKHR(device.getDevice(), swapChain, &imageCount, images.data());
}

void SwapChain::createImageViews()
{
  imageViews.resize(images.size());

  for (size_t i = 0; i < images.size(); i++)
  {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.getDevice(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to create image views!");
  }
}
