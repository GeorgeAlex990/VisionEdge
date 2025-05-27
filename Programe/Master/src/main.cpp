#include <Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Wire.h>

const char* ssid = "SAR-24"; // SSID-ul de pe robot 
const char* password = "-SAR-24-"; // Parola cu care accesam reteaua WiFi a robotului

const int arduino_addr = 1;
int output,state;

Adafruit_MPU6050 mpu;

// Definirea pinilor
int SDA_PIN = 21;
int SCL_PIN = 22;
int DHTPIN = 13;
int MQ2_PIN = 33;
int PIR1 = 4;
int PIR2 = 2;
int ECHO1 = 34;
int ECHO2 = 35;
int ECHO3 = 36;
int ECHO4 = 39;
int TRIG1 = 23;
int TRIG2 = 25;
int TRIG3 = 26;
int TRIG4 = 27;
int BUZZER_PIN = 18;

DHT dht(DHTPIN, DHT22);

AsyncWebServer server(80);

// creare interfata web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>SAR-24</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html { font-family: Arial; display: inline-block; text-align: center; }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    body { max-width: 600px; margin:0 auto; padding-bottom: 25px; }
    .button { background-color: #ccc; border: none; color: white; padding: 16px 40px;
      text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }
    .button:hover { background-color: #555; }
    .button:active { background-color: #aaa; } /* Highlight when button is pressed */
  </style>
</head>
<body>
  <h2>SAR-24 Master Control</h2>
  <div id="buttonContainer">
    <button id="1" class="button">FORWARD</button>
    <br>
    <button id="3" class="button">LEFT</button>
    <button id="2" class="button">BACKWARD</button>
    <button id="4" class="button">RIGHT</button>
    <br>
    <br>
    <button id="5" class="button">HONK</button>
    <br>
    <br>
    <button id="6" class="button">CAM UP</button>
    <br>
    <button id="8" class="button">CAM LEFT</button>
    <button id="7" class="button">CAM DOWN</button>
    <button id="9" class="button">CAM RIGHT</button>
    <br>
    <button id="10" class="button">CAM CENTER</button
  </div>

<script>
document.querySelectorAll('.button').forEach(button => {
  button.addEventListener('touchstart', function() {
    pushButton(this, 1);
  });
  button.addEventListener('touchend', function() {
    pushButton(this, 0);
  });
});

function pushButton(button, state) {
  var pin = button.id;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/update?output="+pin+"&state="+state, true);
  xhr.send();
}
</script>
</body> 
</html>
)rawliteral";

String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<div id=\"buttonContainer\">";
    buttons += "<button id=\"1\" class=\"button\">FORWARD</button>";
    buttons += "<br>";
    buttons += "<button id=\"3\" class=\"button\">LEFT</button>";
    buttons += "<button id=\"2\" class=\"button\">BACKWARD</button>";
    buttons += "<button id=\"4\" class=\"button\">RIGHT</button>";
    buttons += "</div>";
    return buttons;
  }
  return String();
}

/*void Alunecare() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.println(" m/s^2");
  if (a.acceleration.x > 1) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("P");  /* trimite comanda */
    //Wire.endTransmission();    /* opreste transmisia */
    //Serial.println("BRAKE");
  //}
  //if (a.acceleration.x < -1) {
  //  Wire.beginTransmission(arduino_addr);
  //  Wire.write("P");  /* trimite comanda */
  //  Wire.endTransmission();    /* opreste transmisia */
  //  Serial.println("BRAKE");
  //}
//}

void Detectare_Persoana() {
  bool valPir1 = digitalRead(PIR1);
  bool valPir2 = digitalRead(PIR2);

  if (valPir1 && valPir2) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("C");  /* trimite comanda */
    Wire.endTransmission();    /* opreste transmisia */
    Serial.println("CAM CENTER");
  }
  else if (valPir1) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("A");  /* trimite comanda */
    Wire.endTransmission();    /* opreste transmisia */
    Serial.println("CAM LEFT");
  }
  else if (valPir2) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("D");  /* trimite comanda */
    Wire.endTransmission();    /* opreste transmisia */
    Serial.println("CAM RIGHT");
  }
}

// functii pentru citirea valorilor returnate de senzori
String Temperatura() {
  return String(dht.readTemperature());
}

String Umiditate() {
  return String(map(dht.readHumidity(), 0, 100.00, 0, 100));
}

String Poluare() {
  return String(map(analogRead(MQ2_PIN), 0, 4095, 0, 100));
}

void Dist_Spate() {
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
}

void Dist_Fata() {
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
}

void Dist_Stanga() {
  float durata, distanta;
  digitalWrite(TRIG3, LOW);  
  delayMicroseconds(2);  
  digitalWrite(TRIG3, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(TRIG3, LOW);
  durata = pulseIn(ECHO3, HIGH);
  distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
  if (distanta <= 10) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("P");  /* trimite comanda */
    Wire.endTransmission();    /* opreste transmisia */
    Serial.println("BRAKE");
  }
}

void Dist_Dreapta() {
  float durata, distanta;
  digitalWrite(TRIG4, LOW);  
  delayMicroseconds(2);  
  digitalWrite(TRIG4, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(TRIG4, LOW);
  durata = pulseIn(ECHO4, HIGH);
  distanta = (durata * .0343) / 2; // calculam distanta pana la obiect folosind formula vitezei
  if (distanta <= 10) {
    Wire.beginTransmission(arduino_addr);
    Wire.write("P");  /* trimite comanda */
    Wire.endTransmission();    /* opreste transmisia */
    Serial.println("BRAKE");
  }
}

void HONK_START() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void HONK_END() {
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA,SCL);

    pinMode(MQ2_PIN, INPUT);
    pinMode(TRIG1, OUTPUT);
    pinMode(TRIG2, OUTPUT);
    pinMode(TRIG3, OUTPUT);
    pinMode(TRIG4, OUTPUT);
    pinMode(ECHO1, INPUT);
    pinMode(ECHO2, INPUT);
    pinMode(ECHO3, INPUT);
    pinMode(ECHO4, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PIR1, INPUT);
    pinMode(PIR2, INPUT);

    WiFi.softAP(ssid, password, 13); /*Initializam access pointul pe canalul 13*/

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    dht.begin();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/poluare", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", Poluare().c_str());
    });
    server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", Temperatura().c_str());
    });
    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String outputMessage;
    if (request->hasParam("output") && request->hasParam("state")) {
      output = request->getParam("output")->value().toInt();
      state = request->getParam("state")->value().toInt();
      if (output == 1 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("F");  /* trimite comanda*/
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("FORWARD");
      }
      if (output == 4 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("R");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("RIGHT");
      }
      if (output == 2 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("B");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("BACKWARD");
      }
      if (output == 3 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("L");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("LEFT");
      }
      if (output == 6 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("W");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("CAM UP");
      }
      if (output == 8 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("D");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("CAM RIGHT");
      }
      if (output == 7 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("S");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("CAM DOWN");
      }
      if (output == 9 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("A");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("CAM LEFT");
      }
      if (output == 10 && state == 1) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("C");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("CAM CENTER");
      }
      if (output == 5 && state == 1) {
        HONK_START();
        Detectare_Persoana();
      }
      if (output == 5 && state == 0) {
        HONK_END();
      }
      if (output == 1 && state == 0 || output == 2 && state == 0 || output == 3 && state == 0 || output == 4 && state == 0 ) {
        Wire.beginTransmission(arduino_addr);
        Wire.write("P");  /* trimite comanda */
        Wire.endTransmission();    /* opreste transmisia */
        Serial.println("BRAKE");
        //Alunecare(); // In cazul in care robotul se afla pe o panta;
        Detectare_Persoana();
      }
      outputMessage = "GPIO " + String(output) + " set to " + String(state);
    } else {
      outputMessage = "Invalid request";
    }
    request->send(200, "text/plain", "OK");
    });

    server.begin();


  /*if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  */

  for (int i = 1; i <= 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void loop() {
    //Dist_Fata();
    //Dist_Stanga();
    //Dist_Spate();
    //Dist_Dreapta();
    delay(10); // adaugam un mic delay pentru un randament mai mare
}