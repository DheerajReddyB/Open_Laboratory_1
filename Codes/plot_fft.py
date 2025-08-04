import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# --- Load sine wave data ---
sine_data = np.loadtxt("sine_wave.csv", delimiter=",")
time = sine_data[:, 0]
amplitude = sine_data[:, 1]

plt.figure()
plt.plot(time, amplitude, color='b')
plt.title("Sine Wave")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.tight_layout()

# --- Load DFT data with headers ---
try:
    dft_df = pd.read_csv("fft_output.csv")  # Assumes headers are present
except Exception as e:
    print("Error loading FFT data:", e)
    exit()

# Plot Magnitude Spectrum
plt.figure()
plt.plot(dft_df["Frequency(Hz)"], dft_df["Magnitude"], color='r')
plt.title("FFT Magnitude Spectrum")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.tight_layout()

# Plot Phase Spectrum
plt.figure()
plt.plot(dft_df["Frequency(Hz)"], dft_df["Phase(rad)"], color='g')
plt.title("FFT Phase Spectrum")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Phase (radians)")
plt.grid(True)
plt.tight_layout()

plt.show()
