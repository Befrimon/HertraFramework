#include "window.hpp"

#include <iostream>

HertraWindow::HertraWindow(int width, int height, const std::string& title)
  : windowProc(nullptr)
{
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize GLFW");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create window");
  }

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

  std::cout << "Window created successfully" << std::endl;
}

HertraWindow::~HertraWindow()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool HertraWindow::shouldClose() const
{
  return glfwWindowShouldClose(window);
}

void HertraWindow::pollEvents()
{
  glfwPollEvents();
}

void HertraWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
  auto app = reinterpret_cast<HertraWindow*>(glfwGetWindowUserPointer(window));
  if (app->windowProc)
  {
    app->windowProc(width, height);
  }
}
