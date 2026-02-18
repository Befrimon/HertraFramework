#ifndef HERTRA_HPP
#define HERTRA_HPP

#include "window.hpp"
#include "input_device.hpp"
#include "timer.hpp"
#include "vulkan_device.hpp"
#include "swap_chain.hpp"
#include "shader.hpp"
#include "uniform_buffer.hpp"
#include "cube.hpp"
#include "graphics_pipeline.hpp"
#include "descriptor.hpp"
#include "depth_buffer.hpp"

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class HertraApp
{
private:
  std::unique_ptr<HertraWindow> window;
  std::unique_ptr<InputDevice> inputDevice;
  std::unique_ptr<Timer> timer;

  std::unique_ptr<GraphicsPipeline> pipeline;
  std::unique_ptr<Descriptor> descriptor;
  std::unique_ptr<Cube> cube;
  std::unique_ptr<UniformBuffer> uniformBuffer;
  std::unique_ptr<DepthBuffer> depthBuffer;
  std::unique_ptr<Shader> shader;
  std::unique_ptr<SwapChain> swapChain;
  std::unique_ptr<VulkanDevice> device;

  VkInstance instance;
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
  void createFramebuffers();
  void createCommandBuffers();
  void createSyncObjects();
  void createDepthBuffer();
  void cleanup();
  void processInput();
  void drawFrame();
  void updateUniformBuffer(uint32_t currentImage);

public:
  HertraApp();
  ~HertraApp();

  void run();

};

#endif
