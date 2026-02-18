#ifndef PRECOMPILE_HPP
#define PRECOMPILE_HPP

#if defined(__linux__)
  #define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

#endif
