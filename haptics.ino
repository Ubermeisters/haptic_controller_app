#include <vector>

#include <Adafruit_DRV2605.h>

#include "Button.h"
//#include "Timer.h"
#include "Tickable.h"

constexpr unsigned long SHORT_PRESS_MS = 500;
constexpr unsigned long LONG_PRESS_MS = 3000;

constexpr int BAUD_RATE = 115200;

// Pin numbers of components, you may need to adjust
constexpr int PIN_LED_0 = 13;
constexpr int PIN_BUTTON_0 = 0;

std::vector<Tickable*> components;

Button *button;
ButtonContext* haptic_ctx;
ButtonContext* record_ctx;
ButtonContext* playback_ctx;

Adafruit_DRV2605 drv;
constexpr std::size_t N_EFFECTS = 3; // Keep this updated if you add or remove an effect
constexpr int DRV_EFFECT_LIB = 1;    // Described in datasheet, appears to changes fade in/out
std::array<int, N_EFFECTS> effects   // Put the effects you want to cycle through here
    {1, 10, 123};
int effect_idx = N_EFFECTS - 1;

unsigned long recording_start_time;
std::vector<std::pair<unsigned long, unsigned long>> pulses;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println();
  
  // Assign modes for all pins
  pinMode(PIN_LED_0, OUTPUT);
  pinMode(PIN_BUTTON_0, INPUT_PULLUP); // Active low, yours may be different

  // Turn off LED_0
  digitalWrite(PIN_LED_0, LOW);

  // Initialize DRV
  drv.begin();
  drv.selectLibrary(DRV_EFFECT_LIB);
  drv.setMode(DRV2605_MODE_INTTRIG);

  // Allocate button and push into list of components
  button = new Button(PIN_BUTTON_0, false); // Again, active low
  components.push_back(button);

  // Helper function to initialize button behavior
  init_haptic_ctx();
  init_record_ctx();

  // Button starts in "haptic" mode
  button->set_context(haptic_ctx);
  
  Serial.println("exiting setup");
}

void loop() {
  // Tick all components
  for(auto &component : components) {
    component->tick();
  }
}

void print_duration(unsigned long duration) {
  Serial.print("released after ");
  Serial.print(duration); 
  Serial.println("ms");
}

void init_haptic_ctx() {
  Serial.println("initializing haptic_ctx");
  haptic_ctx = new ButtonContext();
  haptic_ctx->on_press([&]() {
    Serial.println("pressed");
  });
  haptic_ctx->on_release([&](unsigned long duration) {
    print_duration(duration);
    
    if(duration < SHORT_PRESS_MS) {
      // On short button press, increment effect
      ++effect_idx;
      effect_idx %= effects.size();
      Serial.print("now playing idx ");
      Serial.print(effect_idx);
      Serial.print(", effect ");
      Serial.println(effects[effect_idx]);
      drv.setWaveform(0, effects[effect_idx]);
      drv.setWaveform(1, 0);
      drv.go();
    } else {
      // On long button press, start recording for 5 seconds, then switch to playback
      haptic_ctx = button->release_context();
      button->set_context(record_ctx);
      pulses.clear();
      recording_start_time = millis();
      //timer->in(
      //  RECORDING_DURATION_MS,
      //  [&](){
      //    record_ctx = button->release_context();
      //    button->set_context(playback_ctx);
      //});
    }
  });
}

void init_record_ctx() {
  Serial.println("initializing record_ctx");
  record_ctx = new ButtonContext();
  record_ctx->on_press([&]() {
    // Create pulse entry with 0 duration
    unsigned long time_from_start = millis() - recording_start_time;
    Serial.print("created entry at ");
    Serial.print(time_from_start);
    Serial.println("ms");
    pulses.push_back(std::make_pair(time_from_start, 0));
  });
  record_ctx->on_release([&](unsigned long duration) {
    // Update duration of most recent pulse
    auto& last_entry = pulses.back();
    last_entry.second = duration;
    Serial.print("duration of entry at ");
    Serial.print(last_entry.first);
    Serial.print("ms set to ");
    Serial.print(duration);
    Serial.println("ms");
  });
}
