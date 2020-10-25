#include "DebouncedButton.h"

#include <functional>

#include "Tickable.h"

class ButtonContext {
public:
  ButtonContext() : rising_callback([=](){}) {}
  std::map<unsigned long, std::function<void()>> falling_callbacks;
  std::function<void()> rising_callback;
};

DebouncedButton::DebouncedButton(int _button_pin, bool _is_active_high) :
    DebouncedButton(_button_pin, _is_active_high, DEFAULT_MIN_PRESS_DURATION)
{
}

// any press less than _min_press_duration will be considered a bounce, and ignored
DebouncedButton::DebouncedButton(int _button_pin, bool _is_active_high, unsigned long _min_press_duration) :
    min_press_duration(_min_press_duration)
  , button_pin(_button_pin)
  , is_active_high(_is_active_high)
  , state(false)
  , last_input(false)
  , ctx(new ButtonContext())
{
}

DebouncedButton::~DebouncedButton() {
  delete ctx;
}

void DebouncedButton::tick() {
  bool raw_input = digitalRead(button_pin);
  bool input = (is_active_high)? raw_input : !raw_input;
  unsigned long current_time = millis();

  if(input != last_input) {
    //Serial.println("\ninput change!");
    last_input_transition_time = current_time;
  }

  unsigned long input_duration = current_time - last_input_transition_time;
  if(input_duration > min_press_duration) {
    if (input != state) {
      //Serial.println("state change!");
      state = input;
      unsigned long state_duration = current_time - last_state_transition_time;
      last_state_transition_time = current_time;
      if((input == false)) {
        //Serial.println("falling edge!");
        execute_next_falling_callback(state_duration);
      }
      if((input == true)) {
        //Serial.println("rising edge!");
        (ctx->rising_callback)();
      }
    }
  }
  last_input = input;
}

void DebouncedButton::on_release(unsigned long max_duration, std::function<void()> fn) {
  
  ctx->falling_callbacks[max_duration] = fn;
}
void DebouncedButton::on_press(std::function<void()> fn) {
  ctx->rising_callback = fn;
}

ButtonContext* DebouncedButton::release_context() {
  ButtonContext* tmp = ctx;
  ctx = new ButtonContext();
  return tmp;
}
void DebouncedButton::refresh_context() {
  delete ctx;
  ctx = new ButtonContext();
}
void DebouncedButton::set_context(ButtonContext *_ctx) {
  delete ctx;
  ctx = _ctx;
}

void DebouncedButton::execute_next_falling_callback(unsigned long press_duration) {
  Serial.print("searching for next falling callback with duration no less than ");
  Serial.print(press_duration);
  Serial.println("ms...");
  auto it = ctx->falling_callbacks.lower_bound(press_duration);
  if(it == ctx->falling_callbacks.end()) {
    Serial.println("no callback found!");
    return;
  }
  Serial.println("executing callback");
  (it->second)();
}
