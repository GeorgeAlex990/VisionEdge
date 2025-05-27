#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Ready");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');  // Read command from Processing
    
    // Parse command
    float motor1, motor2, motor3, motor4;
    sscanf(command.c_str(), "%f,%f,%f,%f", &motor1, &motor2, &motor3, &motor4);
    
    Serial.print("M1:"); Serial.print(motor1);
    Serial.print(" M2:"); Serial.print(motor2);
    Serial.print(" M3:"); Serial.print(motor3);
    Serial.print(" M4:"); Serial.println(motor4);
    
    // Send back confirmation to Processing
    Serial.println("OK");
  }
  delay(10);
}
