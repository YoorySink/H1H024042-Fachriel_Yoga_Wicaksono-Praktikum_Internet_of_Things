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
