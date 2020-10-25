#include "DebouncedButton.h"

#include <functional>

#include "Tickable.h"


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
        ctx->release(state_duration);
      }
      if((input == true)) {
        //Serial.println("rising edge!");
        ctx->press();
      }
    }
  }
  last_input = input;
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
