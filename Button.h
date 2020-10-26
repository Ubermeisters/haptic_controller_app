#ifndef DEBOUNCED_BUTTON_H
#define DEBOUNCED_BUTTON_H

#include <functional>

#include "Tickable.h"

class ButtonContext {
public:
  ButtonContext() : 
      _falling_callback([](unsigned long){})
    , _rising_callback([](){})
  {}
  
  // Registers a callback to execute on button release
  // On button release, callback with the lowest max_duration higher than the press duration
  void on_release(std::function<void(unsigned long)> fn) { _falling_callback = fn; }
  void on_press(std::function<void()> fn) { _rising_callback = fn; }

  // There should be no reason for the user to call these
  // They probably shouldn't be accessible to the user at all
  void release(unsigned long duration) { _falling_callback(duration); }
  void press() { _rising_callback(); }

private:
  // TODO allow arbitrary arguments? or at least a ptr to the Button?
  std::function<void(unsigned long)> _falling_callback;
  std::function<void()> _rising_callback;
};

class Button : public Tickable {
public:
  // any press less than DEFAULT_MIN_PRESS_DURATION will be considered a bounce, and ignored, unless alternative specified
  Button(int button_pin, bool is_active_high);
  Button(int button_pin, bool is_active_high, unsigned long min_press_duration);
  virtual ~Button();
  void tick() override;

  bool get_state();

  // A context represents callbacks registered with the button
  ButtonContext *release_context();
  void refresh_context();
  void set_context(ButtonContext *_ctx);
private:
  void execute_next_falling_callback(unsigned long press_duration);

  static constexpr unsigned long DEFAULT_MIN_PRESS_DURATION = 50; // ms
  const unsigned long _min_press_duration;
  unsigned long _last_input_transition_time;
  unsigned long _last_state_transition_time;
  const int _button_pin;
  bool _is_active_high;
  bool _state;
  bool _last_input;
  ButtonContext *_ctx;
};

#endif
