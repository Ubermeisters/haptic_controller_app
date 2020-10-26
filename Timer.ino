#include "Timer.h"

Timer::Timer() {
}

void Timer::tick() {
  unsigned long time = millis();
  while((!tasks.empty()) && (tasks.top().first < time)) {
    Serial.print("executing task at ");
    Serial.println(tasks.top().first);
    (tasks.top().second)();
    tasks.pop();
  }
}

void Timer::in(unsigned long countdown, std::function<void()> fn) {
  tasks.push(std::make_pair(millis() + countdown, fn));
}
