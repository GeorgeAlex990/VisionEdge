import processing.serial.*;

Serial myPort;
float accX, accY, accZ, gyroX, gyroY, gyroZ, altitude;
int speedFL, speedFR, speedBL, speedBR;
float angleX = 0, angleY = 0, angleZ = 0;

void setup() {
  size(600, 600, P3D);
  myPort = new Serial(this, Serial.list()[1], 115200); 
  myPort.bufferUntil('\n');
}

void draw() {
  background(0);
  lights();

  pushMatrix();
  translate(width/2, height/2, 0);
  rotateX(radians(angleX));
  rotateY(radians(angleY));
  rotateZ(radians(angleZ));

  fill(100, 100, 255);
  stroke(255);
  box(100, 20, 100); // Reprezentarea dronei

  popMatrix();

  // Desenăm vitezele motoarelor
  drawMotors();
}

void drawMotors() {
  fill(255, 0, 0, 150);
  noStroke();

  // Motoare față-stânga
  ellipse(width/2 - 60, height/2 - 60, speedFL / 10, speedFL / 10);

  // Motoare față-dreapta
  ellipse(width/2 + 60, height/2 - 60, speedFR / 10, speedFR / 10);

  // Motoare spate-stânga
  ellipse(width/2 - 60, height/2 + 60, speedBL / 10, speedBL / 10);

  // Motoare spate-dreapta
  ellipse(width/2 + 60, height/2 + 60, speedBR / 10, speedBR / 10);
}

void serialEvent(Serial port) {
  String data = port.readStringUntil('\n');
  if (data != null) {
    data = data.trim();
    if (data.startsWith("<") && data.endsWith(">")) {
      data = data.substring(1, data.length() - 1);
      String[] values = data.split(",");
      if (values.length == 8) {
        accX = float(values[0]);
        accY = float(values[1]);
        accZ = float(values[2]);
        gyroX = float(values[3]);
        gyroY = float(values[4]);
        gyroZ = float(values[5]);
        altitude = float(values[6]);
        int speed = int(values[7]);

        speedFL = speed;
        speedFR = speed;
        speedBL = speed;
        speedBR = speed;

        angleX += gyroX * 0.05;
        angleY += gyroY * 0.05;
        angleZ += gyroZ * 0.05;
      }
    }
  }
}
