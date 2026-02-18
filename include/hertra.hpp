#ifndef HERTRA_HPP
#define HERTRA_HPP

#include "window.hpp"
#include "input_device.hpp"

#include <vulkan/vulkan.h>
#include <memory>

class HertraApp
{
private:
  std::unique_ptr<HertraWindow> window;
  std::unique_ptr<InputDevice> inputDevice;
  VkInstance instance;
  bool running;

  void initVulkan();
  void cleanup();
  void processInput();

public:
  HertraApp();
  ~HertraApp();

  void run();
};

#endif
