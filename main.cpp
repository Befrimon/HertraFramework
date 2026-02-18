#include "hertra.hpp"
#include <iostream>
#include <cstdlib>

int main() {
  const char* sessionType = std::getenv("XDG_SESSION_TYPE");
  if (sessionType)
    std::cout << "Session type: " << sessionType << std::endl;
  else
    std::cout << "XDG_SESSION_TYPE not set" << std::endl;

  try
  {
    HertraApp app;
    app.run();
  } catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
