#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <queue>

#include "Tickable.h"

class TaskCompare {
public:
  bool operator()(const std::pair<unsigned long, std::function<void()>> &lhs,
           const std::pair<unsigned long, std::function<void()>> &rhs) {
    return lhs.first > rhs.first;
  }
};

class Timer : public Tickable {
public:
  Timer();
  void tick() override;

  void after(unsigned long countdown, std::function<void()> fn);
  void clear();
private:
  std::priority_queue<std::pair<unsigned long, std::function<void()>>,
                      std::vector<std::pair<unsigned long, std::function<void()>>>,
                      TaskCompare>
                        tasks;
};

#endif
