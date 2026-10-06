#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char* ssid = "personalX";
const char* password = "177013003";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* topicData = "ahlele/ahlelasData";
const char* topicPerintah = "ahlele/ahlelasPerintah";
const char* topicBuzzer = "ahlele/ahlelasBuzzer"; // Topic MQTT baru untuk menerima perintah buzzer

#define DHTPIN 4
#define DHTTYPE DHT22

const int ledPin = D4;
const int buzzerPin = D5; // Pin D5 dipakai untuk buzzer sebagai aktuator kedua

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000;

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;

  for (unsigned int i = 0; i < length; i++)
    pesan += (char)payload[i];

  JsonDocument doc;

  if (deserializeJson(doc, pesan)) return;

  const char* perintah = doc["perintah"];

  if (strcmp(topic, topicPerintah) == 0) { // Jika pesan masuk dari topic LED (strcmp bernilai 0 bila teks sama)
    digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);

    Serial.print("Perintah diterima -> Aktuator: ");
    Serial.println(perintah);
  }
  else if (strcmp(topic, topicBuzzer) == 0) { // Jika pesan masuk dari topic buzzer, jalankan blok ini
    digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW); // Nyalakan buzzer jika "ON", selain itu matikan

    Serial.print("Perintah diterima -> Buzzer: "); // Cetak penanda bahwa perintah berasal dari topic buzzer
    Serial.println(perintah);                      // Cetak nilai perintah buzzer ke Serial Monitor
  }
}

void hubungkanWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  Serial.println("WiFi berhasil terhubung!");
}

void hubungkanMQTT() {
  while (!client.connected()) {

    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      client.subscribe(topicPerintah);
      client.subscribe(topicBuzzer); // Subscribe ke topic buzzer agar pesan buzzer dapat diterima

      Serial.println("Terhubung dan subscribe topic perintah & buzzer"); // Indikator sukses subscribe kedua topic

    } else {

      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  pinMode(buzzerPin, OUTPUT);   // Mengatur pin buzzer (D5) sebagai output
  digitalWrite(buzzerPin, LOW); // Memastikan buzzer mati saat program pertama kali berjalan

  dht.begin();

  hubungkanWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected())
    hubungkanMQTT();

  client.loop();

  if (millis() - waktuTerakhirPublish > intervalPublish) {

    waktuTerakhirPublish = millis();

    float suhu = dht.readTemperature();
    if (!isnan(suhu)) {

    // int suhu = random(28, 31); // Data dummy ini digunakan saat test praktikum saja karena kendala

    JsonDocument doc;

    doc["suhu"] = suhu;

    char buffer[128];

    serializeJson(doc, buffer);

    client.publish(topicData, buffer);

    Serial.print("Data terkirim: ");
    Serial.println(buffer);

    }
  }
}