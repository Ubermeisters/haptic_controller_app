#include <Adafruit_DRV2605.h>

#include "DebouncedButton.h"
#include "Tickable.h"

constexpr unsigned long SHORT_PRESS_MS = 500;
constexpr unsigned long LONG_PRESS_MS = 5000;

constexpr int BAUD_RATE = 115200;

constexpr int PIN_LED_0 = 13;
constexpr int PIN_BUTTON_0 = 0;

DebouncedButton *power_button;
DebouncedButton *aux_button;
std::vector<Tickable*> components;

ButtonContext* menu_ctx = nullptr;
ButtonContext* led_ctx = nullptr;

Adafruit_DRV2605* drv;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println();
  
  // Assign modes for all pins
  pinMode(PIN_LED_0, OUTPUT);
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);

  // Turn off LED_0
  digitalWrite(PIN_LED_0, LOW);

  // Initialize DRV
  drv = new Adafruit_DRV2605();

  // Allocate aux button and push into list of components
  aux_button = new DebouncedButton(PIN_BUTTON_0, false);
  components.push_back(aux_button);

  define_menu_ctx();
  define_led_ctx();

  aux_button->set_context(menu_ctx);
  
  Serial.println("exiting setup");
}

void loop() {
  // Tick all components
  for(auto &component : components) {
    component->tick();
  }
}

void define_menu_ctx() {
  Serial.println("defining menu_ctx");
  menu_ctx = new ButtonContext();
  menu_ctx->on_press([&]() {
    Serial.println("pressed");
  });
  menu_ctx->on_release([&](unsigned long duration) {
    Serial.print("released after ");
    Serial.print(duration);
    Serial.println("ms");
    if(duration > LONG_PRESS_MS) {
      menu_ctx = aux_button->release_context();
      aux_button->set_context(led_ctx);
    }
  });
}

void define_led_ctx() {
  Serial.println("defining led_ctx");
  led_ctx = new ButtonContext();
  led_ctx->on_press([&]() {
    Serial.println("pressed");
  });
  led_ctx->on_release([&](unsigned long duration) {
    Serial.print("released after ");
    Serial.print(duration);
    Serial.println("ms");
    
    if(duration < SHORT_PRESS_MS) {
      digitalWrite(PIN_LED_0, HIGH);
    } else if(duration < LONG_PRESS_MS) {
      digitalWrite(PIN_LED_0, LOW);
    } else {
      led_ctx = aux_button->release_context();
      aux_button->set_context(menu_ctx);
    }
  });
}
