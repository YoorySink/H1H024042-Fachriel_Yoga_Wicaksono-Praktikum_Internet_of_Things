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
