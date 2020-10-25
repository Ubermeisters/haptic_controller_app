#ifndef DEBOUNCED_BUTTON_H
#define DEBOUNCED_BUTTON_H

#include <functional>
#include <map>

#include "Tickable.h"

class ButtonContext;
class DebouncedButton : public Tickable {
public:
  // any press less than DEFAULT_MIN_PRESS_DURATION will be considered a bounce, and ignored, unless alternative specified
  DebouncedButton(int _button_pin, bool _is_active_high);
  DebouncedButton(int _button_pin, bool _is_active_high, unsigned long _min_press_duration);
  virtual ~DebouncedButton();
  void tick() override;

  // Registers a callback to execute on button release
  // On button release, callback with the lowest max_duration higher than the press duration
  void on_release(unsigned long max_duration, std::function<void()> fn);
  void on_press(std::function<void()> fn);

  // A context represents callbacks registered with the button
  ButtonContext *release_context();
  void refresh_context();
  void set_context(ButtonContext *_ctx);
private:
  void execute_next_falling_callback(unsigned long press_duration);

  static constexpr unsigned long DEFAULT_MIN_PRESS_DURATION = 50; // ms
  const unsigned long min_press_duration;
  unsigned long last_input_transition_time;
  unsigned long last_state_transition_time;
  const int button_pin;
  bool is_active_high;
  bool state;
  bool last_input;
  ButtonContext *ctx;
};

#endif
