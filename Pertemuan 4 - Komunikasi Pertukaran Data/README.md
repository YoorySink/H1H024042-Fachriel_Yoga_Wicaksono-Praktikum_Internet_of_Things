## modul 4 - Komunikasi Pertukaran Data
# percobaan praktikum

### 4.5 Percobaan 4A:  Subscribe dan Deserialisasi Data JSON untuk Kendali Aktuator
Fokus pada percobaan ini mengimplementasikan mekanisme subscribe pada MQTT beserta proses deserialisasi data JSON untuk mengendalikan aktuator berdasarkan perintah yang diterima

<details>
<summary>Lihat kode percobaan 4A</summary>

```C
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
 if (String(perintah) == "ON") {
 digitalWrite(ledPin, HIGH);
 Serial.println("Aktuator: ON");
 } else if (String(perintah) == "OFF") {
 digitalWrite(ledPin, LOW);
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
 digitalWrite(ledPin, LOW);
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

```
</details>

### 4.6 Percobaan 4B: Pertukaran Data Dua Arah (Publish dan Subscribe Secara Bersamaan)
Fokus pada percobaan ini adalah mengimplementasikan sistem IoT yang dapat mempublikasikan data sensor dan menerima perintah kendali secara bersamaan (full duplex)

<details>
<summary>Lihat kode percobaan 4B</summary>

```cpp
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
```
</details>


# Library

### Percobaan 3A

- <ESP8266WiFi.h>       : library untuk berinteraksi dengan WiFi pada ESP8266.
- <ESP8266HTTPClient.h> : library untuk komunikasi HTTP.
- <WiFiClientSecure.h>  : library untuk koneksi HTTPS.
- <ArduinoJson.h>       : library untuk mengolah data JSON.

### Percobaan 3B

- <ESP8266WiFi.h>       : library untuk berinteraksi dengan WiFi pada ESP8266.
- <PubSubClient.h>      : library untuk komunikasi menggunakan protokol MQTT.
- <ArduinoJson.h>       : library untuk mengolah data JSON.



# Pertanyaan praktikum

### 4.5.4 Pertanyaan Praktikum: 
Modifikasi program agar data JSON yang diterima juga memuat nilai intensitas (misalnya {"perintah": "ON", "intensitas": 200}) yang digunakan untuk mengatur kecerahan LED menggunakan PWM (analogWrite/ledcWrite), dan berikan penjelasan di setiap baris kode yang ditambahkan

```Cpp
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
```
#### Penjelasan Modifikasi program yang ditambahkan :

1. baris dibawah mengambil nilai `intensitas` dari JSON yang diterima. Operator `| 255` berarti jika key `intensitas` tidak ada di JSON, maka nilai default 255 (kecerahan penuh) yang dipakai.
```cpp 
int intensitas = doc["intensitas"] | 255;
```
2. baris dibawah membatasi nilai intensitas agar selalu berada di rentang 0 sampai 255, sehingga nilai di luar rentang (misalnya 500 atau -10) tidak menyebabkan PWM bekerja tidak wajar.
```cpp
intensitas = constrain(intensitas, 0, 255);
```
3. baris dibawah menyalakan LED dengan kecerahan sesuai nilai intensitas menggunakan PWM. Semakin besar nilainya, semakin terang LED menyala. Ini menggantikan `digitalWrite(ledPin, HIGH)` yang hanya bisa menyala penuh.
```cpp
analogWrite(ledPin, intensitas);
```
4. baris dibawah menampilkan nilai intensitas yang sedang dipakai ke Serial Monitor untuk memudahkan pengecekan.
```cpp
Serial.print("Aktuator: ON, intensitas: ");
Serial.println(intensitas);
```
5. baris dibawah mematikan LED saat perintah OFF dengan memberi nilai PWM 0 (duty cycle 0%). Ini menggantikan `digitalWrite(ledPin, LOW)`.
```cpp
analogWrite(ledPin, 0);
```
6. baris dibawah (di `setup()`) mengubah rentang PWM ESP8266 menjadi 0-255. Secara bawaan ESP8266 memakai rentang 0-1023, sehingga tanpa baris ini nilai 200 hanya menghasilkan kecerahan sekitar 20%.
```cpp
analogWriteRange(255);
```
7. baris dibawah (di `setup()`) memastikan LED dalam keadaan mati saat program pertama kali berjalan.
```cpp
analogWrite(ledPin, 0);
```

#### potongan program dan penjelasannya :
```cpp
...
  const char* perintah = doc["perintah"];
  int intensitas = doc["intensitas"] | 255;
  intensitas = constrain(intensitas, 0, 255);

  if (String(perintah) == "ON") {
    analogWrite(ledPin, intensitas);
    ...
  }
...
```
> Program di atas membaca perintah dan intensitas dari JSON, lalu mengatur kecerahan LED dengan PWM. Contoh pesan: `{"perintah": "ON", "intensitas": 200}` menyalakan LED dengan kecerahan 200 dari 255, sedangkan `{"perintah": "OFF"}` mematikan LED.

### 4.6.4 Pertanyaan Praktikum: 
Modifikasi program agar menambahkan satu topic perintah baru untuk mengendalikan aktuator kedua (misalnya buzzer), dengan fungsi callback yang dapat membedakan topic mana yang menerima pesan, dan berikan penjelasan di setiap baris kode yang ditambahkan

```Cpp
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
```
#### Penjelasan Modifikasi program yang ditambahkan :

1. baris dibawah mendefinisikan topic MQTT baru khusus untuk perintah buzzer, sehingga LED dan buzzer dikendalikan lewat jalur pesan yang terpisah.
```cpp 
const char* topicBuzzer = "ahlele/ahlelasBuzzer";
```
2. baris dibawah mendefinisikan pin D5 sebagai pin tempat buzzer (aktuator kedua) terhubung.
```cpp
const int buzzerPin = D5;
```
3. baris dibawah membandingkan nama topic pesan yang masuk dengan `topicPerintah`. `strcmp` bernilai 0 jika kedua teks sama, sehingga blok di dalamnya hanya berjalan untuk pesan dari topic LED.
```cpp
if (strcmp(topic, topicPerintah) == 0) {
```
4. baris dibawah (di dalam blok topic LED) menyalakan/mematikan LED sesuai isi `perintah`, lalu mencetak perintah yang diterima ke Serial Monitor. Kode ini sama seperti sebelumnya, hanya sekarang dibungkus dalam `if` agar hanya berjalan untuk pesan dari topic LED.
```cpp
digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
Serial.print("Perintah diterima -> Aktuator: ");
Serial.println(perintah);
```
5. baris dibawah membandingkan nama topic pesan yang masuk dengan `topicBuzzer`. Blok di dalamnya hanya berjalan jika pesan berasal dari topic buzzer, inilah cara callback membedakan topic.
```cpp
else if (strcmp(topic, topicBuzzer) == 0) {
```
6. baris dibawah (di dalam blok topic buzzer) menyalakan/mematikan buzzer sesuai isi `perintah`, lalu mencetak log dengan penanda "Buzzer" ke Serial Monitor sebagai tanda bahwa perintah berasal dari topic buzzer.
```cpp
digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW);
Serial.print("Perintah diterima -> Buzzer: ");
Serial.println(perintah);
```
7. baris dibawah mendaftarkan ESP8266 sebagai subscriber topic buzzer. Tanpa baris ini, pesan yang dikirim ke topic buzzer tidak akan pernah diterima. Teks Serial setelahnya juga diubah agar menunjukkan bahwa kedua topic sudah di-subscribe.
```cpp
client.subscribe(topicBuzzer);
Serial.println("Terhubung dan subscribe topic perintah & buzzer");
```
8. baris dibawah (di `setup()`) mengatur pin buzzer sebagai output dan memastikan buzzer mati di awal program.
```cpp
pinMode(buzzerPin, OUTPUT);
digitalWrite(buzzerPin, LOW);
```

#### potongan program dan penjelasannya :
```cpp
...
  if (strcmp(topic, topicPerintah) == 0) {
    digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
    ...
  }
  else if (strcmp(topic, topicBuzzer) == 0) {
    digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW);
    ...
  }
...
```
> Program di atas membedakan pesan berdasarkan topic-nya. Kirim `{"perintah": "ON"}` ke `ahlele/ahlelasPerintah` untuk menyalakan LED, dan kirim pesan yang sama ke `ahlele/ahlelasBuzzer` untuk membunyikan buzzer.

# dokumentasi
<style>
  .frame {
    width: 350px;
    height: 250px;
    object-fit: cover;
  }
</style>
<table align="center">
  <tr>
    <th colspan="2" align="center" style="text-align: center;">alat dan bahan modul 4</th>
  </tr>
  <tr>
    <td colspan="2" align="center">
      <img src="Dokumentasi/alat dan bahan.jpg" class="frame">
    </td>
  </tr>
  <tr>
    <th style="text-align: center;">Dokumentasi Percobaan 4A</th>
    <th style="text-align: center;">Dokumentasi Percobaan 4B</th>
  </tr>
  <tr>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 4A.jpg" class="frame">
    </td>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 4B.jpg" class="frame">
    </td>
  </tr>
  <tr>
    <th style="text-align: center;">Dokumentasi Video Percobaan 4A</th>
    <th style="text-align: center;">Dokumentasi Video Percobaan 4B</th>
  </tr>
  <tr>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi video percobaan 4A.gif" class="frame">
    </td>
    <td align="center">
      <img src=" " class="frame">
    </td>
  </tr>
</table>