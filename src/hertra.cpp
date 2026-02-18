#include "hertra.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

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
  std::cout << "=== initVulkan started ===" << std::endl;

  std::cout << "[1/10] Creating instance..." << std::endl;
  createInstance();
  std::cout << "Instance created" << std::endl;

  std::cout << "[2/10] Creating surface..." << std::endl;
  createSurface();
  std::cout << "Surface created" << std::endl;

  std::cout << "[3/10] Creating device..." << std::endl;
  device = std::make_unique<VulkanDevice>();
  device->init(instance, surface);
  std::cout << "Device created" << std::endl;

  std::cout << "[4/10] Creating swapchain..." << std::endl;
  swapChain = std::make_unique<SwapChain>(*device, surface, window->getWindow());
  swapChain->init();
  std::cout << "Swapchain created" << std::endl;

  std::cout << "[5/10] Creating depth buffer..." << std::endl;
  createDepthBuffer();
  std::cout << "Depth buffer created" << std::endl;

  std::cout << "[6/10] Creating render pass..." << std::endl;
  createRenderPass();
  std::cout << "Render pass created" << std::endl;

  std::cout << "[7/10] Creating framebuffers..." << std::endl;
  createFramebuffers();
  std::cout << "Framebuffers created" << std::endl;

  std::cout << "[8/10] Creating command pool..." << std::endl;
  createCommandPool();
  std::cout << "Command pool created" << std::endl;

  std::cout << "[9/10] Creating shader..." << std::endl;
  shader = std::make_unique<Shader>(device->getDevice(), "shaders/vert.spv", "shaders/frag.spv");
  std::cout << "Shader created" << std::endl;

  std::cout << "[10/10] Creating cube..." << std::endl;
  cube = std::make_unique<Cube>(
    device->getPhysicalDevice(), device->getDevice(), commandPool, device->getGraphicsQueue()
  );
  std::cout << "Cube created" << std::endl;

  uniformBuffer = std::make_unique<UniformBuffer>(
    device->getPhysicalDevice(), device->getDevice(), swapChain->getImages().size()
  );
  std::cout << "Uniform buffer created" << std::endl;

  descriptor = std::make_unique<Descriptor>(
    device->getPhysicalDevice(), device->getDevice(), swapChain->getImages().size()
  );
  std::cout << "Descriptor created" << std::endl;

  pipeline = std::make_unique<GraphicsPipeline>(
    device->getDevice(), swapChain->getExtent(), renderPass, *shader, descriptor->getPipelineLayout()
  );
  std::cout << "Pipeline created" << std::endl;

  createCommandBuffers();
  std::cout << "Command buffers created" << std::endl;

  createSyncObjects();
  std::cout << "Sync objects created" << std::endl;

  std::cout << "=== initVulkan completed ===" << std::endl;
}

void HertraApp::updateUniformBuffer(uint32_t currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  ubo.proj = glm::perspective(
    glm::radians(45.0f), swapChain->getExtent().width / (float)swapChain->getExtent().height, 0.1f, 10.0f
  );
  ubo.proj[1][1] *= -1; // Flip Y for Vulkan

  ubo.lightPos = glm::vec3(2.0f, 2.0f, 2.0f);
  ubo.viewPos = glm::vec3(2.0f, 2.0f, 2.0f);
  ubo.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

  uniformBuffer->update(currentImage, ubo);
  descriptor->update(currentImage, *uniformBuffer);
}

void HertraApp::createDepthBuffer()
{
  depthBuffer = std::make_unique<DepthBuffer>(device->getPhysicalDevice(),device->getDevice(), swapChain->getExtent());
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
  // Color attachment
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChain->getImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Depth attachment
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = depthBuffer->getFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
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
    std::array<VkImageView, 2> attachments =
    {
      swapChain->getImageViews()[i],
      depthBuffer->getImageView()
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
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

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.05f, 0.05f, 0.05f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getExtent().width);
    viewport.height = static_cast<float>(swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->getExtent();
    vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {cube->getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffers[i], cube->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

    VkDescriptorSet descriptorSet = descriptor->getDescriptorSet(i);
    vkCmdBindDescriptorSets(
      commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
      descriptor->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr
    );

    vkCmdDrawIndexed(commandBuffers[i], cube->getIndexCount(), 1, 0, 0, 0);
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
  std::cout << "=== Starting cleanup ===" << std::endl;

  if (device && device->getDevice() != VK_NULL_HANDLE)
  {
    std::cout << "Waiting for device idle..." << std::endl;
    vkDeviceWaitIdle(device->getDevice());
  }

  // 1. Pipeline (использует shader + pipeline layout)
  std::cout << "[1/13] Destroying pipeline..." << std::endl;
  pipeline.reset();

  // 2. Shader (нужен device)
  std::cout << "[2/13] Destroying shader..." << std::endl;
  shader.reset();

  // 3. Descriptor (содержит pipeline layout, нужен device)
  std::cout << "[3/13] Destroying descriptor..." << std::endl;
  descriptor.reset();

  // 4. Cube (vertex/index buffers, нужен device)
  std::cout << "[4/13] Destroying cube..." << std::endl;
  cube.reset();

  // 5. Uniform buffer (нужен device)
  std::cout << "[5/13] Destroying uniform buffer..." << std::endl;
  uniformBuffer.reset();

  std::cout << "[6/13] Destroying uniform buffer..." << std::endl;
  depthBuffer.reset();

  // 6. SwapChain (нужен device)
  std::cout << "[7/13] Destroying swapchain..." << std::endl;
  swapChain.reset();

  // 7. Command buffers
  std::cout << "[8/13] Clearing command buffers..." << std::endl;
  commandBuffers.clear();

  // 8. Sync objects
  std::cout << "[9/13] Destroying sync objects..." << std::endl;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (device && device->getDevice() != VK_NULL_HANDLE)
    {
      if (renderFinishedSemaphores[i] != VK_NULL_HANDLE)
      {
        vkDestroySemaphore(device->getDevice(), renderFinishedSemaphores[i], nullptr);
        renderFinishedSemaphores[i] = VK_NULL_HANDLE;
      }
      if (imageAvailableSemaphores[i] != VK_NULL_HANDLE)
      {
        vkDestroySemaphore(device->getDevice(), imageAvailableSemaphores[i], nullptr);
        imageAvailableSemaphores[i] = VK_NULL_HANDLE;
      }
      if (inFlightFences[i] != VK_NULL_HANDLE)
      {
        vkDestroyFence(device->getDevice(), inFlightFences[i], nullptr);
        inFlightFences[i] = VK_NULL_HANDLE;
      }
    }
  }

  // 9. Command pool
  std::cout << "[10/13] Destroying command pool..." << std::endl;
  if (device && device->getDevice() != VK_NULL_HANDLE && commandPool != VK_NULL_HANDLE)
  {
    vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
  }

  // 10. Framebuffers
  std::cout << "[11/13] Destroying framebuffers..." << std::endl;
  if (device && device->getDevice() != VK_NULL_HANDLE)
  {
    for (auto& framebuffer : swapChainFramebuffers)
      if (framebuffer != VK_NULL_HANDLE)
      {
        vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
      }
    swapChainFramebuffers.clear();
  }

  // 11. Render pass
  std::cout << "[12/13] Destroying render pass..." << std::endl;
  if (device && device->getDevice() != VK_NULL_HANDLE && renderPass != VK_NULL_HANDLE)
  {
    vkDestroyRenderPass(device->getDevice(), renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;
  }

  // 12. Device
  std::cout << "[13/13] Destroying device..." << std::endl;
  device.reset();

  // Surface
  std::cout << "Destroying surface..." << std::endl;
  if (instance != VK_NULL_HANDLE && surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    surface = VK_NULL_HANDLE;
  }

  // Instance
  std::cout << "Destroying instance..." << std::endl;
  if (instance != VK_NULL_HANDLE)
  {
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }

  std::cout << "=== Cleanup completed ===" << std::endl;
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

  updateUniformBuffer(imageIndex);
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
  {
    swapChain->recreate();
    createFramebuffers();
    createCommandBuffers();
  }
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
