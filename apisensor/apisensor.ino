#include <Wire.h>
#include <Arduino.h>
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // Asegúrate de tener la biblioteca ArduinoJson instalada en tu entorno de desarrollo

const char *ssid = "FLIA_ACUNA";
const char *password = "03251979";
#include <LiquidCrystal_PCF8574.h>

int contadorBloqueos = 0;

int LED1 = 26;
int LED2 = 23;
int LED3 = 4;
int SENSOR = 25;
// Definir pines de Botones
const int BTN1 = 18;
const int BTN2 = 19;
const int BTN3 = 27;
bool statusE1 = false, statusE2 = false, statusE3 = false;
// Inicializar LCD en la dirección I2C (0x27 o 0x3F)
LiquidCrystal_PCF8574 lcd(0x3F);

// Función para actualizar el LCD
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
  lcd.print("Cnt: "); lcd.print(contadorBloqueos);

}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  // Inicializar LCD con comunicación I2C
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  lcd.begin(16, 2);
  lcd.setBacklight(255);  // Encender luz de fondo

  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());
  // Configurar pines de LEDs como salida
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Configurar pines de botones como entrada con pull-up interno
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  pinMode(SENSOR, INPUT);

  

  // Apagar LEDs al inicio
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);


  actualizarLCD();  // Mostrar estado inicial en LCD
}

// Función para leer botones con antirrebote
bool leerBoton(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);  // Evita lecturas erróneas por rebote
    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW)
        ;  // Espera a que se suelte
      return true;
    }
  }
  return false;
}
bool detectarBloqueo() {
    if (digitalRead(SENSOR) == HIGH) {  // Si el sensor detecta un objeto
        delay(100);  // Pequeño delay para evitar rebotes
        if (digitalRead(SENSOR) == HIGH) {  
            while (digitalRead(SENSOR) == HIGH);  // Espera a que se libere
            return true;
        }
    }
    return false;
}

void sendDato() {
  HTTPClient http;  // Declarar una instancia de HTTPClient
  //////////////////////////////////////////////////////////////////////

  http.begin("http://192.168.1.2:8001/api/motion/data/create");  //Specify the URL
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDY0ODM0NjJ9.AHia56zmcUka7t8v6_W8qmzSrblD7Yt3eVGxgWuL624");

  int httpResponseCode = http.POST("{\"valor\":\"1\"}");
  

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta POST con datos JSON: " + payload);
  } else {
    Serial.println("Error en la solicitud POST con datos JSON");
  }

  http.end();  // Esperar 5 segundos antes de la siguiente iteración
}
void sendDatoBombillos(int led1, int led2, int led3, int bloqueos) {
  HTTPClient http;  // Declarar una instancia de HTTPClient
  //////////////////////////////////////////////////////////////////////
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["led1"] = led1;
  jsonDoc["led2"] = led2;
  jsonDoc["led3"] = led3;
  jsonDoc["bloqueos"] = bloqueos;

  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  http.begin("http://192.168.1.2:8001/api/motion/data/bombillos");  //Specify the URL
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDY0ODM0NjJ9.AHia56zmcUka7t8v6_W8qmzSrblD7Yt3eVGxgWuL624");

  int httpResponseCode = http.POST(jsonString);


  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("✅ POST exitoso: " + payload);
  } else {
    Serial.println("❌ Error en POST");
    Serial.println("Código HTTP: " + String(httpResponseCode));
    Serial.println("Descripción: " + http.errorToString(httpResponseCode));  // ← Este es clave
  }


  http.end();  // Esperar 5 segundos antes de la siguiente iteración
  http.setTimeout(2000);  // 10 segundos
}


void loop() {
  if (leerBoton(BTN1)) {
    statusE1 = !statusE1;
    digitalWrite(LED1, statusE1 == true ? 1 : 0);
    sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);  // <-- aquí
    delay(100);
    actualizarLCD();
  }

  if (leerBoton(BTN2)) {
    statusE2 = !statusE2;
    digitalWrite(LED2, statusE2 == true ? 1 : 0);
    delay(200);
    sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);  // <-- aquí
    actualizarLCD();
  }

  if (leerBoton(BTN3)) {
    statusE3 = !statusE3;
    digitalWrite(LED3, statusE3 == true ? 1 : 0);
    sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);  // <-- aquí
    delay(300);
    actualizarLCD();
  }
  if (detectarBloqueo()) {  
        contadorBloqueos++;  // Incrementa el contador
        Serial.print("Detección #: ");
        Serial.println(contadorBloqueos);
        sendDatoBombillos(statusE1, statusE2, statusE3, contadorBloqueos);
        actualizarLCD();  // Actualizar LCD con nuevo conteo
  }


  if (digitalRead(SENSOR) == LOW) {
    sendDato();
    actualizarLCD();
  }
  
  
}
