#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer
{
private:
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point endTime;
  bool running;
public:
  Timer();

  void start();
  void stop();
  double getElapsedSeconds() const;
  double getElapsedMilliseconds() const;
  void reset();
};

#endif
