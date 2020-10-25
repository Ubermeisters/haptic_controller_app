#include <vector>

#include <Adafruit_DRV2605.h>

#include "Button.h"
#include "Tickable.h"

constexpr unsigned long SHORT_PRESS_MS = 500;
constexpr unsigned long LONG_PRESS_MS = 5000;

constexpr int BAUD_RATE = 115200;

// Pin numbers of components, you may need to adjust
constexpr int PIN_LED_0 = 13;
constexpr int PIN_BUTTON_0 = 0;

std::vector<Tickable*> components;

Button *button;

ButtonContext* menu_ctx;
ButtonContext* led_ctx;
ButtonContext* haptic_ctx;

constexpr int MAX_EFFECT = 123;
Adafruit_DRV2605 drv;
int effect = 0;

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
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

  // Allocate aux button and push into list of components
  button = new Button(PIN_BUTTON_0, false); // Again, active low
  components.push_back(button);

  // Helper functions to initialize various button behaviors
  init_menu_ctx();
  init_led_ctx();
  init_haptic_ctx();

  // Button starts in "menu" mode
  button->set_context(menu_ctx);
  
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

void init_menu_ctx() {
  Serial.println("initializing menu_ctx");
  menu_ctx = new ButtonContext();
  menu_ctx->on_press([&]() {
    Serial.println("pressed");
  });
  menu_ctx->on_release([&](unsigned long duration) {
    print_duration(duration);
    if(duration > LONG_PRESS_MS) {
      Serial.println("Switching to haptic controller");
      menu_ctx = button->release_context();
      button->set_context(haptic_ctx);
    }
  });
}

void init_led_ctx() {
  Serial.println("initializing led_ctx");
  led_ctx = new ButtonContext();
  led_ctx->on_press([&]() {
    Serial.println("pressed");
  });
  led_ctx->on_release([&](unsigned long duration) {
    print_duration(duration);
    
    if(duration < SHORT_PRESS_MS) {
      digitalWrite(PIN_LED_0, HIGH);
    } else if(duration < LONG_PRESS_MS) {
      digitalWrite(PIN_LED_0, LOW);
    } else {
      Serial.println("Switching to menu");
      led_ctx = button->release_context();
      button->set_context(menu_ctx);
    }
  });
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
      ++effect;
    } else if(duration < LONG_PRESS_MS) {
      --effect;
    } else {
      Serial.println("switching to LED controller");
      haptic_ctx = button->release_context();
      button->set_context(led_ctx);
      return;
    }
    if(effect < 0) { effect = 0; }
    if(effect > MAX_EFFECT) { effect = MAX_EFFECT; }
    
    Serial.print("now playing effect: ");
    Serial.println(effect);
    drv.setWaveform(0, effect);
    drv.setWaveform(1, 0);
    drv.go();
  });
}
