#include <ESP8266WiFi.h>      // Library untuk mengoneksikan ESP8266 ke jaringan WiFi
#include <PubSubClient.h>     // Library untuk protokol komunikasi MQTT
#include <ArduinoJson.h>      // Library untuk membuat dan membaca format data JSON
// #include <DHT.h>           // Library DHT tidak digunakan karena data suhu disimulasikan

const char* ssid = "personalX";        // Nama jaringan WiFi (SSID) yang akan dihubungi
const char* password = "177013003";    // Kata sandi/password WiFi

const char* mqttServer = "broker.hivemq.com"; // Alamat server/broker MQTT publik HiveMQ
const int mqttPort = 1883;                    // Port standar komunikasi MQTT tanpa enkripsi

const char* topicData = "ahlele/ahlelasData";          // Topik MQTT tempat ESP8266 mengirim (publish) data
const char* topicPerintah = "ahlele/ahlelasPerintah";  // Topik MQTT tempat ESP8266 menerima (subscribe) perintah

// #define DHTPIN 4           // Konfigurasi pin DHT belum digunakan
// #define DHTTYPE DHT22      // Tipe sensor DHT belum digunakan

const int ledPin = D4;        // Mendefinisikan pin D4 ESP8266 untuk LED/aktuator

// DHT dht(DHTPIN, DHTTYPE);  // Sensor DHT belum digunakan dalam pengujian ini

WiFiClient espClient;           // Objek koneksi jaringan WiFi
PubSubClient client(espClient); // Objek klien MQTT yang menggunakan jaringan dari espClient

unsigned long waktuTerakhirPublish = 0; // Variabel penampung waktu (ms) terakhir data dikirim
const long intervalPublish = 5000;      // Pengatur jeda pengiriman data (5000 ms = 5 detik)

// Fungsi callback yang otomatis berjalan saat ada pesan MQTT masuk
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan; // Variabel lokal penampung isi pesan MQTT

  // Menyatukan karakter payload byte demi byte menjadi satu String
  for (unsigned int i = 0; i < length; i++)
    pesan += (char)payload[i];

  JsonDocument doc; // Menyiapkan wadah objek dokumen JSON

  if (deserializeJson(doc, pesan)) return; // Mengubah String ke JSON; jika format salah, batalkan fungsi

  const char* perintah = doc["perintah"]; // Mengambil nilai dari key "perintah" di dalam JSON

  // Menyala/mematikan LED berdasarkan isi pesan (jika "ON" maka HIGH, selain itu LOW)
  digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);

  Serial.print("Perintah diterima -> Aktuator: "); // Menampilkan pesan ke Serial Monitor
  Serial.println(perintah);                        // Menampilkan nilai perintah ke Serial Monitor
}

// Fungsi untuk menghubungkan ESP8266 ke jaringan WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password); // Memulai proses koneksi ke router WiFi

  // Melakukan perulangan dan menunggu hingga status WiFi terhubung
  while (WiFi.status() != WL_CONNECTED)
    delay(500); // Jeda 0.5 detik di tiap pengecekan

  Serial.println("WiFi berhasil terhubung!"); // Pesan indikator sukses WiFi di Serial Monitor
}

// Fungsi untuk menghubungkan ESP8266 ke broker MQTT
void hubungkanMQTT() {
  // Melakukan perulangan sampai terhubung ke server MQTT
  while (!client.connected()) {

    // Membuat ID unik acak untuk klien MQTT agar tidak bertabrakan dengan device lain
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    // Mencoba terhubung ke server dengan Client ID acak tadi
    if (client.connect(clientId.c_str())) {

      client.subscribe(topicPerintah); // Mendaftarkan diri (subscribe) ke topik perintah

      Serial.println("Terhubung dan subscribe topic perintah"); // Indikator sukses MQTT

    } else {

      delay(2000); // Jika gagal terhubung, tunggu 2 detik sebelum coba lagi
    }
  }
}

// Fungsi setup: Dijalankan hanya 1 kali saat mikrokontroler pertama kali menyala
void setup() {
  Serial.begin(115200); // Membuka komunikasi serial ke komputer pada baudrate 115200

  pinMode(ledPin, OUTPUT); // Mengatur pin LED (D4) sebagai output/keluaran

  // dht.begin();              // Inisialisasi sensor DHT tidak diperlukan karena menggunakan data simulasi

  hubungkanWiFi(); // Memanggil fungsi koneksi WiFi

  client.setServer(mqttServer, mqttPort); // Mengatur alamat broker dan port MQTT
  client.setCallback(callback);           // Mengatur fungsi callback yang merespons pesan masuk
}

// Fungsi loop: Dijalankan terus-menerus secara berulang setelah setup selesai
void loop() {

  // Jika koneksi MQTT putus, panggil fungsi reconnection
  if (!client.connected())
    hubungkanMQTT();

  client.loop(); // Memproses tugas internal MQTT (menjaga koneksi & cek pesan masuk)

  // Mengecek apakah sudah berlalu 5 detik sejak publish terakhir (non-blocking)
  if (millis() - waktuTerakhirPublish > intervalPublish) {

    waktuTerakhirPublish = millis(); // Memperbarui nilai waktu pengiriman terakhir

    // float suhu = dht.readTemperature();   // Pembacaan sensor tidak digunakan dalam pengujian ini
    // if (!isnan(suhu)) {                    // Validasi sensor tidak diperlukan karena data disimulasikan

    int suhu = random(28, 31); // Membuat data suhu simulasi secara acak (28, 29, atau 30)

    JsonDocument doc; // Menyiapkan wadah dokumen JSON baru

    doc["suhu"] = suhu; // Memasukkan key "suhu" dengan nilai simulasi ke dokumen JSON

    char buffer[128]; // Menyiapkan array karakter penampung teks JSON

    serializeJson(doc, buffer); // Mengubah format objek JSON menjadi String teks di dalam buffer

    client.publish(topicData, buffer); // Mengirimkan data JSON ke topik MQTT

    Serial.print("Data terkirim: "); // Menampilkan log pengiriman ke Serial Monitor
    Serial.println(buffer);          // Cetak isi buffer data JSON

    // } // Penutup validasi sensor tidak digunakan karena memakai data simulasi
  }
}