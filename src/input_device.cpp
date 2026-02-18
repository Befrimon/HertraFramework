#include "input_device.hpp"

InputDevice::InputDevice(GLFWwindow* window) : window(window) {}

bool InputDevice::isKeyPressed(int key) const
{
  return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputDevice::isMouseButtonPressed(int button) const
{
  return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void InputDevice::getMousePosition(double& x, double& y) const
{
  glfwGetCursorPos(window, &x, &y);
}
