// Library dan konfigurasi
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// #include <DHT.h>


const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";


const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;


const char* topicData = "ahlele/ahlelas/dhtPalsu";   // harus sama dengan TOPIC di Python
// const char* topicPerintah = "ahlele/ahlelasPerintah";


// #define DHTPIN 4
// #define DHTTYPE DHT22


// const int ledPin = D4;


// DHT dht(DHTPIN, DHTTYPE);


WiFiClient espClient;
PubSubClient client(espClient);


unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000;   // publish tiap 5 detik


// Mock function: simulasi suhu (tren sinus + noise + lonjakan anomali tiap 25 pembacaan)
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;


  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);
  float noise = random(-5, 6) * 0.1;
  float suhu = suhuDasar + noise;


  if (jumlahBaca % 25 == 0) {
    suhu += 8.0;
  }


  return suhu;
}


// Callback penerima perintah (tidak digunakan di Percobaan 5B)
// void callback(char* topic, byte* payload, unsigned int length) {
//   String pesan;
//   for (unsigned int i = 0; i < length; i++)
//     pesan += (char)payload[i];
//
//   JsonDocument doc;
//   if (deserializeJson(doc, pesan)) return;
//
//   const char* perintah = doc["perintah"];
//   digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
//
//   Serial.print("Perintah diterima -> Aktuator: ");
//   Serial.println(perintah);
// }


// Koneksi WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED)
    delay(500);


  Serial.println("WiFi berhasil terhubung!");
}


// Koneksi ke broker MQTT (ulangi sampai berhasil)
void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);


    if (client.connect(clientId.c_str())) {
      // client.subscribe(topicPerintah);
      Serial.println("Terhubung ke broker MQTT");
    } else {
      delay(2000);
    }
  }
}


void setup() {
  Serial.begin(115200);


  // pinMode(ledPin, OUTPUT);
  // dht.begin();


  hubungkanWiFi();


  client.setServer(mqttServer, mqttPort);
  // client.setCallback(callback);
}


void loop() {
  // Jaga koneksi MQTT tetap hidup
  if (!client.connected())
    hubungkanMQTT();


  client.loop();


  // Publish data suhu mock setiap 5 detik (non-blocking)
  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();


    // float suhu = dht.readTemperature();
    // if (!isnan(suhu)) {


    // int suhu = random(28, 31);
    float suhu = bacaSuhuMock();


    // Susun payload JSON: {"suhu":28.53}
    JsonDocument doc;
    doc["suhu"] = suhu;


    char buffer[128];
    serializeJson(doc, buffer);


    client.publish(topicData, buffer);


    Serial.print("Data terkirim: ");
    Serial.println(buffer);


    // }
  }
}
