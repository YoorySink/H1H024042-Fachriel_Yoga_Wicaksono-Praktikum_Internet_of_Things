#include <ESP8266WiFi.h>      // Library untuk menghubungkan ESP8266 ke jaringan WiFi
#include <PubSubClient.h>     // Library untuk komunikasi menggunakan protokol MQTT
#include <ArduinoJson.h>      // Library untuk membaca data dalam format JSON

const char* ssid = "personalX";        // Nama jaringan WiFi yang akan dihubungi
const char* password = "177013003";    // Kata sandi jaringan WiFi
const char* mqttServer = "broker.hivemq.com"; // Alamat broker MQTT yang digunakan
const int mqttPort = 1883;                    // Port standar MQTT tanpa enkripsi
const char* topicPerintah = "ahlele/ahlelas"; // Topik MQTT untuk menerima perintah
const int ledPin = D4;                        // Pin D4 ESP8266 yang digunakan sebagai aktuator

WiFiClient espClient;           // Objek klien untuk koneksi jaringan WiFi
PubSubClient client(espClient); // Objek klien MQTT yang menggunakan koneksi WiFi

// Fungsi callback yang otomatis dijalankan saat pesan MQTT baru diterima
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan; // Variabel untuk menyimpan isi pesan yang diterima

  // Menggabungkan setiap byte payload menjadi satu teks pesan
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i]; // Mengubah byte saat ini menjadi karakter
  } // Mengakhiri perulangan pembacaan payload

  Serial.print("Pesan diterima ["); // Menampilkan awal informasi pesan
  Serial.print(topic);             // Menampilkan nama topik pesan
  Serial.print("]: ");             // Menampilkan pemisah sebelum isi pesan
  Serial.println(pesan);           // Menampilkan isi pesan yang diterima

  // Menyiapkan wadah untuk mengurai data JSON yang diterima
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, pesan); // Mengubah teks pesan menjadi dokumen JSON

  // Memeriksa apakah proses penguraian JSON mengalami kesalahan
  if (error) {
    Serial.print("Gagal parsing JSON: "); // Menampilkan informasi kegagalan
    Serial.println(error.c_str());        // Menampilkan rincian kesalahan JSON
    return;                               // Menghentikan callback jika JSON tidak valid
  } // Mengakhiri pemeriksaan kesalahan JSON

  const char* perintah = doc["perintah"]; // Mengambil nilai dari key "perintah" pada JSON

  // Menyalakan LED jika perintah bernilai "ON"
  if (String(perintah) == "ON") {
    digitalWrite(ledPin, HIGH);       // Mengatur pin aktuator ke kondisi HIGH
    Serial.println("Aktuator: ON");   // Menampilkan status aktuator menyala
  } else if (String(perintah) == "OFF") {
    digitalWrite(ledPin, LOW);        // Mengatur pin aktuator ke kondisi LOW
    Serial.println("Aktuator: OFF"); // Menampilkan status aktuator mati
  } // Mengakhiri pemeriksaan perintah ON dan OFF
} // Mengakhiri fungsi callback

// Fungsi untuk menghubungkan ESP8266 ke jaringan WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password); // Memulai koneksi WiFi menggunakan kredensial yang ditentukan
  Serial.print("Menghubungkan ke WiFi"); // Menampilkan status proses koneksi

  // Menunggu hingga ESP8266 berhasil tersambung ke jaringan WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);           // Menunggu selama 500 milidetik sebelum memeriksa kembali
    Serial.print(".");    // Menampilkan tanda titik sebagai indikator proses koneksi
  } // Mengakhiri perulangan setelah WiFi terhubung

  Serial.println("\nWiFi berhasil terhubung!"); // Menampilkan pemberitahuan koneksi berhasil
} // Mengakhiri fungsi koneksi WiFi

// Fungsi untuk menghubungkan klien ke broker MQTT dan berlangganan topik perintah
void hubungkanMQTT() {
  // Mengulangi percobaan selama klien belum terhubung ke broker MQTT
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT..."); // Menampilkan status koneksi MQTT
    String clientId = "ESP32Client-" + String(random(0xffff), HEX); // Membuat ID klien dengan nilai acak

    // Mencoba menghubungkan klien menggunakan ID yang telah dibuat
    if (client.connect(clientId.c_str())) {
      Serial.println("berhasil terhubung!"); // Menampilkan status koneksi berhasil
      client.subscribe(topicPerintah); // Berlangganan ke topik untuk menerima perintah
      Serial.print("Subscribe ke topic: "); // Menampilkan informasi topik langganan
      Serial.println(topicPerintah);       // Menampilkan nama topik yang diikuti
    } else {
      Serial.print("gagal, rc=");     // Menampilkan status koneksi yang gagal
      Serial.print(client.state());   // Menampilkan kode status dari klien MQTT
      Serial.println(" coba lagi dalam 2 detik"); // Menampilkan jeda sebelum mencoba kembali
      delay(2000);                    // Menunggu dua detik sebelum percobaan berikutnya
    } // Mengakhiri pemeriksaan hasil koneksi MQTT
  } // Mengakhiri perulangan setelah berhasil terhubung
} // Mengakhiri fungsi koneksi MQTT

// Fungsi setup dijalankan satu kali saat ESP8266 mulai bekerja
void setup() {
  Serial.begin(115200);       // Memulai komunikasi serial dengan baud rate 115200
  pinMode(ledPin, OUTPUT);    // Mengatur pin aktuator sebagai keluaran
  digitalWrite(ledPin, LOW);  // Memastikan aktuator berada dalam kondisi mati saat awal
  hubungkanWiFi();            // Menghubungkan perangkat ke jaringan WiFi
  client.setServer(mqttServer, mqttPort); // Mengatur alamat dan port broker MQTT
  client.setCallback(callback); // Mendaftarkan callback untuk menangani pesan yang masuk
} // Mengakhiri fungsi setup

// Fungsi loop dijalankan berulang kali setelah fungsi setup selesai
void loop() {
  // Memeriksa koneksi MQTT dan menghubungkan ulang jika koneksi terputus
  if (!client.connected()) {
    hubungkanMQTT(); // Menjalankan proses koneksi ulang ke broker MQTT
  } // Mengakhiri pemeriksaan koneksi MQTT

  client.loop(); // Memproses komunikasi MQTT agar pesan dapat diterima
} // Mengakhiri fungsi loop
