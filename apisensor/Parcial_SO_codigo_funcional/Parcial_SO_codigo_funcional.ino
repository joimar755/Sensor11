#include <HTTPClient.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Asegúrate de tener la biblioteca ArduinoJson instalada en tu entorno de desarrollo

const char *ssid = "FLIA_ACUNA";
const char *password = "03251979";
//const char *apiUrl = "http://127.0.0.1:3000"; // Reemplaza con la URL de tu API



void setup() {
  Serial.begin(115200);

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());

  
}

void sendDato(){
  HTTPClient http;
  Serial.println("Enviando petición a /sendDato");

  http.begin("http://192.168.1.19:8000/api/motion/data/create"); 
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJleHAiOjE3NDU4Njc2ODN9.wkj8WHZZqrgaOwzHJPQoVUsppQQdCdNDcb1k3RrVOFg");



  int httpResponseCode = http.POST("{\"valor\":\"1\"}");         

  Serial.println("Código HTTP: " + String(httpResponseCode));

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta del servidor:" +payload);
    Serial.println(payload);
  } else {
    Serial.println("Error en la solicitud POST");
  }

  http.end();
  delay(3000);
}

void getDatos(){
  HTTPClient http;  // Declarar una instancia de HTTPClient
  http.begin("http://192.168.3.23:5000/getDatos");
  // Realizar la solicitud GET y verificar el código de respuesta
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta de la API:");
    Serial.println(payload);
  } else {
    Serial.println("Error en la solicitud GET");
    Serial.println("Código de error: " + String(httpCode));
  }

    http.end(); //Free the resources
    delay(3000);
}

void getDato(int id){
  HTTPClient http;  // Declarar una instancia de HTTPClient
  http.begin(String("http://192.168.3.23:5000/getDato/")+id);
  // Realizar la solicitud GET y verificar el código de respuesta
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta de la API:");
    Serial.println(payload);
  } else {
    Serial.println("Error en la solicitud GET");
    Serial.println("Código de error: " + String(httpCode));
  }

    http.end(); //Free the resources
    delay(3000);
}


void updateDato(int id){
 HTTPClient http;  // Declarar una instancia de HTTPClient
  //////////////////////////////////////////////////////////////////////
  http.begin(String("http://192.168.3.23:5000/updateDato/")+id); //Specify the URL
     http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT("{\"valor\":\"1\"}");         
  

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta POST con datos JSON: " + payload);
  } else {
    Serial.println("Error en la solicitud POST con datos JSON");
  }

  http.end(); // Liberar recursos

  delay(3000); // Esperar 5 segundos antes de la siguiente iteración
}

void loop() {
//getDatos();

sendDato();

//getDato(1);

//updateDato(1);
    
}
