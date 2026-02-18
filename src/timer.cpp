#include "timer.hpp"

Timer::Timer() : running(false) {}

void Timer::start()
{
  startTime = std::chrono::high_resolution_clock::now();
  running = true;
}

void Timer::stop()
{
  endTime = std::chrono::high_resolution_clock::now();
  running = false;
}

double Timer::getElapsedSeconds() const
{
  auto end = running ? std::chrono::high_resolution_clock::now() : endTime;
  return std::chrono::duration<double>(end - startTime).count();
}

double Timer::getElapsedMilliseconds() const
{
  return getElapsedSeconds() * 1000.0;
}

void Timer::reset()
{
  running = false;
}
