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