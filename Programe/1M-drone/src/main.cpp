#include <Arduino.h>
#include <Servo.h>

Servo ESC;

void setup() {
  // put your setup code here, to run once:
  ESC.attach(9, 1000, 2000);
}

void loop() {
  // put your main code here, to run repeatedly:

  for (int i = 0; i < 180; i++) {
    ESC.write(i);
    delay(50);
  }
  for (int i = 180; i > 0; i--) {
    ESC.write(i);
    delay(50);
  }
}