#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_PCF8574.h>

const char *ssid = "FLIA_ACUNA";
const char *password = "03251979";

int contadorBloqueos = 0;

int LED1 = 26;
int LED2 = 23;
int LED3 = 4;
int SENSOR = 25;

const int BTN1 = 18;
const int BTN2 = 19;
const int BTN3 = 27;

bool statusE1 = false, statusE2 = false, statusE3 = false;

LiquidCrystal_PCF8574 lcd(0x3F);


unsigned long previousMillis = 0;  // Para calcular el tiempo
const long interval = 300;  // 
bool controlRemoto = true;  

unsigned long previousMillisConsulta = 0;
unsigned long previousMillisEnvio = 0;
const long intervaloConsulta = 3000;  // cada 3 segundos
const long intervaloEnvio = 1000;   

void actualizarLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LD1: ");
  lcd.print(statusE1 ? "ON " : "OFF");
  lcd.setCursor(9, 0);
  lcd.print("LD2: ");
  lcd.print(statusE2 ? "ON " : "OFF");
  lcd.setCursor(0, 1);
  lcd.print("LD3: ");
  lcd.print(statusE3 ? "ON " : "OFF");
  lcd.print("Cnt: ");
  lcd.print(contadorBloqueos);
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.setBacklight(255);

  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  pinMode(SENSOR, INPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
    

  actualizarLCD();
}

bool leerBoton(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);
    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW);
      return true;
    }
  }
  return false;
}

bool detectarBloqueo() {
  if (digitalRead(SENSOR) == HIGH) {
    delay(100);
    if (digitalRead(SENSOR) == HIGH) {
      while (digitalRead(SENSOR) == HIGH);
      return true;
    }
  }
  return false;
}

void sendDato() {
  HTTPClient http;
  http.begin("http://192.168.1.16:8001/api/motion/data/create");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDgwNjA0OTl9.IXCRj9_psWq0Z257HtHYMujXefSa2cEJRK942sspCIg");

  int httpResponseCode = http.POST("{\"valor\":\"1\"}");
  if (httpResponseCode > 0) {
    Serial.println("Respuesta POST: " + http.getString());
  } else {
    Serial.println("Error en POST");
  }
  http.end();
}

void sendDatoBombillos(int led1, int led2, int led3, int bloqueos) {
  HTTPClient http;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["led1"] = led1;
  jsonDoc["led2"] = led2;
  jsonDoc["led3"] = led3;
  jsonDoc["bloqueos"] = bloqueos;

  String jsonString;
  serializeJson(jsonDoc, jsonString);
  http.begin("http://192.168.1.16:8001/api/motion/data/bombillos");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDgwNjA0OTl9.IXCRj9_psWq0Z257HtHYMujXefSa2cEJRK942sspCIg");

  int httpResponseCode = http.POST(jsonString);
  if (httpResponseCode > 0) {
    Serial.println("✅ POST exitoso: " + http.getString());
  } else {
    Serial.println("❌ Error en POST: " + String(httpResponseCode));
  }
  http.end();
}

// 🔁 CONSULTAR ESTADO DESDE FASTAPI (control desde React)
void consultarEstadoDesdeAPI() {
   if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.1.16:8001/api/motion/data/bombillos/status");
    http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDgwNjA0OTl9.IXCRj9_psWq0Z257HtHYMujXefSa2cEJRK942sspCIg");

    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        int estadoLed1 = doc["led1"];
        int estadoLed2 = doc["led2"];
        int estadoLed3 = doc["led3"];

        digitalWrite(LED1, estadoLed1);
        digitalWrite(LED2, estadoLed2);
        digitalWrite(LED3, estadoLed3);

        Serial.print("LED1: "); Serial.println(estadoLed1);
        Serial.print("LED2: "); Serial.println(estadoLed2);
        Serial.print("LED3: "); Serial.println(estadoLed3);


        Serial.println("✅ Estado actualizado desde servidor");
      } else {
            Serial.println("❌ Error al parsear JSON: " + String(error.f_str()));
      }
    } else {
      Serial.print("❌ Error HTTP: ");
      Serial.println(httpCode);
    }

    http.end();
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED) {
    // Solo si el modo es remoto, consulta la API y actualiza los LEDs
    if (controlRemoto && currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      consultarEstadoDesdeAPI();
      detectarBloqueo(); 
      sendDato();                 
      actualizarLCD();
    }
  } else {
    Serial.println("WiFi no conectado");
  }

  // Solo si el modo es manual, usa botones físicos
  if (!controlRemoto) {
    if (leerBoton(BTN1)) {
      statusE1 = !statusE1;
      digitalWrite(LED1, statusE1);
      sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
      actualizarLCD();
    }

    if (leerBoton(BTN2)) {
      statusE2 = !statusE2;
      digitalWrite(LED2, statusE2);
      sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
      actualizarLCD();
    }

    if (leerBoton(BTN3)) {
      statusE3 = !statusE3;
      digitalWrite(LED3, statusE3);
      sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
      actualizarLCD();
    }
  }

  if (detectarBloqueo()) {
    contadorBloqueos++;
    Serial.print("Detección #: ");
    Serial.println(contadorBloqueos);
    sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
    actualizarLCD();
  }
}

