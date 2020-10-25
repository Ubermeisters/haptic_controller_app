#ifndef TICKABLE_H
#define TICKABLE_H

class Tickable {
public:
  Tickable() {}
  virtual ~Tickable() {}
  virtual void tick() = 0;
};

#endif
