#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "personalX";
const char* password = "177013003";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topicPerintah = "ahlele/ahlelas";
const int ledPin = D4;

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi callback dipanggil otomatis setiap ada pesan baru masuk
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(pesan);

  // Deserialisasi data JSON yang diterima
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, pesan);
  if (error) {
    Serial.print("Gagal parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* perintah = doc["perintah"];
  int intensitas = doc["intensitas"] | 255;   // Ambil nilai intensitas dari JSON, jika tidak ada pakai 255 (terang penuh)
  intensitas = constrain(intensitas, 0, 255); // Batasi nilai intensitas agar tetap di rentang 0-255

  if (String(perintah) == "ON") {
    analogWrite(ledPin, intensitas);            // Nyalakan LED dengan kecerahan sesuai intensitas (PWM)
    Serial.print("Aktuator: ON, intensitas: "); // Cetak status ON ke Serial Monitor
    Serial.println(intensitas);                 // Cetak nilai intensitas yang dipakai
  } else if (String(perintah) == "OFF") {
    analogWrite(ledPin, 0);                     // Matikan LED dengan PWM bernilai 0
    Serial.println("Aktuator: OFF");
  }
}

void hubungkanWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi berhasil terhubung!");
}

void hubungkanMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("berhasil terhubung!");
      client.subscribe(topicPerintah); // subscribe setelah berhasil terhubung
      Serial.print("Subscribe ke topic: ");
      Serial.println(topicPerintah);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 2 detik");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  analogWriteRange(255);   // Atur rentang PWM ESP8266 menjadi 0-255 (bawaan 0-1023)
  analogWrite(ledPin, 0);  // Pastikan LED mati di awal dengan PWM bernilai 0
  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback); // daftarkan fungsi callback
}

void loop() {
  if (!client.connected()) {
    hubungkanMQTT();
  }
  client.loop(); // wajib dipanggil terus-menerus agar pesan dapat diterima
}