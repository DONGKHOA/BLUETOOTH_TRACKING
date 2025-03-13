import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# Load the data from a CSV file
file_path = "rssi_distance_data.csv"
data = pd.read_csv(file_path)

# Extract the distance and RSSI_NEW values from the data
distance = data['DISTANCE(m)'].values
rssi = data['RSSI'].values

# Define a modified path-loss model with fixed A = -69 (RSSI at 1m)
def path_loss_model(d, n):
    return -69 + 10 * n * np.log10(d)

# Fit the model to the data (ignoring the first point where distance is 0)
popt, _ = curve_fit(path_loss_model, distance[1:], rssi[1:])

# Extracting the fitted parameter n
n = popt[0]

# Generate fitted RSSI values
fitted_rssi = path_loss_model(distance, n)

# Plotting the results
plt.figure(figsize=(10, 6))
plt.scatter(distance, rssi, label='Measured RSSI', color='blue')
plt.plot(distance, fitted_rssi, label=f'Fitted Model: RSSI = -69 + 10 * {n:.2f} * log10(Distance)', color='red')
plt.title('Distance vs RSSI')
plt.xlabel('Distance (m)')
plt.ylabel('RSSI (dBm)')
plt.legend()
plt.grid(True)

# Save the plot to a file
plot_file_path = "distance_vs_rssi.png"
plt.savefig(plot_file_path, bbox_inches='tight', dpi=300)
plt.show()
