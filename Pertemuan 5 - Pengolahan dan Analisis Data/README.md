## modul 5 - Pengolahan dan Analisis Data
# percobaan praktikum

### 5.5 Percobaan 5A: Pengolahan Data Sensor pada ESP (Moving Average dan Statistik)
Fokus pada percobaan ini adalah mengimplementasikan pengolahan data sensor pada sisi perangkat (edge processing) menggunakan filter moving average serta pencatatan nilai minimum dan maksimum. Karena sensor DHT22 rusak, pembacaan suhu diganti dengan fungsi data dummy `bacaSuhuMock()`.

<details>
<summary>Lihat kode percobaan 5A</summary>

```cpp
//#include <DHT.h>
//#define DHTPIN 4
//#define DHTTYPE DHT22
//DHT dht(DHTPIN, DHTTYPE);

// Variabel moving average dan statistik
const int N = 5;         // jumlah sampel moving average
float buffer[N];         // menyimpan N pembacaan terakhir
int indeks = 0;          // posisi penulisan berikutnya pada buffer
int jumlah = 0;          // jumlah sampel yang sudah terisi

float suhuMin = 1000.0;  // nilai awal ekstrem agar pembacaan pertama langsung tercatat
float suhuMax = -1000.0;

// Mock function: simulasi suhu (tren sinus + noise + lonjakan anomali)
float bacaSuhuMock() {
    static unsigned long jumlahBaca = 0;
    jumlahBaca++;

    // Suhu dasar berubah secara bertahap
    float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);

    // Noise kecil untuk mensimulasikan fluktuasi sensor
    float noise = random(-5, 6) * 0.1;

    float suhu = suhuDasar + noise;

    // Lonjakan suhu setiap 25 pembacaan untuk simulasi anomali
    if (jumlahBaca % 25 == 0) {
        suhu += 8.0;
    }

    return suhu;
}

// Filter moving average: rata-rata maksimal N pembacaan terakhir (circular buffer)
float hitungMovingAverage(float nilaiBaru) {
  buffer[indeks] = nilaiBaru;
  indeks = (indeks + 1) % N;
  if (jumlah < N) jumlah++;

  float total = 0;
  for (int i = 0; i < jumlah; i++) {
    total += buffer[i];
  }
  return total / jumlah;
}

void setup() {
  Serial.begin(115200);
  //dht.begin();
}

void loop() {
  // Akuisisi data: ambil suhu dari mock (pengganti dht.readTemperature())
  //float suhu = dht.readTemperature();
  float suhu =  bacaSuhuMock();

  if (isnan(suhu)) {
    Serial.println("Pembacaan gagal, data diabaikan");
  } else {
    // Pengolahan: filter moving average serta pencatatan min dan max
    float suhuFilter = hitungMovingAverage(suhu);

    if (suhu < suhuMin) suhuMin = suhu;
    if (suhu > suhuMax) suhuMax = suhu;

    // Keluaran: tampilkan data mentah, hasil filter, min, dan max di Serial Monitor
    Serial.print("Mentah: ");
    Serial.print(suhu);
    Serial.print(" | Filter: ");
    Serial.print(suhuFilter);
    Serial.print(" | Min: ");
    Serial.print(suhuMin);
    Serial.print(" | Max: ");
    Serial.println(suhuMax);
  }

  delay(2000);  // jeda 2 detik antar pembacaan
}
```
</details>

### 5.6 Percobaan 5B: Akuisisi, Analisis, dan Visualisasi Data
Fokus pada percobaan ini adalah mengimplementasikan pengolahan data pada sisi komputer, mulai dari pengambilan data melalui MQTT, penyimpanan ke file CSV, analisis statistik, deteksi anomali dengan z-score, hingga visualisasi data. Sisi ESP mempublikasikan suhu dummy, sisi Python menerima dan menganalisisnya.

<details>
<summary>Lihat kode percobaan 5B (Arduino IDE)</summary>

```cpp
// Library dan konfigurasi
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// #include <DHT.h>

const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* topicData = "ahlele/ahlelas/dhtPalsu";   // harus sama dengan TOPIC di Python
// const char* topicPerintah = "ahlele/ahlelasPerintah";

// #define DHTPIN 4
// #define DHTTYPE DHT22

// const int ledPin = D4;

// DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000;   // publish tiap 5 detik

// Mock function: simulasi suhu (tren sinus + noise + lonjakan anomali tiap 25 pembacaan)
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;

  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);
  float noise = random(-5, 6) * 0.1;
  float suhu = suhuDasar + noise;

  if (jumlahBaca % 25 == 0) {
    suhu += 8.0;
  }

  return suhu;
}

// Callback penerima perintah (tidak digunakan di Percobaan 5B)
// void callback(char* topic, byte* payload, unsigned int length) {
//   String pesan;
//   for (unsigned int i = 0; i < length; i++)
//     pesan += (char)payload[i];
//
//   JsonDocument doc;
//   if (deserializeJson(doc, pesan)) return;
//
//   const char* perintah = doc["perintah"];
//   digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
//
//   Serial.print("Perintah diterima -> Aktuator: ");
//   Serial.println(perintah);
// }

// Koneksi WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  Serial.println("WiFi berhasil terhubung!");
}

// Koneksi ke broker MQTT (ulangi sampai berhasil)
void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      // client.subscribe(topicPerintah);
      Serial.println("Terhubung ke broker MQTT");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // pinMode(ledPin, OUTPUT);
  // dht.begin();

  hubungkanWiFi();

  client.setServer(mqttServer, mqttPort);
  // client.setCallback(callback);
}

void loop() {
  // Jaga koneksi MQTT tetap hidup
  if (!client.connected())
    hubungkanMQTT();

  client.loop();

  // Publish data suhu mock setiap 5 detik (non-blocking)
  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();

    // float suhu = dht.readTemperature();
    // if (!isnan(suhu)) {

    // int suhu = random(28, 31);
    float suhu = bacaSuhuMock();

    // Susun payload JSON: {"suhu":28.53}
    JsonDocument doc;
    doc["suhu"] = suhu;

    char buffer[128];
    serializeJson(doc, buffer);

    client.publish(topicData, buffer);

    Serial.print("Data terkirim: ");
    Serial.println(buffer);

    // }
  }
}
```
</details>

<details>
<summary>Lihat kode percobaan 5B (Python)</summary>

```python
import json
import time
import paho.mqtt.client as mqtt
import pandas as pd
import matplotlib.pyplot as plt


# Konfigurasi broker, topic, dan jumlah data yang dikumpulkan
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "ahlele/ahlelas/dhtPalsu" # = topicData ESP32
JUMLAH_SAMPEL = 30 # jumlah data


data = []  # penampung seluruh data suhu yang diterima


# Callback saat terhubung ke broker: langsung subscribe ke topic data ESP
def on_connect(client, userdata, flags, reason_code, properties):
    print("Terhubung ke broker, kode:", reason_code)
    client.subscribe(TOPIC)


# Callback saat pesan masuk: ambil nilai suhu dari JSON, catat bersama waktu terima,
# lalu putuskan koneksi jika jumlah data sudah cukup. Pesan rusak diabaikan lewat try-except
def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        suhu = float(payload["suhu"])
    except (ValueError, KeyError):
        print("Data tidak valid, diabaikan")
        return


    data.append({"waktu": time.strftime("%Y-%m-%d %H:%M:%S"), "suhu": suhu})
    print(f"[{len(data)}/{JUMLAH_SAMPEL}] suhu = {suhu}")


    if len(data) >= JUMLAH_SAMPEL:
        client.disconnect()


# --- Tahap 1: akuisisi data dari MQTT ---
# Membuat klien, memasang callback, lalu menunggu data sampai disconnect dipanggil
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_forever()


# --- Tahap 2: simpan ke CSV ---
# Mengubah list data menjadi DataFrame (kolom waktu dan suhu), lalu menyimpannya
df = pd.DataFrame(data)
df.to_csv("data_suhu_analisis.csv", index=False)
print("Data tersimpan di data_suhu_analisis.csv")


# --- Tahap 3: analisis statistik dan deteksi anomali (z-score) ---
# Hitung rata-rata dan simpangan baku, lalu z-score tiap data = (suhu - rata-rata) / std.
# Jika std = 0 (semua data sama), z-score diset 0 untuk menghindari pembagian dengan nol
rata2 = df["suhu"].mean()
std = df["suhu"].std()


if std == 0:
    df["zscore"] = 0.0
else:
    df["zscore"] = (df["suhu"] - rata2) / std


# Data dianggap anomali jika jaraknya lebih dari 2 simpangan baku dari rata-rata
df["anomali"] = df["zscore"].abs() > 2


# Tampilkan ringkasan statistik dan jumlah anomali
print(f"Rata-rata : {rata2:.2f}")
print(f"Std deviasi: {std:.2f}")
print(f"Minimum : {df['suhu'].min():.2f}")
print(f"Maksimum : {df['suhu'].max():.2f}")
print(f"Jumlah anomali: {df['anomali'].sum()}")


# --- Tahap 4: visualisasi ---
# Grafik garis suhu, titik merah untuk anomali, dan garis putus-putus untuk rata-rata
plt.figure(figsize=(10, 4))
plt.plot(df.index, df["suhu"], marker="o", label="Suhu")
anom = df[df["anomali"]]
plt.scatter(anom.index, anom["suhu"], color="red", s=100, label="Anomali", zorder=3)
plt.axhline(rata2, color="gray", linestyle="--", label="Rata-rata")
plt.xlabel("Urutan data")
plt.ylabel("Suhu (°C)")
plt.title("Data Suhu dan Deteksi Anomali")


# Cetak tabel lengkap (suhu, z-score, status anomali) dan simpan ke CSV terpisah
print(df.round(2).to_string())
df.to_csv("data_suhu_analisis_analisis.csv", index=False)


# Finalisasi grafik: legenda, simpan sebagai gambar, lalu tampilkan
plt.legend()
plt.tight_layout()
plt.savefig("grafik_suhu.png")
plt.show()
```
</details>


# Library

### Percobaan 5A

- Tidak ada library tambahan, hanya fungsi bawaan Arduino (`sin()`, `random()`, `isnan()`, `delay()`).
- <DHT.h>               : library sensor DHT, tidak digunakan karena sensor rusak (diganti `bacaSuhuMock()`).

### Percobaan 5B

- <ESP8266WiFi.h>       : library untuk berinteraksi dengan WiFi pada ESP8266.
- <PubSubClient.h>      : library untuk komunikasi menggunakan protokol MQTT.
- <ArduinoJson.h>       : library untuk mengolah data JSON.
- json                  : pustaka Python untuk mengubah teks JSON menjadi dictionary.
- time                  : pustaka Python untuk mencatat waktu penerimaan data.
- paho-mqtt             : pustaka Python untuk berlangganan (subscribe) data dari broker MQTT.
- pandas                : pustaka Python untuk mengelola data tabular (DataFrame) dan menghitung statistik.
- matplotlib            : pustaka Python untuk membuat grafik/visualisasi data.



# Pertanyaan praktikum

### 5.5.4 Pertanyaan Praktikum:
Modifikasi program agar ESP32 juga menghitung simpangan baku dari data pada buffer, dan berikan penjelasan di setiap baris kode yang ditambahkan dalam bentuk README.md

```cpp
const int N = 5;
float buffer[N];
int indeks = 0;
int jumlah = 0;

float suhuMin = 1000.0;
float suhuMax = -1000.0;

float bacaSuhuMock() {
    static unsigned long jumlahBaca = 0;
    jumlahBaca++;

    float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);

    float noise = random(-5, 6) * 0.1;

    float suhu = suhuDasar + noise;

    if (jumlahBaca % 25 == 0) {
        suhu += 8.0;
    }

    return suhu;
}

float hitungMovingAverage(float nilaiBaru) {
  buffer[indeks] = nilaiBaru;
  indeks = (indeks + 1) % N;
  if (jumlah < N) jumlah++;

  float total = 0;
  for (int i = 0; i < jumlah; i++) {
    total += buffer[i];
  }
  return total / jumlah;
}

// Fungsi baru: hitung simpangan baku (sampel) dari data yang ada di buffer
float hitungSimpanganBaku(float rataRata) {
  if (jumlah < 2) return 0.0;                    // Simpangan baku butuh minimal 2 data, jika kurang kembalikan 0

  float totalKuadrat = 0;                        // Penampung jumlah kuadrat selisih
  for (int i = 0; i < jumlah; i++) {             // Ulangi sebanyak data yang sudah terisi
    float selisih = buffer[i] - rataRata;        // Jarak tiap data terhadap rata-rata
    totalKuadrat += selisih * selisih;           // Kuadratkan selisih lalu jumlahkan
  }
  return sqrt(totalKuadrat / (jumlah - 1));      // Bagi (n-1) lalu akar kuadrat
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  float suhu =  bacaSuhuMock();

  if (isnan(suhu)) {
    Serial.println("Pembacaan gagal, data diabaikan");
  } else {
    float suhuFilter = hitungMovingAverage(suhu);
    float suhuStd = hitungSimpanganBaku(suhuFilter);   // Hitung simpangan baku buffer memakai rata-rata hasil filter

    if (suhu < suhuMin) suhuMin = suhu;
    if (suhu > suhuMax) suhuMax = suhu;

    Serial.print("Mentah: ");
    Serial.print(suhu);
    Serial.print(" | Filter: ");
    Serial.print(suhuFilter);
    Serial.print(" | Min: ");
    Serial.print(suhuMin);
    Serial.print(" | Max: ");
    Serial.print(suhuMax);         // Diganti dari println menjadi print agar baris belum pindah sebelum Std tercetak
    Serial.print(" | Std: ");      // Cetak label simpangan baku
    Serial.println(suhuStd);       // Cetak nilai simpangan baku dan pindah baris
  }

  delay(2000);
}
```
#### Penjelasan Modifikasi program yang ditambahkan :

1. baris dibawah mendeklarasikan fungsi baru yang menerima rata-rata buffer dan mengembalikan simpangan baku. Fungsi dipisah agar `loop()` tetap rapi.
```cpp
float hitungSimpanganBaku(float rataRata) {   // Fungsi baru penghitung simpangan baku
```
2. baris dibawah menjaga agar perhitungan tidak dilakukan saat data di buffer baru 0 atau 1 buah. Dengan 1 data, pembagian `(jumlah - 1)` menjadi pembagian dengan nol, sehingga fungsi langsung mengembalikan 0.
```cpp
if (jumlah < 2) return 0.0;   // Kembalikan 0 jika data kurang dari 2
```
3. baris dibawah menyiapkan variabel penampung jumlah kuadrat selisih setiap data terhadap rata-rata, dimulai dari 0.
```cpp
float totalKuadrat = 0;   // Penampung jumlah kuadrat selisih
```
4. baris dibawah melakukan perulangan hanya sebanyak `jumlah` (data yang sudah terisi), bukan `N`, agar elemen buffer yang belum terisi tidak ikut dihitung.
```cpp
for (int i = 0; i < jumlah; i++) {   // Ulangi sebanyak data yang sudah terisi
```
5. baris dibawah menghitung selisih tiap data terhadap rata-rata, lalu mengkuadratkannya dan menambahkannya ke `totalKuadrat`. Kuadrat dipakai agar selisih negatif dan positif tidak saling menghilangkan.
```cpp
float selisih = buffer[i] - rataRata;   // Jarak tiap data terhadap rata-rata
totalKuadrat += selisih * selisih;      // Kuadratkan selisih lalu jumlahkan
```
6. baris dibawah membagi jumlah kuadrat dengan `(jumlah - 1)` (simpangan baku sampel, sama seperti `.std()` pada pandas di Percobaan 5B), lalu mengambil akar kuadratnya dengan `sqrt()` sehingga satuannya kembali sama dengan suhu (°C).
```cpp
return sqrt(totalKuadrat / (jumlah - 1));   // Bagi (n-1) lalu akar kuadrat
```
7. baris dibawah (di `loop()`) memanggil fungsi tersebut setelah `hitungMovingAverage()`, dengan rata-rata hasil filter sebagai masukan. Karena `hitungMovingAverage()` sudah memperbarui buffer, simpangan baku dihitung dari data buffer yang terbaru.
```cpp
float suhuStd = hitungSimpanganBaku(suhuFilter);   // Hitung simpangan baku buffer memakai rata-rata hasil filter
```
8. baris dibawah menampilkan nilai simpangan baku di Serial Monitor sebagai tambahan pada baris keluaran sebelumnya. `Serial.println(suhuMax)` diganti menjadi `Serial.print(suhuMax)` agar baris tidak berpindah sebelum Std tercetak.
```cpp
Serial.print(suhuMax);         // Diganti dari println menjadi print agar baris belum pindah sebelum Std tercetak
Serial.print(" | Std: ");      // Cetak label simpangan baku
Serial.println(suhuStd);       // Cetak nilai simpangan baku dan pindah baris
```

#### potongan program dan penjelasannya :
```cpp
...
float hitungSimpanganBaku(float rataRata) {   // Fungsi baru penghitung simpangan baku
  if (jumlah < 2) return 0.0;                 // Kembalikan 0 jika data kurang dari 2
  float totalKuadrat = 0;                     // Penampung jumlah kuadrat selisih
  for (int i = 0; i < jumlah; i++) {          // Ulangi sebanyak data yang sudah terisi
    float selisih = buffer[i] - rataRata;     // Jarak tiap data terhadap rata-rata
    totalKuadrat += selisih * selisih;        // Kuadratkan selisih lalu jumlahkan
  }
  return sqrt(totalKuadrat / (jumlah - 1));   // Bagi (n-1) lalu akar kuadrat
}
...
```
> Program di atas menghitung seberapa menyebar 5 pembacaan terakhir di buffer terhadap rata-ratanya. Saat suhu stabil nilai Std kecil, sedangkan saat terjadi lonjakan (anomali mock setiap 25 pembacaan) nilai Std naik tajam selama lonjakan itu masih ada di dalam buffer.

### 5.6.4 Pertanyaan Praktikum:
Modifikasi program agar menggunakan metode moving average (jendela 5 data) pada data hasil CSV sebelum dihitung z-score-nya, kemudian bandingkan jumlah anomali yang terdeteksi, dan berikan penjelasan di setiap baris kode yang ditambahkan dalam bentuk README.md

```python
import json
import time
import paho.mqtt.client as mqtt
import pandas as pd
import matplotlib.pyplot as plt


BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "ahlele/ahlelas/dhtPalsu"
JUMLAH_SAMPEL = 30
JENDELA = 5  # Ukuran jendela moving average


data = []


def on_connect(client, userdata, flags, reason_code, properties):
    print("Terhubung ke broker, kode:", reason_code)
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        suhu = float(payload["suhu"])
    except (ValueError, KeyError):
        print("Data tidak valid, diabaikan")
        return


    data.append({"waktu": time.strftime("%Y-%m-%d %H:%M:%S"), "suhu": suhu})
    print(f"[{len(data)}/{JUMLAH_SAMPEL}] suhu = {suhu}")


    if len(data) >= JUMLAH_SAMPEL:
        client.disconnect()


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_forever()


df = pd.DataFrame(data)
df.to_csv("data_suhu_analisis.csv", index=False)
print("Data tersimpan di data_suhu_analisis.csv")


rata2 = df["suhu"].mean()
std = df["suhu"].std()


if std == 0:
    df["zscore"] = 0.0
else:
    df["zscore"] = (df["suhu"] - rata2) / std


df["anomali"] = df["zscore"].abs() > 2


print(f"Rata-rata : {rata2:.2f}")
print(f"Std deviasi: {std:.2f}")
print(f"Minimum : {df['suhu'].min():.2f}")
print(f"Maksimum : {df['suhu'].max():.2f}")
print(f"Jumlah anomali: {df['anomali'].sum()}")


df_ma = pd.read_csv("data_suhu_analisis.csv")  # Baca ulang data hasil CSV ke DataFrame baru
df_ma["suhu_ma"] = df_ma["suhu"].rolling(window=JENDELA, min_periods=1).mean()  # Moving average 5 data, 4 data pertama tetap dihitung dari data yang ada


rata2_ma = df_ma["suhu_ma"].mean()  # Rata-rata data yang sudah dihaluskan
std_ma = df_ma["suhu_ma"].std()  # Simpangan baku data yang sudah dihaluskan


if std_ma == 0:  # Hindari pembagian dengan nol
    df_ma["zscore_ma"] = 0.0
else:
    df_ma["zscore_ma"] = (df_ma["suhu_ma"] - rata2_ma) / std_ma  # Z-score dihitung dari data moving average


df_ma["anomali_ma"] = df_ma["zscore_ma"].abs() > 2  # Aturan anomali sama (|z| > 2) agar perbandingan adil


print(f"[MA] Rata-rata : {rata2_ma:.2f}")  # Statistik data moving average diberi awalan [MA]
print(f"[MA] Std deviasi: {std_ma:.2f}")
print(f"[MA] Minimum : {df_ma['suhu_ma'].min():.2f}")
print(f"[MA] Maksimum : {df_ma['suhu_ma'].max():.2f}")


jumlah_mentah = int(df["anomali"].sum())  # Jumlah anomali data mentah
jumlah_ma = int(df_ma["anomali_ma"].sum())  # Jumlah anomali data moving average
print("--- Perbandingan jumlah anomali ---")
print(f"Tanpa moving average   : {jumlah_mentah}")
print(f"Dengan moving average  : {jumlah_ma}")


df_ma["zscore_mentah"] = df["zscore"]  # Salin z-score data mentah agar bisa dibandingkan per baris
df_ma["anomali_mentah"] = df["anomali"]  # Salin status anomali data mentah
df_ma.to_csv("data_suhu_analisis_analisis_ma.csv", index=False)  # Simpan hasil perbandingan ke CSV terpisah


plt.figure(figsize=(10, 4))
plt.plot(df.index, df["suhu"], marker="o", label="Suhu")
anom = df[df["anomali"]]
plt.scatter(anom.index, anom["suhu"], color="red", s=100, label="Anomali", zorder=3)
plt.axhline(rata2, color="gray", linestyle="--", label="Rata-rata")
plt.xlabel("Urutan data")
plt.ylabel("Suhu (°C)")
plt.title("Data Suhu dan Deteksi Anomali")


print(df.round(2).to_string())
df.to_csv("data_suhu_analisis_analisis.csv", index=False)


plt.legend()
plt.tight_layout()
plt.savefig("grafik_suhu.png")  # plt.show() dihapus dari sini agar grafik kedua ikut dibuat sebelum ditampilkan


plt.figure(figsize=(10, 4))  # Grafik kedua: perbandingan data mentah dan moving average
plt.plot(df_ma.index, df_ma["suhu"], marker="o", alpha=0.4, label="Suhu mentah")  # Data mentah dibuat transparan
plt.plot(df_ma.index, df_ma["suhu_ma"], marker="o", label=f"Moving average ({JENDELA})")  # Data hasil moving average
anom_ma = df_ma[df_ma["anomali_ma"]]  # Ambil data yang terdeteksi anomali setelah moving average
plt.scatter(anom_ma.index, anom_ma["suhu_ma"], color="red", s=100, label="Anomali (MA)", zorder=3)  # Titik merah anomali MA
plt.axhline(rata2_ma, color="gray", linestyle="--", label="Rata-rata (MA)")  # Garis rata-rata data MA
plt.xlabel("Urutan data")
plt.ylabel("Suhu (°C)")
plt.title("Data Suhu Moving Average dan Deteksi Anomali")
plt.legend()
plt.tight_layout()
plt.savefig("grafik_suhu_ma.png")  # Simpan grafik kedua


plt.show()  # Tampilkan kedua grafik sekaligus
```
#### Penjelasan Modifikasi program yang ditambahkan :

1. baris dibawah mendefinisikan ukuran jendela moving average sebagai konstanta di bagian konfigurasi, sehingga mudah diganti (misalnya 3 atau 10) tanpa mengubah kode lain.
```python
JENDELA = 5  # Ukuran jendela moving average
```
2. baris dibawah membaca ulang file `data_suhu_analisis.csv` ke DataFrame baru `df_ma`, sesuai soal bahwa moving average diterapkan pada data hasil CSV.
```python
df_ma = pd.read_csv("data_suhu_analisis.csv")  # Baca ulang data hasil CSV ke DataFrame baru
```
3. baris dibawah menghitung moving average dengan jendela 5 data. `rolling(window=JENDELA)` mengambil 5 data terakhir, `.mean()` merata-ratakannya, dan `min_periods=1` membuat 4 data pertama tetap dihitung dari data yang tersedia (tidak menjadi NaN), sama seperti perilaku `hitungMovingAverage()` pada ESP.
```python
df_ma["suhu_ma"] = df_ma["suhu"].rolling(window=JENDELA, min_periods=1).mean()  # Moving average 5 data
```
4. baris dibawah menghitung rata-rata dan simpangan baku dari data yang sudah dihaluskan, bukan dari data mentah.
```python
rata2_ma = df_ma["suhu_ma"].mean()  # Rata-rata data yang sudah dihaluskan
std_ma = df_ma["suhu_ma"].std()  # Simpangan baku data yang sudah dihaluskan
```
5. baris dibawah menghitung z-score pada data moving average. Jika `std_ma` bernilai 0, z-score diset 0 untuk menghindari pembagian dengan nol, sama seperti pada data mentah.
```python
if std_ma == 0:  # Hindari pembagian dengan nol
    df_ma["zscore_ma"] = 0.0
else:
    df_ma["zscore_ma"] = (df_ma["suhu_ma"] - rata2_ma) / std_ma  # Z-score dihitung dari data moving average
```
6. baris dibawah menandai data sebagai anomali jika |z| lebih dari 2, dengan aturan yang sama seperti data mentah agar perbandingan adil.
```python
df_ma["anomali_ma"] = df_ma["zscore_ma"].abs() > 2  # Aturan anomali sama (|z| > 2) agar perbandingan adil
```
7. baris dibawah menampilkan statistik data hasil moving average dengan awalan `[MA]` supaya mudah dibedakan dari statistik data mentah.
```python
print(f"[MA] Rata-rata : {rata2_ma:.2f}")  # Statistik data moving average diberi awalan [MA]
print(f"[MA] Std deviasi: {std_ma:.2f}")
print(f"[MA] Minimum : {df_ma['suhu_ma'].min():.2f}")
print(f"[MA] Maksimum : {df_ma['suhu_ma'].max():.2f}")
```
8. baris dibawah menghitung dan mencetak jumlah anomali sebelum (data mentah) dan sesudah moving average sebagai bahan perbandingan. `int()` dipakai agar hasil penjumlahan `True/False` tercetak sebagai bilangan bulat.
```python
jumlah_mentah = int(df["anomali"].sum())  # Jumlah anomali data mentah
jumlah_ma = int(df_ma["anomali_ma"].sum())  # Jumlah anomali data moving average
print(f"Tanpa moving average   : {jumlah_mentah}")
print(f"Dengan moving average  : {jumlah_ma}")
```
9. baris dibawah menyalin kolom z-score dan anomali data mentah ke `df_ma`, lalu menyimpan keduanya dalam satu file CSV terpisah (`data_suhu_analisis_analisis_ma.csv`) agar hasil sebelum dan sesudah moving average dapat dibandingkan baris per baris.
```python
df_ma["zscore_mentah"] = df["zscore"]  # Salin z-score data mentah agar bisa dibandingkan per baris
df_ma["anomali_mentah"] = df["anomali"]  # Salin status anomali data mentah
df_ma.to_csv("data_suhu_analisis_analisis_ma.csv", index=False)  # Simpan hasil perbandingan ke CSV terpisah
```
10. baris dibawah (pada Tahap 4) menghapus `plt.show()` dari grafik pertama dan menggantinya dengan menyimpan gambar saja, supaya program tidak berhenti menunggu jendela grafik pertama ditutup sebelum grafik kedua dibuat.
```python
plt.savefig("grafik_suhu.png")  # plt.show() dihapus dari sini agar grafik kedua ikut dibuat sebelum ditampilkan
```
11. baris dibawah membuat grafik kedua yang menampilkan data mentah (transparan), data moving average, garis rata-rata MA, dan titik merah untuk anomali hasil MA, lalu menyimpannya sebagai `grafik_suhu_ma.png`.
```python
plt.figure(figsize=(10, 4))  # Grafik kedua: perbandingan data mentah dan moving average
plt.plot(df_ma.index, df_ma["suhu"], marker="o", alpha=0.4, label="Suhu mentah")  # Data mentah dibuat transparan
plt.plot(df_ma.index, df_ma["suhu_ma"], marker="o", label=f"Moving average ({JENDELA})")  # Data hasil moving average
anom_ma = df_ma[df_ma["anomali_ma"]]  # Ambil data yang terdeteksi anomali setelah moving average
plt.scatter(anom_ma.index, anom_ma["suhu_ma"], color="red", s=100, label="Anomali (MA)", zorder=3)  # Titik merah anomali MA
plt.axhline(rata2_ma, color="gray", linestyle="--", label="Rata-rata (MA)")  # Garis rata-rata data MA
plt.savefig("grafik_suhu_ma.png")  # Simpan grafik kedua
```
12. baris dibawah (di akhir program) menampilkan kedua grafik sekaligus.
```python
plt.show()  # Tampilkan kedua grafik sekaligus
```

#### potongan program dan penjelasannya :
```python
...
df_ma = pd.read_csv("data_suhu_analisis.csv")  # Baca ulang data hasil CSV ke DataFrame baru
df_ma["suhu_ma"] = df_ma["suhu"].rolling(window=JENDELA, min_periods=1).mean()  # Moving average 5 data
...
df_ma["anomali_ma"] = df_ma["zscore_ma"].abs() > 2  # Aturan anomali sama (|z| > 2) agar perbandingan adil
...
print(f"Tanpa moving average   : {jumlah_mentah}")  # Jumlah anomali sebelum moving average
print(f"Dengan moving average  : {jumlah_ma}")  # Jumlah anomali sesudah moving average
...
```
> Program di atas menghaluskan data suhu dengan moving average 5 data sebelum dihitung z-score-nya, lalu membandingkan jumlah anomali dengan data mentah. Lonjakan suhu sesaat (+8 °C dari mock) pada data mentah menyimpang jauh dari rata-rata sehingga terdeteksi sebagai anomali, sedangkan pada data moving average lonjakan itu "dibagi" ke 5 data sehingga nilainya jauh lebih kecil dan jumlah anomali cenderung berkurang (bahkan bisa menjadi 0). Isi hasil perbandingan sebenarnya dari percobaan: anomali tanpa moving average = ..., dengan moving average = ...

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
    <th colspan="2" align="center" style="text-align: center;">alat dan bahan modul 5</th>
  </tr>
  <tr>
    <td colspan="2" align="center">
      <img src="Dokumentasi/alat dan bahan5.jfif" class="frame">
    </td>
  </tr>
  <tr>
    <th style="text-align: center;">Dokumentasi Percobaan 5A</th>
    <th style="text-align: center;">Dokumentasi Percobaan 5B</th>
  </tr>
  <tr>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 5A.jfif" class="frame">
    </td>
    <td align="center">
      <img src="Dokumentasi/Dokumentasi percobaan 5B.jfif" class="frame">
    </td>
  </tr>
</table>