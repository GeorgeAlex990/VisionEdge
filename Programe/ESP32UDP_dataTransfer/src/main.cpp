#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <DHT_U.h>
#include <DHT.h>

// WiFi and TCP settings
const char* ssid = "SAR-24";
const char* password = "-SAR-24-";
const int tcpPort = 4210;
WiFiServer server(tcpPort);
WiFiClient client;

const int arduino_addr = 1;
TaskHandle_t CommandTaskHandle, SensorTaskHandle;

Adafruit_MPU6050 mpu;
int SDA_PIN = 21;
int SCL_PIN = 22;
int DHTPIN = 13;
int PIR1 = 4;
int PIR2 = 2;
int BUZZER_PIN = 18;

int ECHO1 = 34;
int ECHO2 = 25;
int ECHO3 = 26;
int ECHO4 = 27;
int TRIG1 = 23;
int TRIG2 = 35;
int TRIG3 = 36;
int TRIG4 = 39;

DHT dht(DHTPIN, DHT22);
String lastCommand = "";

float Dist_Spate() {
    float durata, distanta;
    digitalWrite(TRIG1, LOW);  
    delayMicroseconds(2);  
    digitalWrite(TRIG1, HIGH);  
    delayMicroseconds(10);  
    digitalWrite(TRIG1, LOW);
    durata = pulseIn(ECHO1, HIGH);
    distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
    if (distanta <= 10) {
      Wire.beginTransmission(arduino_addr);
      Wire.write("P");  /* trimite comanda */
      Wire.endTransmission();    /* opreste transmisia */
      Serial.println("BRAKE");
    }
    return distanta;
}
  
float Dist_Fata() {
    float durata, distanta;
    digitalWrite(TRIG2, LOW);  
    delayMicroseconds(2);  
    digitalWrite(TRIG2, HIGH);  
    delayMicroseconds(10);  
    digitalWrite(TRIG2, LOW);
    durata = pulseIn(ECHO2, HIGH);
    distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
    if (distanta <= 10) {
      Wire.beginTransmission(arduino_addr);
      Wire.write("P");  /* trimite comanda */
      Wire.endTransmission();    /* opreste transmisia */
      Serial.println("BRAKE");
    }
    return distanta;
}
  
float Dist_Stanga() {
    float durata, distanta;
    digitalWrite(TRIG3, LOW);  
    delayMicroseconds(2);  
    digitalWrite(TRIG3, HIGH);  
    delayMicroseconds(10);  
    digitalWrite(TRIG3, LOW);
    durata = pulseIn(ECHO3, HIGH);
    distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
    return distanta;
}
  
float Dist_Dreapta() {
    float durata, distanta;
    digitalWrite(TRIG4, LOW);  
    delayMicroseconds(2);  
    digitalWrite(TRIG4, HIGH);  
    delayMicroseconds(10);  
    digitalWrite(TRIG4, LOW);
    durata = pulseIn(ECHO4, HIGH);
    distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
    return distanta;
}

// Command Processing Function
void ProcessCommand(const String& command) {
    if (command == lastCommand) return; // Avoid duplicate commands
    lastCommand = command;
    Serial.println("Processing Command: " + command);

    if (command == "F") {
        Wire.beginTransmission(arduino_addr);
        Wire.write("F");
        Wire.endTransmission();
        Serial.println("FORWARD");
    } else if (command == "B") {
        Wire.beginTransmission(arduino_addr);
        Wire.write("B");
        Wire.endTransmission();
        Serial.println("BACKWARD");
    } else if (command == "L") {
        Wire.beginTransmission(arduino_addr);
        Wire.write("L");
        Wire.endTransmission();
        Serial.println("LEFT");
    } else if (command == "R") {
        Wire.beginTransmission(arduino_addr);
        Wire.write("R");
        Wire.endTransmission();
        Serial.println("RIGHT");
    } else if (command == "P") {
        Wire.beginTransmission(arduino_addr);
        Wire.write("P");
        Wire.endTransmission();
        Serial.println("BRAKE");
    } else if (command == "HONK_START") {
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("HONK_START");
    } else if (command == "HONK_END") {
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("HONK_END");
    } else {
        Serial.println("Unknown command: " + command);
    }

    // Send ACK back to the sender
    if (client && client.connected()) {
        //client.print("ACK\n");
        Serial.println("ACK sent to client");
    } else {
        Wire.beginTransmission(arduino_addr);
        Wire.write("P");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("BRAKE");
    }
}

// High-priority task for Command Handling
void CommandTask(void* parameter) {
    while (true) {
        if (client && client.connected()) {
            if (client.available()) {
                String command = client.readStringUntil('\n');
                command.trim();
                Serial.println("Received command: " + command);
                ProcessCommand(command);
            }
        } else {
            Serial.println("Client disconnected! Applying brakes...");
            Wire.beginTransmission(arduino_addr);
            Wire.write("P");  // Send brake command
            Wire.endTransmission();
            Serial.println("BRAKE applied!");

            Serial.println("Waiting for client connection...");
            client = server.available();  // Wait for reconnection
        }
        vTaskDelay(1);  // Yield to other tasks
    }
}



// Lower-priority task for Sensor Data
void SensorTask(void* parameter) {
    while (true) {
        bool valPir1 = digitalRead(PIR1);
        bool valPir2 = digitalRead(PIR2);

        float temp = dht.readTemperature();
        float hum = dht.readHumidity();

        float durata;
        float distSpate;
        digitalWrite(TRIG1, LOW);  
        delayMicroseconds(2);  
        digitalWrite(TRIG1, HIGH);  
        delayMicroseconds(10);  
        digitalWrite(TRIG1, LOW);
        durata = pulseIn(ECHO1, HIGH);
        distSpate = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei

        /*float distFata;
        digitalWrite(TRIG2, LOW);  
        delayMicroseconds(2);  
        digitalWrite(TRIG2, HIGH);  
        delayMicroseconds(10);  
        digitalWrite(TRIG2, LOW);
        durata = pulseIn(ECHO2, HIGH);
        distFata = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei

        float distStanga;
        digitalWrite(TRIG3, LOW);  
        delayMicroseconds(2);  
        digitalWrite(TRIG3, HIGH);  
        delayMicroseconds(10);  
        digitalWrite(TRIG3, LOW);
        durata = pulseIn(ECHO3, HIGH);
        distStanga = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei

        float distDreapta;        
        digitalWrite(TRIG4, LOW);  
        delayMicroseconds(2);  
        digitalWrite(TRIG4, HIGH);  
        delayMicroseconds(10);  
        digitalWrite(TRIG4, LOW);
        durata = pulseIn(ECHO4, HIGH);
        distDreapta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei

        /*float distSpate = Dist_Spate(); 
        float distFata = Dist_Fata(); 
        float distStanga = Dist_Stanga(); 
        float distDreapta = Dist_Dreapta();*/

        String message = "PIR1:" + String(valPir1) + ";PIR2:" + String(valPir2) + ";Temp:" + String(temp) + ";Hum:" + String(hum) + ";DistSpate:" + String(distSpate);// + ";DistFata:" + String(distFata) + ";DistStanga:" + String(distStanga) + ";DistDreapta:" + String(distDreapta);
        if (client && client.connected()) {
            client.print(message + "\n");
        }

        vTaskDelay(500 / portTICK_PERIOD_MS); // Send every 500 ms
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PIR1, INPUT);
    pinMode(PIR2, INPUT);

    pinMode(TRIG1, OUTPUT);
    //pinMode(TRIG2, OUTPUT);
    //pinMode(TRIG3, OUTPUT);
    //pinMode(TRIG4, OUTPUT);
    pinMode(ECHO1, INPUT);
    pinMode(ECHO2, INPUT);
    pinMode(ECHO3, INPUT);
    pinMode(ECHO4, INPUT);

    /*
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());
    */

    WiFi.softAP(ssid, password, 13); /*Initializam access pointul pe canalul 13*/

    IPAddress IP(192, 168, 4, 1);
    WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));

    // Start TCP server
    server.begin();

    // Create high-priority task for commands
    xTaskCreatePinnedToCore(
        CommandTask,    // Task function
        "CommandTask",  // Name of the task
        4096,           // Stack size
        NULL,           // Task input parameter
        2,              // Priority (higher than sensor task)
        &CommandTaskHandle, // Task handle
        1               // Run on Core 1
    );

    // Create lower-priority task for sensor data
    xTaskCreatePinnedToCore(
        SensorTask,     // Task function
        "SensorTask",   // Name of the task
        4096,           // Stack size
        NULL,           // Task input parameter
        1,              // Priority (lower than command task)
        &SensorTaskHandle, // Task handle
        0               // Run on Core 0
    );
}

void loop() {
    // Main loop can be used for other low-priority tasks
    vTaskDelay(100);  // Non-blocking delay
}


