#include "hertra.hpp"

#include <iostream>
#include <vector>

HertraApp::HertraApp()
  : instance(VK_NULL_HANDLE), running(true)
{
  window = std::make_unique<HertraWindow>(800, 600, "Hertra Framework");
  inputDevice = std::make_unique<InputDevice>(window->getWindow());

  window->setWindowProc([this](int width, int height) {
    std::cout << "Window resized: " << width << "x" << height << std::endl;
  });

  initVulkan();
}

HertraApp::~HertraApp()
{
  cleanup();
}

void HertraApp::initVulkan()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "HertraApp";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "HertraEngine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (glfwExtensions != nullptr)
  {
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
  } else
    throw std::runtime_error("Failed to get required Vulkan extensions");

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan instance: " + std::to_string(result));

  std::cout << "Vulkan initialized successfully!" << std::endl;
}

void HertraApp::cleanup()
{
  if (instance != VK_NULL_HANDLE)
  {
    vkDestroyInstance(instance, nullptr);
    std::cout << "Vulkan instance destroyed" << std::endl;
  }
}

void HertraApp::processInput()
{
  if (inputDevice->isKeyPressed(GLFW_KEY_ESCAPE))
    running = false;
}

void HertraApp::run()
{
  std::cout << "Starting main loop..." << std::endl;

  while (!window->shouldClose() && running)
  {
    window->pollEvents();
    processInput();
  }

  std::cout << "Main loop ended." << std::endl;
}
