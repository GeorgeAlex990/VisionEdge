#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

// setam pinii motoarelor pe arduuino
#define M1A 2
#define PWM1 3 
#define M1B 4
#define M2A 5
#define PWM2 6 
#define M2B 7
#define BUZZER 8
#define im1 A0
#define im2 A1
#define vbat A3

// setam adresa I2C pentru arduino
#define ADDRESS 1

// setam pinii pentru servomotoare
#define SERVO_Y_PIN 9
#define SERVO_Z_PIN 10

Servo servo_y; /*obiectul servo_y reprezinta servomotorul responsabil pentru miscarea pe verticala*/
Servo servo_z; /*obiectul servo_z reprezinta servomotorul responsabil pentru rotire (miscare pe orizontala)*/

// Pozitiile de centru ale servomotoarelor
int poz_y = 70; 
int poz_z = 50;

void bipait(){
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
}

// setam pozitii pentru camera
void CAM_Center() {
  poz_y = 70;
  poz_z = 100;

  servo_y.write(poz_y);
  servo_z.write(poz_z);
}

void CAM_Up() {
  poz_y = 30;

  servo_y.write(poz_y);
}

void CAM_Down() {
  poz_y = 100;

  servo_y.write(poz_y);
}

void CAM_Left() {
  poz_z = 80;

  servo_z.write(poz_z);
}

void CAM_Right() {
  poz_z = 120;

  servo_z.write(poz_z);
}

void Im1() {                
  int val = analogRead(im1);
  if (val < 100) {
    bipait();
  } 
  else if (val > 924) {
    bipait();
  }
}

void Im2() { 
  int val = analogRead(im2);
  if (val < 100) {
    bipait();
  } 
  if (val > 924) {
    bipait();
  }
}

// setam miscarile robotului
void Viteze() {
  analogWrite(PWM1, 170);
  analogWrite(PWM2, 200);
}

void Backward() {
  digitalWrite(M1A, HIGH);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, HIGH);
  digitalWrite(M2B, LOW);
  Viteze();
  bipait();
}

void Forward() {
  digitalWrite(M1A, LOW);
  digitalWrite(M1B, HIGH);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, HIGH);
  Viteze();
  bipait();
}

void Right() {
  digitalWrite(M1A, LOW);
  digitalWrite(M1B, HIGH);
  digitalWrite(M2A, HIGH);
  digitalWrite(M2B, LOW);
  analogWrite(PWM1, 200);
  analogWrite(PWM2, 200);
  bipait();
}

void Left() {
  digitalWrite(M1A, HIGH);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, HIGH);
  analogWrite(PWM1, 200);
  analogWrite(PWM2, 200);
  bipait();
}

void Brake() {
  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);
}

void convertV() {                
  float cV=map(analogRead(vbat),0,1024,0,186)/10.0;
  if(cV<10) bipait();
}

// functii care sunt executate de fiecare data cand se primestc comenzi de la master
void receiveEvent(int howMany) {
  while (Wire.available()) {
    char command[2]; // considerand comenzile nu vor depasi 2 caractere 
    int i = 0;
    while (Wire.available()) {
      char c = Wire.read();
      if (c == '\n') { // sfarsitul comenzii
        command[i] = '\0'; // stringul comenzii este null-terminate
        break;
      }
      command[i++] = c;
    }
    Serial.println(command);
    // executa actiuni in functie de comanda primita
    // CONTROL MOTOARE
    if (strncmp(command, "F",1) == 0) {
      // miscare INAINTE
      Forward();
      bipait();
      Serial.println("Received FORWARD command");
    } else if (strncmp(command, "B",1) == 0) {
      // miscasre INAPOI
      Backward();
      bipait();
      Serial.println("Received BACKWARD command");
    } else if (strncmp(command, "L",1) == 0) {
      // miscare STANGA
      Left();
      bipait();
      Serial.println("Received LEFT command");
    } else if (strncmp(command, "R",1) == 0) {
      // miscare DREAPTA
      Right();
      bipait();
      Serial.println("Received RIGHT command");
    } else if (strncmp(command, "P",1) == 0) {
      // FRANA (REPAUS)
      Brake();
      Serial.println("Received BRAKE command");
    } // CONTROL CAMERA
    else if (strncmp(command, "W",1) == 0) {
      // miscare CAMERA SUS
      CAM_Up();
      Serial.println("Received CAM UP command");
    } else if (strncmp(command, "S",1) == 0) {
      // miscare CAMERA JOS
      CAM_Down();
      Serial.println("Received CAM DOWN command");
    } else if (strncmp(command, "D",1) == 0) {
      // miscare CAMERA DREAPTA
      CAM_Right();
      Serial.println("Received CAM RIGHT command");
    } else if (strncmp(command, "A",1) == 0) {
      // miscare CAMERA STANGA
      CAM_Left();
      Serial.println("Received CAM LEFT command");
    } else if (strncmp(command, "C",1) == 0) {
      // miscare CAMERA CENTRU
      CAM_Center();
      Serial.println("Received CAM CENTER command");
    } else {
      // comanda necunoscuta
      Serial.println("Received unknown command");
    }
  }
}

// functii executate cand se primesc comenzi 
void requestEvent() {
 Wire.write("Master");  /*trimite string la cerere */
}

void setup() {
  Serial.begin(115200);
  // atasare servomotoare la arduino
  servo_y.attach(SERVO_Y_PIN);
  servo_z.attach(SERVO_Z_PIN);

  //Resetare pozitie camera
  CAM_Center();

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);
  pinMode(PWM1, OUTPUT); 
  pinMode(PWM2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  Wire.begin(ADDRESS);               
  Wire.onReceive(receiveEvent); 
  Wire.onRequest(requestEvent); 
  Serial.begin(9600);           
}

void loop() { 
  Im1();
  Im2();
  convertV();
}