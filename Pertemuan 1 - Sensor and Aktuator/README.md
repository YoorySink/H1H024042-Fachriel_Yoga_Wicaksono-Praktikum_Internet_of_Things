# Pertanyaan praktikum
---
### 1.5.4 Pertanyaan Praktikum: 
1) Gambarkan diagram alur (flowchart) proses akuisisi data sensor DHT22 pada program 
di atas! 
<img width="419" height="500" alt="image" src="https://github.com/user-attachments/assets/7a01a790-97ed-44b7-970f-807811862a50" />


2) Apa fungsi dari perintah isnan() pada program tersebut? 
> isnan() memeriksa apakah nilai yang dibaca dari sensor bernilai NaN (Not a Number) artinya pembacaan gagal/tidak valid (misal karena gangguan komunikasi dengan sensor). Jika hasilnya NaN, program tidak menampilkan data yang salah, melainkan mencetak pesan error. 

3) Jelaskan mengapa diperlukan jeda (delay) minimal sekitar 2 detik antar pembacaan 
sensor DHT22! 
> DHT22 punya keterbatasan kecepatan sampling internal sensor ini butuh waktu untuk menyelesaikan satu siklus pengukuran dan komunikasi data sebelum siap dibaca lagi. Jika dibaca terlalu cepat (kurang dari 2 detik), hasil pembacaan bisa tidak stabil, gagal, atau menghasilkan nilai NaN karena sensor belum selesai memperbarui datanya.

4) Modifikasi program agar data suhu dan kelembaban dirata-ratakan dari 5 kali 
pembacaan sebelum ditampilkan, dan berikan penjelasan di setiap baris kode yang 
ditambahkan dalam bentuk README.md!
```c
#include <DHT.h>              // Memasukkan library DHT agar ESP dapat menggunakan fungsi-fungsi untuk membaca sensor DHT22

#define DHTPIN 4               // Menentukan GPIO 4 ESP sebagai pin yang digunakan untuk menerima data dari DHT22
#define DHTTYPE DHT22          // Menentukan jenis sensor yang digunakan adalah DHT22

DHT dht(DHTPIN, DHTTYPE);      // Membuat objek dht dengan konfigurasi pin GPIO 4 dan tipe sensor DHT22

const int jumlahSampel = 5;    // Menentukan berapa kali pembacaan akan dirata-ratakan sebelum ditampilkan

void setup() {
  Serial.begin(115200);        // Mengaktifkan komunikasi serial dengan baud rate 115200
  dht.begin();                 // Melakukan inisialisasi sensor DHT22 sebelum digunakan
  Serial.println("Memulai akuisisi data sensor DHT22...");
}

void loop() {
  float totalSuhu = 0;              // Variabel penampung jumlah seluruh nilai suhu dari 5 pembacaan
  float totalKelembaban = 0;        // Variabel penampung jumlah seluruh nilai kelembaban dari 5 pembacaan
  int   sampelValid = 0;            // Menghitung berapa banyak pembacaan yang berhasil (tidak NaN)

  for (int i = 0; i < jumlahSampel; i++) {
    // Perulangan sebanyak 5 kali untuk mengambil beberapa sampel data

    float kelembaban = dht.readHumidity();     // Membaca kelembaban pada iterasi ke-i
    float suhu       = dht.readTemperature();  // Membaca suhu pada iterasi ke-i

    if (!isnan(kelembaban) && !isnan(suhu)) {
      // Hanya menjumlahkan data yang valid (bukan NaN) agar rata-rata tidak rusak oleh data error
      totalSuhu       += suhu;         // Menambahkan nilai suhu ke total
      totalKelembaban += kelembaban;   // Menambahkan nilai kelembaban ke total
      sampelValid++;                   // Menambah jumlah sampel yang berhasil dibaca
    } else {
      Serial.println("Satu pembacaan gagal, dilewati.");  // Memberi tahu jika ada pembacaan yang gagal
    }

    delay(2000);   // Jeda 2 detik antar pembacaan agar sensor sempat memperbarui datanya
  }

  if (sampelValid > 0) {
    // Jika ada minimal satu data valid, hitung rata-ratanya
    float rataSuhu       = totalSuhu / sampelValid;         // Rata-rata suhu = total suhu dibagi jumlah sampel valid
    float rataKelembaban = totalKelembaban / sampelValid;   // Rata-rata kelembaban = total kelembaban dibagi jumlah sampel valid

    Serial.print("Rata-rata Suhu: ");
    Serial.print(rataSuhu);                 // Menampilkan hasil rata-rata suhu
    Serial.print(" °C, Rata-rata Kelembaban: ");
    Serial.print(rataKelembaban);           // Menampilkan hasil rata-rata kelembaban
    Serial.println(" %");
  } else {
    // Jika seluruh 5 pembacaan gagal, tidak ada data untuk dirata-ratakan
    Serial.println("Semua pembacaan gagal, data rata-rata tidak dapat dihitung.");
  }
}
```

### 1.6.4 Pertanyaan praktikum
1. Mengapa diperlukan nilai ambang batas (threshold) dalam sistem kendali aktuator 
berbasis sensor? 
> Threshold diperlukan sebagai patokan otomatis bagi sistem untuk memutuskan kapan aktuator harus ON/OFF tanpa campur tangan kita (manusia). 

2. Jelaskan apa yang akan terjadi apabila nilai suhuThreshold diturunkan menjadi sangat 
rendah, misalnya 20.0! 
> Jika threshold diturunkan jadi 20°C, aktuator akan hampir selalu ON karena suhu ruangan normal biasanya sudah di atas 20°C sistem kehilangan fungsi kendalinya dan boros energi/mempercepat keausan komponen. 

3. Apa perbedaan antara kendali aktuator secara terus-menerus (kondisi tunggal) dengan 
kendali menggunakan histerisis (dua ambang batas)? 
> Kondisi tunggal rawan chattering (ON/OFF cepat berulang saat suhu naik-turun tipis di sekitar threshold), sedangkan histerisis punya zona aman di antara dua batas sehingga status lebih stabil dan aktuator tidak sering berubah. 

4. Modifikasi program agar menggunakan dua ambang batas (histerisis), misalnya aktuator 
menyala pada suhu di atas 30°C dan baru mati pada suhu di bawah 28°C, dan berikan 
penjelasan di setiap baris kode nya dalam bentuk README.md!

```c
#include <DHT.h>              // Memasukkan library DHT agar ESP dapat menggunakan fungsi-fungsi untuk membaca sensor DHT22

#define DHTPIN 4               // Menentukan GPIO 4 ESP sebagai pin yang digunakan untuk menerima data dari DHT22
#define DHTTYPE DHT22          // Menentukan jenis sensor yang digunakan adalah DHT22
#define RELAYPIN 26            // Menentukan GPIO 26 sebagai pin kendali relay/LED indikator

DHT dht(DHTPIN, DHTTYPE);      // Membuat objek dht dengan konfigurasi pin GPIO 4 dan tipe sensor DHT22

const float suhuAtas  = 30.0;  // Ambang batas atas: suhu di atas nilai ini akan menyalakan aktuator
const float suhuBawah = 28.0;  // Ambang batas bawah: suhu di bawah nilai ini akan mematikan aktuator

bool statusAktuator = false;   // Variabel penyimpan status aktuator saat ini (false = OFF, true = ON)
                                // Variabel ini diperlukan agar sistem "mengingat" kondisi sebelumnya
void setup() {
  Serial.begin(115200);        // Mengaktifkan komunikasi serial dengan baud rate 115200 sehingga data dapat ditampilkan pada Serial Monitor
  dht.begin();                 // Melakukan inisialisasi sensor DHT22 sebelum digunakan
  pinMode(RELAYPIN, OUTPUT);   // Mengatur GPIO 26 sebagai pin keluaran karena digunakan untuk memberikan sinyal kepada aktuator
  digitalWrite(RELAYPIN, LOW); // Memastikan aktuator berada dalam kondisi mati ketika sistem pertama kali dijalankan
}

void loop() {
  float suhu = dht.readTemperature();   // Membaca suhu dari sensor DHT22

  if (isnan(suhu)) {
    // Memeriksa apakah hasil pembacaan sensor menghasilkan nilai NaN (Not a Number)
    Serial.println("Gagal membaca data sensor!");
  } else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C -> ");

    // Logika histerisis: aktuator hanya berubah status pada dua kondisi berikut
    if (!statusAktuator && suhu > suhuAtas) {
      // Jika aktuator sedang OFF dan suhu melewati batas atas (30°C), maka nyalakan
      statusAktuator = true;            // Perbarui status menjadi ON
      digitalWrite(RELAYPIN, HIGH);     // Kirim sinyal HIGH untuk menyalakan relay/LED
    } 
    else if (statusAktuator && suhu < suhuBawah) {
      // Jika aktuator sedang ON dan suhu turun di bawah batas bawah (28°C), maka matikan
      statusAktuator = false;           // Perbarui status menjadi OFF
      digitalWrite(RELAYPIN, LOW);      // Kirim sinyal LOW untuk mematikan relay/LED
    }
    // Jika suhu berada di antara 28°C dan 30°C, tidak ada perintah digitalWrite baru
    // sehingga status aktuator tetap sama seperti sebelumnya (inilah efek histerisis)

    // Menampilkan status aktuator saat ini ke Serial Monitor
    Serial.println(statusAktuator ? "Aktuator: ON" : "Aktuator: OFF");
  }

  delay(2000);   // Memberikan jeda selama 2000 ms (2 detik) sebelum melakukan pembacaan berikutnya
}
```
# Dokumentasi
---
<p align="center">
  <img src="<img width="1280" height="960" alt="66f6b013-deb4-4625-b0a6-97e960a68ac1" src="https://github.com/user-attachments/assets/974860e5-af63-40fe-8414-c7dce68c8582" />
" width="350">
  <img src="<img width="1280" height="960" alt="96e78248-326d-45eb-95d3-88c311b193f0" src="https://github.com/user-attachments/assets/99e85b0c-09bf-42f0-af12-b0237869bb26" />
" width="350">
</p>
