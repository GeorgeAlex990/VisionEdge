#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>

MPU9250_asukiaaa mySensor;
Adafruit_BMP280 bmp;

// Motoare
Servo motorFL, motorFR, motorBL, motorBR;

// Date
float accX, accY, accZ, gyroX, gyroY, gyroZ, altitude;
int speed;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inițializare MPU9250
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  delay(1000);  // important pentru stabilizare

  // Inițializare BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 nu a fost detectat!");
    while (1);
  }

  // Inițializare ESC
  motorFL.attach(3);
  motorFR.attach(5);
  motorBL.attach(6);
  motorBR.attach(9);

  motorFL.writeMicroseconds(1000);
  motorFR.writeMicroseconds(1000);
  motorBL.writeMicroseconds(1000);
  motorBR.writeMicroseconds(1000);
  delay(3000); // timp pentru armare
}

void loop() {
  mySensor.accelUpdate();
  mySensor.gyroUpdate();

  accX = mySensor.accelX();
  accY = mySensor.accelY();
  accZ = mySensor.accelZ();

  gyroX = mySensor.gyroX();
  gyroY = mySensor.gyroY();
  gyroZ = mySensor.gyroZ();

  altitude = bmp.readAltitude(1013.25);

  // Simulare viteză motoare (doar un exemplu simplificat)
  speed = map((int)(altitude * 10), 0, 500, 1100, 1400);
  speed = constrain(speed, 1100, 1400);

  motorFL.writeMicroseconds(speed);
  motorFR.writeMicroseconds(speed);
  motorBL.writeMicroseconds(speed);
  motorBR.writeMicroseconds(speed);

  // Trimitere date spre Processing
  Serial.print("<");
  Serial.print(accX, 2); Serial.print(",");
  Serial.print(accY, 2); Serial.print(",");
  Serial.print(accZ, 2); Serial.print(",");
  Serial.print(gyroX, 2); Serial.print(",");
  Serial.print(gyroY, 2); Serial.print(",");
  Serial.print(gyroZ, 2); Serial.print(",");
  Serial.print(altitude, 2); Serial.print(",");
  Serial.print(speed);
  Serial.println(">");
  
  delay(50);
}
