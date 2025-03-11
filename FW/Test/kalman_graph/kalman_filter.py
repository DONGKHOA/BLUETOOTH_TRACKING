import pandas as pd
import matplotlib.pyplot as plt

# Đọc file CSV
csv_file_path = "rssi_data.csv"
df = pd.read_csv(csv_file_path)

# Lấy dữ liệu từ DataFrame
time = df["Time (ms)"]
filter_values = df["Filtered RSSI"]
raw_values = df["Raw RSSI"]

# Vẽ biểu đồ
plt.figure(figsize=(12, 6))
plt.plot(time, filter_values, label="Filtered RSSI", color="red", linewidth=2, marker="o")
plt.plot(time, raw_values, label="Raw RSSI", color="blue", alpha=0.7, linewidth=1, marker="x")
plt.xlabel("Time (ms)")
plt.ylabel("RSSI (dBm)")
plt.title("Filtered vs Raw RSSI Values Over Time")
plt.legend()
plt.grid(True)
plt.savefig("Filtered_vs_Raw_RSSI_Timeline.png", dpi=300)
plt.show()
