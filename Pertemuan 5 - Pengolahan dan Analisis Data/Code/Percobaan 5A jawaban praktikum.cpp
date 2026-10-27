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
