#include <WiFi.h>                 
#include <HTTPClient.h>           
#include <WiFiClientSecure.h>     
#include <ArduinoJson.h>          

const char* ssid = "namaWifi";                  
const char* password = "Password";              
const char* serverName = "https://httpbin.org/post"; 

void setup() {
  Serial.begin(115200);            
  WiFi.begin(ssid, password);      

  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("\nWiFi berhasil terhubung!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["suhu"] = 28.5;
    doc["kelembaban"] = 65.0;
    doc["iniWaktu"] = millis();       //Menambahkan waktu sejak ESP32 menyala (ms)

    String requestBody;
    serializeJson(doc, requestBody);

    Serial.print("\nMengirim data: ");
    Serial.println(requestBody);

    int httpResponseCode = http.POST(requestBody);

    Serial.print("Kode Response HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Isi Response:");
      Serial.println(response);
    } else {
      Serial.print("Error saat mengirim POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  }

  delay(10000);
}