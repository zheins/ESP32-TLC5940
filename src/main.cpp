#include <Arduino.h>

#include "Tlc5940.h"

void setup() {
  TLC5940_init();
  TLC5940_setPWM(5, 2000);
}

void loop() {

}
