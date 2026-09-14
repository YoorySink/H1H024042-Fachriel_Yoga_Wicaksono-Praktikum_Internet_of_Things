## modul 2 - Konfigurasi Jaringan
# percobaan praktikum
---
### Percobaan 2A: Konfigurasi Mode Station (STA)
Pada percobaan pertama, kita mengatur esp8266 menjadi sebuah sta dan mengkoneksikan esp8266 ke hotspot hp dan memantau kekuatan wifi RSSI di serial monitor
<details>
<summary>Lihat kode percobaan 2A</summary>

```cpp
#include <ESP8266WiFi.h>
const char* ssid     = "personalX";
const char* password = "177013003";
const int ledPin = 2;   // LED indikator status koneksi

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Set mode WiFi menjadi Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Jika berhasil terhubung
  Serial.println();
  Serial.println("WiFi berhasil terhubung!");
  Serial.print("IP Address  : ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address : ");
  Serial.println(WiFi.macAddress());
  Serial.print("RSSI (dBm)  : ");
  Serial.println(WiFi.RSSI());

  digitalWrite(ledPin, HIGH); // nyalakan LED sebagai indikator
}

void loop() {
  // Cek status koneksi setiap 5 detik
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Status: Terhubung");
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("Status: Terputus");
    digitalWrite(ledPin, LOW);
  }
  Serial.print("RSSI (dBm)  : ");
  Serial.println(WiFi.RSSI());
  delay(5000);
}
```
</details>

### 2.6 Percobaan 2B: Konfigurasi Mode Access Point (AP)
Pada percobaan kedua, kita mengatur esp8266 menjadi sebuah AP dan menghubungkan beberapa hp ke esp8266 dan memantau status perangkat yang terhubung

<details>
<summary>Lihat kode percobaan 2B</summary>

```cpp
#include <ESP8266WiFi.h>
const char* ap_ssid = "ESP32_AccessPoint";
const char* ap_password = "12345678"; // minimal 8 karakter
void setup() {
 Serial.begin(115200);
 // Set mode WiFi menjadi Access Point
 WiFi.mode(WIFI_AP);
 WiFi.softAP(ap_ssid, ap_password);
 IPAddress apIP = WiFi.softAPIP();
 Serial.println("Access Point aktif!");
 Serial.print("SSID : ");
 Serial.println(ap_ssid);
 Serial.print("IP Address : ");
 Serial.println(apIP);
}
void loop() {
 // Menampilkan jumlah perangkat yang terhubung setiap 5 detik
 int jumlahClient = WiFi.softAPgetStationNum();
 Serial.print("Jumlah perangkat terhubung: ");
 Serial.println(jumlahClient);
 delay(5000);
}
```
</details>


# Library
---
### percobaan 2A

- <ESP8266WiFi.h>: library untuk berinteraksi dengan WiFi pada esp8266

### Percobaan 2B

- <ESP8266WiFi.h>: library untuk berinteraksi dengan WiFi pada esp8266


# Pertanyaan praktikum
---
### 2.5.4 Pertanyaan Praktikum: 
Modifikasi program agar ESP32 mencoba menghubungkan ulang (reconnect) secara
otomatis apabila koneksi WiFi terputus, dan berikan penjelasan di setiap baris kode yang ditambahkan!
```Cpp
#include <ESP8266WiFi.h>
const char* ssid     = "personalX";
const char* password = "177013003";
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Mengaktifkan fitur reconnect otomatis
  WiFi.setAutoReconnect(true);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi berhasil terhubung!");
  Serial.print("IP Address  : ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address : ");
  Serial.println(WiFi.macAddress());
  Serial.print("RSSI (dBm)  : ");
  Serial.println(WiFi.RSSI());

  digitalWrite(ledPin, HIGH);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Status: Terhubung");
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("Status: Terputus");
    digitalWrite(ledPin, LOW);
  }
  Serial.print("RSSI (dBm)  : ");
  Serial.println(WiFi.RSSI());
  delay(5000);
}
```
Penjelasan Modifikasi program yang ditambahkan :
```cpp 
WiFi.setAutoReconnect(true); 
```
Baris ini mengaktifkan fitur auto reconnect pada ESP8266 sehingga perangkat akan mencoba menghubungkan kembali ke jaringan WiFi secara otomatis apabila koneksi terputus.
> 
```cpp
...
WiFi.setAutoReconnect(true); // Mengaktifkan fitur reconnect otomatis jika WiFi terputus 
Serial.print("Menghubungkan ke WiFi"); // Menampilkan pesan proses koneksi pada Serial Monitor 
while (WiFi.status() != WL_CONNECTED) { // Menunggu sampai ESP8266 berhasil terhubung 
delay(500); // Memberikan jeda selama 500 milidetik 
Serial.print("."); // Menampilkan titik sebagai indikator proses koneksi 
}
...
```

### 2.6.4 Pertanyaan praktikum
Modifikasi program agar ESP32 berjalan pada mode AP+STA (terhubung ke WiFi
rumah sekaligus menyediakan Access Point), dan berikan penjelasan di setiap baris kode nya!
```cpp
#include <ESP8266WiFi.h>

const char* sta_ssid = "NamaWiFiRumah";
const char* sta_password = "PasswordWiFi";
const char* ap_ssid = "ESP8266_AccessPoint";
const char* ap_password = "12345678";

void setup() {
  Serial.begin(115200);
  // Mengatur ESP8266 menjadi AP + STA
  WiFi.mode(WIFI_AP_STA);
  // Menghubungkan ESP8266 ke WiFi rumah
  WiFi.begin(sta_ssid, sta_password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Jika berhasil terhubung ke wifi
  Serial.println();
  Serial.println("WiFi rumah berhasil terhubung!");
  Serial.print("IP STA : ");
  Serial.println(WiFi.localIP());

  // Membuat Access Point
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point aktif!");
  Serial.print("SSID AP : ");
  Serial.println(ap_ssid);
  Serial.print("IP AP : ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Mengecek status WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Status STA: Terhubung");
    Serial.print("IP STA : ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Status STA: Terputus");
  }
  // Menampilkan jumlah perangkat yang terhubung ke Access Point
  int jumlahClient = WiFi.softAPgetStationNum();
  Serial.print("Jumlah perangkat AP: ");
  Serial.println(jumlahClient);

  delay(5000);
}
```

### Penjelasan Modifikasi program yang ditambahkan
1. Baris ini mengatur ESP8266 agar bekerja dalam dua mode sekaligus, yaitu sebagai Station (STA) yang terhubung ke WiFi rumah dan sebagai Access Point (AP) yang menyediakan jaringan WiFi sendiri.
```cpp
WiFi.mode(WIFI_AP_STA);
```
2. Baris ini digunakan untuk menghubungkan ESP8266 ke WiFi rumah sebagai Station.

```cpp
WiFi.begin(sta_ssid, sta_password);
```
3. Baris ini digunakan untuk membuat Access Point pada ESP8266 sehingga perangkat lain dapat terhubung ke WiFi yang dibuat ESP8266.

```cpp
WiFi.softAP(ap_ssid, ap_password);
```
4. Baris ini digunakan untuk mengetahui jumlah perangkat yang sedang terhubung ke Access Point ESP8266.

```cpp
WiFi.softAPgetStationNum();
```
##### potongan program dan penjelasannya
```cpp
// Menambahkan SSID dan password untuk STA 
const char* sta_ssid = "NamaWiFiRumah"; // SSID WiFi rumah 
const char* sta_password = "PasswordWiFi"; // Password WiFi rumah // Menambahkan SSID dan password untuk AP 
const char* ap_ssid = "ESP8266_AccessPoint"; // SSID Access Point 
const char* ap_password = "12345678"; // Password Access Point
...
```
> program diatas menambahkan ssid dan password untuk mode sta dan ap
```cpp
...
  // Mengatur ESP8266 menjadi AP + STA
  WiFi.mode(WIFI_AP_STA);
  // Menghubungkan ESP8266 ke WiFi rumah
  WiFi.begin(sta_ssid, sta_password);
  ...
```
> program diatas mengatur esp8266 menjadi AP dan STA sekaligus menghubungkan esp8266 ke wifi dengan ssid dan password yang tersedia (ada)
```cpp
  ...
  // Jika berhasil terhubung ke wifi
  Serial.println();
  Serial.println("WiFi rumah berhasil terhubung!"); // Menampilkan status koneksi STA
  // Menampilkan IP Address STA
  Serial.print("IP STA : ");
  Serial.println(WiFi.localIP());
```
```cpp
  // Membuat Access Point
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point aktif!"); // Menampilkan status Access Point
  Serial.print("SSID AP : ");
  Serial.println(ap_ssid);
  // Menampilkan IP Address Access Point
  Serial.print("IP AP : ");
  Serial.println(WiFi.softAPIP());
  ...
```
> Dua potongan program di atas digunakan untuk menampilkan informasi koneksi STA dan AP, yaitu IP Address yang diperoleh ESP8266 dari WiFi rumah dan IP Address yang digunakan oleh Access Point ESP8266.
<p align="center">
  <img src="Dokumentasi/alat dan bahan.jpg" width="350" height="250">
  <img src="Dokumentasi/dokumentasi video percobaan 2B.gif" width="350" height="250">
</p>
<p align="center">
  <img src="Dokumentasi/Dokumentasi percobaan 2A.jpg" width="350">
  <img src="Dokumentasi/Dokumentasi percobaan 2B.jpg" width="350">
</p>