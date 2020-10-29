#include <vector>

#include <Adafruit_DRV2605.h>

#include "Button.h"
#include "Timer.h"
#include "Tickable.h"

typedef unsigned long interval_t;
typedef unsigned long point_t;

constexpr interval_t RECORDING_DURATION_MS = 10000;

constexpr interval_t SHORT_PRESS_MS = 500;
constexpr interval_t LONG_PRESS_MS = 3000;

constexpr int DRV_EFFECT_LIB = 1;

// Pin numbers of components, you may need to adjust
constexpr int PIN_LED_0 = 13;
constexpr bool LED_0_ACTIVE_HIGH = true;
constexpr int PIN_BUTTON_0 = 0;
constexpr bool BUTTON_0_ACTIVE_HIGH = false;

constexpr int BAUD_RATE = 115200;

class State {
public:
  State() :
      _entry_callback([](){})
    , _exit_callback([](){})
  {}
  void on_entry(std::function<void()> fn) {_entry_callback = fn;}
  void on_exit(std::function<void()> fn) {_exit_callback = fn;}

  // TODO fix this, these should not be exposed to anything but the state machine
  std::function<void()> _entry_callback;
  std::function<void()> _exit_callback;
private:
};
class StateMachine {
public:
  StateMachine() : _active_state(nullptr) {}
  void transition(State *next_state) {
    if(_active_state) {
      _active_state->_exit_callback();
    }
    next_state->_entry_callback();
    _active_state = next_state;
  }
private:
  State* _active_state;
};

void print_start_time(point_t start_time) {
  //Serial.print("at ");
  //Serial.print(start_time); 
  //Serial.println("ms");
  Serial.println("press");
}
void print_duration(interval_t duration) {
  Serial.print("release after  ");
  Serial.print(duration); 
  Serial.println("ms");
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println();
  
  // Assign modes for all pins
  pinMode(PIN_LED_0, OUTPUT);
  pinMode(PIN_BUTTON_0, (BUTTON_0_ACTIVE_HIGH)? INPUT : INPUT_PULLUP);

  // Turn off LED_0
  digitalWrite(PIN_LED_0, !LED_0_ACTIVE_HIGH);

  // Allocate event driven components
  std::vector<Tickable*> components;

  Timer timer{};
  components.push_back(&timer);

  Button button{PIN_BUTTON_0, BUTTON_0_ACTIVE_HIGH};
  components.push_back(&button);

  // Initialize DRV
  Adafruit_DRV2605 drv;
  drv.begin();
  drv.selectLibrary(DRV_EFFECT_LIB);
  std::vector<int> effects = {1, 10, 100}; // !!! add/remove/modify effects here
  std::size_t effect_idx = effects.size() - 1;
  std::vector<std::pair<point_t, interval_t>> pulses;
  std::function<void()> repeat_preset = [&]() {
    timer.after(1000, repeat_preset);
    drv.setWaveform(0, effects[effect_idx]);
    drv.setWaveform(1, 0);
    drv.go();

  };

  // Define our states
  StateMachine machine;
  State presets_state;
  State record_state;
  State playback_state;

  presets_state.on_entry([&]() {
    Serial.println("entering presets state");
    //Serial.println("1");
    drv.setMode(DRV2605_MODE_INTTRIG);
    button.on_press([&](point_t start_time) {
      print_start_time(start_time);
    });
    button.on_release([&](interval_t duration) {
      print_duration(duration);
      if(duration < SHORT_PRESS_MS) {
        ++effect_idx;
        effect_idx %= effects.size();
      } else {
        machine.transition(&record_state);
      }
    });
    if(!effects.empty()) {
      timer.after(0, repeat_preset);
    }
  });

  presets_state.on_exit([&]() {
    Serial.println("exiting presets state");
    // Force down the button to prevent getting unexpected releases
    // Stop the driver and clear the timer to end playback
    button.reset();
    drv.stop();
    timer.clear();
  });

  point_t recording_start_time;
  record_state.on_entry([&]() {
    Serial.println("entering record state");
    pulses.clear();
    recording_start_time = millis();
    button.on_press([&](point_t event_time) {
      print_start_time(event_time);
      pulses.push_back(std::make_pair(event_time - recording_start_time, 0));
    });
    button.on_release([&](interval_t duration) {
      print_duration(duration);
      pulses.back().second = duration;
    });
    timer.after(
      RECORDING_DURATION_MS,
      [&](){
        machine.transition(&playback_state);
    });
  });
  record_state.on_exit([&]() {
    Serial.println("exiting record state");
    button.reset();
  });


  playback_state.on_entry([&]() {
    Serial.println("entering playback state");
    drv.setMode(DRV2605_MODE_REALTIME);
    for(const auto& pulse : pulses) {
      point_t t = pulse.first;
      interval_t dur = pulse.second;
      timer.after(
        t,
        [&]() {
          digitalWrite(PIN_LED_0, LED_0_ACTIVE_HIGH);
          drv.setRealtimeValue(100);
      });
      timer.after(
        t + dur,
        [&]() {
          digitalWrite(PIN_LED_0, !LED_0_ACTIVE_HIGH);
          drv.setRealtimeValue(0);
      });
    }
    timer.after(
      RECORDING_DURATION_MS,
      [&]() {
        machine.transition(&presets_state);
    });
  });

  playback_state.on_exit([&]() {
    Serial.println("exiting playback state");
    // Force down the button to prevent getting unexpected releases
    // Stop the driver and clear the timer to end playback
    drv.setRealtimeValue(0);
    button.reset();
  });

  Serial.println("exiting setup");
  machine.transition(&presets_state);
  while(true) {
    // Tick all components
    for(auto &component : components) {
      component->tick();
    }
  }
}

void loop() {
  // This never executes, we loop in setup
}
