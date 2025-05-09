#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_PCF8574.h>

const char *ssid = "Joimar";
const char *password = "1002212701";

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

unsigned long ultimoChequeo = 0;

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
  http.begin("http://192.168.1.11:8001/api/motion/data/create");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer TU_TOKEN_AQUI");

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
  http.begin("http://192.168.1.11:8001/api/motion/data/bombillos");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer ");

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
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://192.168.1.11:8001/api/motion/data/estado");  // Ruta GET
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDgwNjA0OTl9.IXCRj9_psWq0Z257HtHYMujXefSa2cEJRK942sspCIg");

  int httpCode = http.GET();
  if (httpCode > 0) {
    String respuesta = http.getString();
    Serial.println("🌐 Estado recibido: " + respuesta);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, respuesta);
    if (!error) {
      statusE1 = doc["led1"];
      statusE2 = doc["led2"];
      statusE3 = doc["led3"];

      digitalWrite(LED1, statusE1 ? HIGH : LOW);
      digitalWrite(LED2, statusE2 ? HIGH : LOW);
      digitalWrite(LED3, statusE3 ? HIGH : LOW);

      actualizarLCD();
    } else {
      Serial.println("❗ Error al parsear JSON");
    }
  } else {
    Serial.println("❌ Error GET: " + http.errorToString(httpCode));
  }

  http.end();
}

void loop() {
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

  if (detectarBloqueo()) {
    contadorBloqueos++;
    Serial.print("Detección #: ");
    Serial.println(contadorBloqueos);
    sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
    actualizarLCD();
  }

  if (digitalRead(SENSOR) == LOW) {
    sendDato();
    actualizarLCD();
  }

  // 🔄 Consultar cada 5 segundos el estado desde la API (React)
  if (millis() - ultimoChequeo > 5000) {
    consultarEstadoDesdeAPI();

  }

  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - ultimoChequeo > 5000) {
      ultimoChequeo = millis();
      sendDato();
      sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
         
    }
  } else {
    Serial.println("WiFi no conectado");
  }  
}
