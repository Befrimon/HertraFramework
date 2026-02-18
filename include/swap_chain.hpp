#ifndef SWAP_CHAIN_HPP
#define SWAP_CHAIN_HPP

#include <vector>
#include "vulkan_device.hpp"

class SwapChain
{
private:
  VulkanDevice& device;
  VkSurfaceKHR surface;
  GLFWwindow* window;

  VkSwapchainKHR swapChain;
  VkFormat imageFormat;
  VkExtent2D extent;
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  // std::vector<VkFramebuffer> framebuffers;

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

  SwapChainSupportDetails querySwapChainSupport();
  void createSwapChain();
  void createImageViews();
  void createFramebuffers(VkRenderPass renderPass);

public:
  SwapChain(VulkanDevice& device, VkSurfaceKHR surface, GLFWwindow* window);
  ~SwapChain();

  void init();
  void cleanup();
  void recreate();

  VkSwapchainKHR getSwapChain() const { return swapChain; }
  VkFormat getImageFormat() const { return imageFormat; }
  VkExtent2D getExtent() const { return extent; }
  const std::vector<VkImage>& getImages() const { return images; }
  const std::vector<VkImageView>& getImageViews() const { return imageViews; }
};

#endif
