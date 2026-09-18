#include <ESP8266WiFi.h> // Library koneksi WiFi
#include <PubSubClient.h> // Library komunikasi MQTT
#include <ArduinoJson.h> // Library format JSON


const char* ssid = "personalX"; // Nama WiFi
const char* password = "177013003"; // Password WiFi


const char* mqttServer = "broker.hivemq.com"; // Alamat server broker MQTT
const int mqttPort = 1883; // Port standar MQTT
const char* mqttTopic = "ahlele/ahlelas"; // Jalur tujuan pengiriman data


WiFiClient espClient; // Buat kendaraan WiFi biasa
PubSubClient client(espClient); // Masukkan WiFi ke sistem MQTT


void hubungkanWiFi() {
  WiFi.begin(ssid, password); // Mulai sambung WiFi
  Serial.print("Menghubungkan ke WiFi"); // Cetak label
  while (WiFi.status() != WL_CONNECTED) { // Ulangi kalau belum nyambung
    delay(500); // Jeda 0,5 detik
    Serial.print("."); // Cetak titik loading
  }
  Serial.println("\nWiFi berhasil terhubung!"); // Cetak sukses
}


void hubungkanMQTT() {
  while (!client.connected()) { // Ulangi selama MQTT terputus
    Serial.print("Menghubungkan ke broker MQTT..."); // Cetak label
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX); // Buat ID unik acak


    if (client.connect(clientId.c_str())) { // Coba login ke broker pakai ID tadi
      Serial.println("berhasil terhubung!"); // Jika sukses
    } else {
      Serial.print("gagal, rc="); // Jika gagal
      Serial.print(client.state()); // Cetak kode error dari MQTT
      Serial.println(" coba lagi dalam 2 detik"); // Cetak info jeda
      delay(2000); // Tunggu 2 detik sebelum coba lagi
    }
  }
}


void setup() {
  Serial.begin(115200); // Mulai komunikasi serial
  hubungkanWiFi(); // Panggil fungsi sambung WiFi
  client.setServer(mqttServer, mqttPort); // Kunci alamat dan port broker MQTT
}


void loop() {
  if (!client.connected()) { // Jika koneksi MQTT putus di tengah jalan
    hubungkanMQTT(); // Panggil fungsi sambung ulang
  }
  client.loop(); // Jaga detak jantung alat agar tetap "Online" di server


  // Membuat data sensor dalam format JSON
  JsonDocument doc; // Siapkan wadah JSON
  doc["suhu"] = 28.5; // Isi nilai suhu
  doc["kelembaban"] = 65.0; // Isi nilai kelembaban


  char buffer[128]; // Siapkan memori penyimpan teks
  serializeJson(doc, buffer); // Ubah bungkus JSON jadi teks dan simpan ke buffer


  // Mempublikasikan data ke topic MQTT
  client.publish(mqttTopic, buffer); // Kirim (publish) data dari buffer ke topik
  Serial.print("Data terkirim ke topic "); // Cetak label
  Serial.print(mqttTopic); // Cetak nama topik
  Serial.print(": "); // Cetak pemisah
  Serial.println(buffer); // Cetak isi data JSON-nya


  delay(10000); // Kirim data setiap 10 detik
}
