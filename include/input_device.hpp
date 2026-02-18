#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <unordered_map>

class InputDevice
{
private:
  GLFWwindow* window;

public:
  InputDevice(GLFWwindow* window);

  bool isKeyPressed(int key) const;
  bool isMouseButtonPressed(int button) const;
  void getMousePosition(double& x, double& y) const;
};

#endif
