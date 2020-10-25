#include <Adafruit_DRV2605.h>

#include "DebouncedButton.h"
#include "Tickable.h"

constexpr int BAUD_RATE = 9600;

constexpr int PIN_LED_0 = 13;
constexpr int PIN_BUTTON_0 = 0;
constexpr int PIN_BUTTON_1 = 0;

DebouncedButton *power_button;
DebouncedButton *aux_button;
std::vector<Tickable*> components;

ButtonContext* menu_ctx = nullptr;
ButtonContext* ctx2 = nullptr;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println();
  
  // Assign modes for all pins
  pinMode(PIN_LED_0, OUTPUT);
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);

  // Turn off LED_0
  digitalWrite(PIN_LED_0, LOW);

  // Allocate power button and push into list of components
  power_button = new DebouncedButton(PIN_BUTTON_0, false);
  //components.push_back(power_button);

  // Allocate aux button and push into list of components
  aux_button = new DebouncedButton(PIN_BUTTON_1, false);
  components.push_back(aux_button);


  // Define ctx2
  Serial.println("defining ctx2");
  aux_button->on_release(
    500,
    [=]() {
      Serial.println("short press ctx2");
    });
    
  aux_button->on_release(
    2000,
    [=]() {
      Serial.println("long press ctx2");
  });
  
  aux_button->on_release(
    10000,
    [&]() {
      Serial.println("longer press ctx2");
      ctx2 = aux_button->release_context();
      aux_button->set_context(menu_ctx);   
  });

  // Swap to menu context and define it
  Serial.println("context swap");
  ctx2 = aux_button->release_context();
  
  Serial.println("defining menu_context");
  aux_button->on_release(
    500,
    [=]() {
      Serial.println("short press");
      digitalWrite(PIN_LED_0, HIGH);
    });
    
  aux_button->on_release(
    2000,
    [=]() {
      Serial.println("long press");
      digitalWrite(PIN_LED_0, LOW);
  });
  
  aux_button->on_release(
    10000,
    [&]() {
      Serial.println("longer press");
      menu_ctx = aux_button->release_context();
      aux_button->set_context(ctx2);
  });
  menu_ctx = aux_button->release_context();
  aux_button->set_context(menu_ctx);

  Serial.println("exiting setup");
}

void loop() {
  // Tick all components
  for(auto &component : components) {
    component->tick();
  }
  // Execute loop logic

}
