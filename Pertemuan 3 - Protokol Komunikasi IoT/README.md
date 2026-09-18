## modul 3 - Protokol Komunikasi IoT
# percobaan praktikum

### Percobaan 3A: Komunikasi Data Menggunakan HTTP
Pada percobaan ini, ESP8266 mengirimkan data suhu dan kelembaban dalam format JSON ke server menggunakan metode HTTP POST, kemudian menampilkan kode dan isi respons server pada Serial Monitor.
<details>
<summary>Lihat kode percobaan 3A</summary>

```cpp
#include <ESP8266WiFi.h> // Library koneksi WiFi ESP8266
#include <ESP8266HTTPClient.h> // Library fungsi HTTP (POST/GET)
#include <WiFiClientSecure.h> // Library koneksi aman HTTPS
#include <ArduinoJson.h> // Library untuk format JSON


const char* ssid = "personalX"; // Nama WiFi
const char* password = "177013003"; // Password WiFi
const char* serverName = "https://httpbin.org/post"; // URL server tujuan


void setup() {
  Serial.begin(115200); // Memulai komunikasi serial dengan kecepatan 115200
  WiFi.begin(ssid, password); // Memulai proses sambung WiFi
 
  Serial.print("Menghubungkan ke WiFi"); // Cetak teks loading
  while (WiFi.status() != WL_CONNECTED) { // Ulangi terus selama WiFi belum nyambung
    delay(500); // Jeda 0,5 detik
    Serial.print("."); // Cetak titik (animasi loading)
  }
  serial.println();
  Serial.println("\nWiFi berhasil terhubung!"); // Cetak teks sukses
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) { // Pastikan WiFi masih terhubung
    WiFiClientSecure client; // Buat klien untuk jalur HTTPS
    client.setInsecure(); // Matikan verifikasi sertifikat SSL (biar RAM nggak penuh)
   
    HTTPClient http; // Buat eksekutor HTTP
    http.begin(client, serverName); // Siapkan pengiriman ke URL tujuan
    http.addHeader("Content-Type", "application/json"); // Beri label bahwa paketnya berupa JSON


    JsonDocument doc; // Buat wadah JSON
    doc["suhu"] = 28.5; // Isi variabel suhu
    doc["kelembaban"] = 65.0; // Isi variabel kelembaban
   
    String requestBody; // Siapkan variabel teks kosong
    serializeJson(doc, requestBody); // Ubah JSON jadi format teks (String)


    Serial.print("\nMengirim data: ");
    Serial.println(requestBody);       //nyetak data JSON-nya
   
    int httpResponseCode = http.POST(requestBody); // Kirim data (POST) & catat kode balasannya
   
    Serial.print("Kode Response HTTP: "); // Cetak label balasan
    Serial.println(httpResponseCode); // Cetak angka balasan (contoh: 200 = Sukses)


    if (httpResponseCode > 0) { // Jika pengiriman jalan (tidak error/putus)
      String response = http.getString(); // Sedot teks balasan dari server
      Serial.println("Isi Response:"); // Cetak teks label
      Serial.println(response); // Tampilkan balasan dari server
    } else { // Jika gagal konek ke server
      Serial.print("Error saat mengirim POST: "); // Cetak label error
      Serial.println(http.errorToString(httpResponseCode).c_str()); // Cetak alasan error-nya
    }
   
    http.end(); // Putus koneksi HTTP agar memori alat tidak bocor/hang
  }
 
  delay(10000); // Jeda 10 detik sebelum mengulang ke atas
}

```
</details>

### 3.6 Percobaan 3B: Komunikasi MQTT
Pada percobaan ini, ESP8266 mengirimkan data suhu dan kelembaban dalam format JSON menggunakan protokol MQTT melalui broker HiveMQ ke topic yang telah ditentukan.

<details>
<summary>Lihat kode percobaan 3B</summary>

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


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

### 3.5.4 Pertanyaan Praktikum: 
Modifikasi program agar ESP32 dapat mengirimkan data tambahan berupa waktu
(dalam milidetik sejak dinyalakan menggunakan millis()) ke dalam JSON yang dikirim,
dan berikan penjelasan di setiap baris kode yang ditambahkan!

```Cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
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
```
#### Penjelasan Modifikasi program yang ditambahkan :

1. baris dibawah menambahkan field waktu ke JSON yang berisi waktu dalam milidetik sejak ESP32 dinyalakan, sehingga nilainya akan terus bertambah setiap program berjalan.
```cpp 
doc["waktu"] = millis(); 
```
2. baris dibawah membuat objek client untuk menangani koneksi HTTPS ke server.
```cpp
WiFiClientSecure client;
```
3. baris dibawah menonaktifkan verifikasi sertifikat SSL agar ESP8266 dapat terhubung ke server HTTPS tanpa memerlukan sertifikat.
```cpp
client.setInsecure();
```

#### potongan program dan penjelasannya :
```cpp
...
   JsonDocument doc; // Buat wadah JSON
    doc["suhu"] = 28.5; // Isi variabel suhu
    doc["kelembaban"] = 65.0; // Isi variabel kelembaban
    doc["iniWaktu"] = millis(); //isi waktu sejak ESP32 menyala (ms)
...
```
> Program di atas menampilkan data suhu, kelembaban, dan waktu sejak ESP32 menyala dalam format JSON di Serial Monitor

# dokumentasi
<table align="center">
  <tr>
    <th colspan="2" align="center" style="text-align: center;">alat dan bahan modul 3</th>
  </tr>
  <tr>
    <td colspan="2" align="center">
      <img src="Dokumentasi/alat dan bahan.jpg" width="350">
    </td>
  </tr>
  <tr>
    <th style="text-align: center;">Dokumentasi Percobaan 3A</th>
    <th style="text-align: center;">Dokumentasi Percobaan 3B</th>
  </tr>
  <tr>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 3A.jpg" width="350">
    </td>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 3B.jpg" width="350">
    </td>
  </tr>
  <tr>
    <th style="text-align: center;">Dokumentasi Video Percobaan 3A</th>
    <th style="text-align: center;">Dokumentasi Video Percobaan 3B</th>
  </tr>
  <tr>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi video percobaan 3A.gif" width="350">
    </td>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi video percobaan 3B.gif" width="350">
    </td>
  </tr>
</table>
