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
df.to_csv("data_suhu.csv", index=False)
print("Data tersimpan di data_suhu.csv")


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
df.to_csv("data_suhu_analisis.csv", index=False)


# Finalisasi grafik: legenda, simpan sebagai gambar, lalu tampilkan
plt.legend()
plt.tight_layout()
plt.savefig("grafik_suhu.png")
plt.show()


