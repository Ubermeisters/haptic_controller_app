#include "Button.h"

#include <functional>

#include "Tickable.h"


Button::Button(int button_pin, bool is_active_high) :
    Button(button_pin, is_active_high, DEFAULT_MIN_PRESS_DURATION)
{
}

// any press less than _min_press_duration will be considered a bounce, and ignored
Button::Button(int button_pin, bool is_active_high, unsigned long min_press_duration) :
    _min_press_duration(min_press_duration)
  , _button_pin(button_pin)
  , _is_active_high(is_active_high)
  , _state(false)
  , _last_input(false)
  , _ctx(new ButtonContext())
{
}

Button::~Button() {
  delete _ctx;
}

void Button::tick() {
  bool raw_input = digitalRead(_button_pin);
  bool input = (_is_active_high)? raw_input : !raw_input;

  unsigned long current_time = millis();

  if(input != _last_input) {
    //Serial.println("\ninput change!");
    _last_input_transition_time = current_time;
  }

  unsigned long input_duration = current_time - _last_input_transition_time;
  if(input_duration > _min_press_duration) {
    if (input != _state) {
      //Serial.println("state change!");
      _state = input;
      unsigned long state_duration = current_time - _last_state_transition_time;
      _last_state_transition_time = current_time;
      if((input == false)) {
        //Serial.println("falling edge!");
        _ctx->release(state_duration);
      }
      if((input == true)) {
        //Serial.println("rising edge!");
        _ctx->press();
      }
    }
  }
  _last_input = input;
}

bool Button::get_state() {
  return _state;
}

ButtonContext* Button::release_context() {
  ButtonContext* tmp = _ctx;
  _ctx = new ButtonContext();
  return tmp;
}
void Button::refresh_context() {
  delete _ctx;
  _ctx = new ButtonContext();
}
void Button::set_context(ButtonContext *ctx) {
  delete _ctx;
  _ctx = ctx;
}
