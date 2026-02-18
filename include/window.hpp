#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <functional>

class HertraWindow
{
public:
  using WindowProc = std::function<void(int, int)>;

private:
  GLFWwindow* window;
  WindowProc windowProc;

  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

public:
  HertraWindow(int width, int height, const std::string& title);
  ~HertraWindow();

  bool shouldClose() const;
  void pollEvents();
  GLFWwindow* getWindow() { return window; }

  void setWindowProc(WindowProc proc) { windowProc = proc; }
};

#endif
