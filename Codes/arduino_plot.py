import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# --- Load your raw time-domain samples (NOT FFT values) ---
data = pd.read_csv("D:\SEM-5\Open_Laboratory_1\Codes\Putty outputs\putty_fft5.csv", header=None)

# Convert to numpy float array
x = data[0].astype(float).values   # now it's numeric

Fs = 20000  # your sampling frequency (set to your actual value!)
N = len(x)

# --- Apply windowing ---
window = np.hanning(N)
xw = x * window

# --- FFT ---
X = np.fft.rfft(xw)
freqs = np.fft.rfftfreq(N, 1/Fs)
mag = np.abs(X) / (N/5)

# --- Plot time domain (first 500 samples) ---
plt.figure(figsize=(10,4))
plt.plot(x[:500])
plt.title("Time-domain Signal (first 500 samples)")
plt.xlabel("Sample Index")
plt.ylabel("Amplitude")
plt.grid(True)
plt.tight_layout()
plt.show()

# --- Plot frequency domain ---
plt.figure(figsize=(10,4))
plt.plot(freqs, mag, 'r')
plt.title("FFT Spectrum (Hanning Window)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.xlim(0, Fs/2)
plt.grid(True)
plt.tight_layout()
plt.show()
