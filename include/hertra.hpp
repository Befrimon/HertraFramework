#ifndef HERTRA_HPP
#define HERTRA_HPP

#include "window.hpp"
#include "input_device.hpp"
#include "timer.hpp"
#include "vulkan_device.hpp"
#include "swap_chain.hpp"

#include <memory>

class HertraApp
{
private:
  std::unique_ptr<HertraWindow> window;
  std::unique_ptr<InputDevice> inputDevice;
  std::unique_ptr<Timer> timer;
  std::unique_ptr<VulkanDevice> device;
  std::unique_ptr<SwapChain> swapChain;

  VkInstance instance;  // Добавлено
  VkSurfaceKHR surface;
  VkRenderPass renderPass;
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  uint32_t currentFrame;
  bool running;

  static const int MAX_FRAMES_IN_FLIGHT = 2;

  void initVulkan();
  void createInstance();
  void createSurface();
  void createRenderPass();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  void createFramebuffers();
  void cleanup();
  void processInput();
  void drawFrame();

public:
    HertraApp();
    ~HertraApp();

    void run();

};

#endif
