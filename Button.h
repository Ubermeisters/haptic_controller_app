#ifndef BUTTON_H
#define BUTTON_H

#include <functional>

#include "Tickable.h"

class Button : public Tickable {
public:
  // any press less than DEFAULT_MIN_PRESS_DURATION will be considered a bounce, and ignored, unless alternative specified
  Button(int button_pin, bool is_active_high);
  Button(int button_pin, bool is_active_high, unsigned long min_press_duration);
  virtual ~Button();
  void tick() override;
  
  // Registers a callback to execute on button release
  // On button release, callback with the lowest max_duration higher than the press duration
  void on_press(std::function<void(unsigned long)> fn);
  void on_release(std::function<void(unsigned long)> fn);

  // Force the button into an inactive state, to end the current press if active
  // Also clear callbacks
  void reset();

private:
  std::function<void(unsigned long)> _falling_callback;
  std::function<void(unsigned long)> _rising_callback;

  static constexpr unsigned long DEFAULT_MIN_PRESS_DURATION = 50; // ms
  const unsigned long _min_press_duration;
  unsigned long _last_input_transition_time;
  unsigned long _last_state_transition_time;
  const int _button_pin;
  bool _is_active_high;
  bool _state;
  bool _last_input;
};

#endif
