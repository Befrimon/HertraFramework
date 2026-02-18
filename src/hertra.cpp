#include "hertra.hpp"

#include <iostream>
#include <vector>

HertraApp::HertraApp()
  : surface(VK_NULL_HANDLE), renderPass(VK_NULL_HANDLE), commandPool(VK_NULL_HANDLE), currentFrame(0), running(true)
{
  window = std::make_unique<HertraWindow>(800, 600, "Hertra Framework");
  inputDevice = std::make_unique<InputDevice>(window->getWindow());
  timer = std::make_unique<Timer>();

  window->setWindowProc([this](int width, int height) {
    std::cout << "Window resized: " << width << "x" << height << std::endl;
  });

  initVulkan();
  timer->start();
}

HertraApp::~HertraApp()
{
  cleanup();
}

void HertraApp::initVulkan()
{
  createInstance();
  createSurface();

  device = std::make_unique<VulkanDevice>();
  device->init(instance, surface);

  swapChain = std::make_unique<SwapChain>(*device, surface, window->getWindow());
  swapChain->init();

  createRenderPass();
  createFramebuffers();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();

  std::cout << "Vulkan initialized successfully!" << std::endl;
}

void HertraApp::createInstance()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  if (glfwExtensions == nullptr)
    throw std::runtime_error("Failed to get required GLFW instance extensions");

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
  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  createInfo.enabledLayerCount = 0;

  std::cout << "Creating Vulkan instance with " << glfwExtensionCount << " extensions..." << std::endl;

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    throw std::runtime_error("Failed to create Vulkan instance!");

  std::cout << "Vulkan instance created successfully!" << std::endl;
}

void HertraApp::createSurface()
{
  if (glfwCreateWindowSurface(instance, window->getWindow(), nullptr, &surface) != VK_SUCCESS)
    throw std::runtime_error("Failed to create window surface!");
}

void HertraApp::createRenderPass()
{
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChain->getImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    throw std::runtime_error("Failed to create render pass!");
}

void HertraApp::createCommandPool()
{
  QueueFamilyIndices queueFamilies = device->getQueueFamilies();

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily;

  if (vkCreateCommandPool(device->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    throw std::runtime_error("Failed to create command pool!");
}

void HertraApp::createFramebuffers()
{
  swapChainFramebuffers.resize(swapChain->getImageViews().size());

  for (size_t i = 0; i < swapChain->getImageViews().size(); i++)
  {
    VkImageView attachments[] =
    {
      swapChain->getImageViews()[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChain->getExtent().width;
    framebufferInfo.height = swapChain->getExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to create framebuffer!");
  }

  std::cout << "Created " << swapChainFramebuffers.size() << " framebuffers" << std::endl;
}

void HertraApp::createCommandBuffers()
{
  commandBuffers.resize(swapChainFramebuffers.size());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate command buffers!");

  // Record command buffers
  for (size_t i = 0; i < commandBuffers.size(); i++)
  {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
      throw std::runtime_error("Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    VkClearValue clearColor = {{{0.1f, 0.2f, 0.3f, 1.0f}}}; // Blue-ish color
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer!");
  }
}

void HertraApp::createSyncObjects()
{
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (
      vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
      vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
      vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS
    ) {
      throw std::runtime_error("Failed to create synchronization objects!");
    }
  }
}

void HertraApp::cleanup()
{
  vkDeviceWaitIdle(device->getDevice());

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    vkDestroySemaphore(device->getDevice(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device->getDevice(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device->getDevice(), inFlightFences[i], nullptr);
  }

  for (auto framebuffer : swapChainFramebuffers)
    vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
  swapChainFramebuffers.clear();

  vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
  vkDestroyRenderPass(device->getDevice(), renderPass, nullptr);

  swapChain->cleanup();
  device->cleanup();
  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance, surface, nullptr);
  if (instance != VK_NULL_HANDLE)
    vkDestroyInstance(instance, nullptr);
}

void HertraApp::processInput()
{
  if (inputDevice->isKeyPressed(GLFW_KEY_ESCAPE))
    running = false;
}

void HertraApp::drawFrame()
{
  vkWaitForFences(device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(device->getDevice(), swapChain->getSwapChain(),
    UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    swapChain->recreate();
    createFramebuffers();
    createCommandBuffers();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("Failed to acquire swap chain image!");

  vkResetFences(device->getDevice(), 1, &inFlightFences[currentFrame]);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    throw std::runtime_error("Failed to submit draw command buffer!");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    swapChain->recreate();
  else if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to present swap chain image!");

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HertraApp::run()
{
  std::cout << "Starting main loop..." << std::endl;
  std::cout << "Press ESC to exit" << std::endl;

  while (!window->shouldClose() && running)
  {
    window->pollEvents();
    processInput();
    drawFrame();

    // Print FPS every second
    static double lastTime = 0;
    static int frameCount = 0;
    double currentTime = timer->getElapsedSeconds();
    frameCount++;

    if (currentTime - lastTime >= 1.0)
    {
      std::cout << "FPS: " << frameCount << std::endl;
      frameCount = 0;
      lastTime = currentTime;
    }
  }

  vkDeviceWaitIdle(device->getDevice());
  std::cout << "Main loop ended." << std::endl;
  std::cout << "Total time: " << timer->getElapsedSeconds() << " seconds" << std::endl;
}
