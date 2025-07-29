import matplotlib.pyplot as plt
import numpy as np

# Plot the sine wave
data = np.loadtxt("sine_wave.csv", delimiter=",")
plt.figure(figsize=(10, 4))
plt.plot(data[:, 0], data[:, 1])
plt.title("Sine Wave (Time Domain)")
plt.xlabel("Sample")
plt.ylabel("Amplitude")
plt.grid(True)
plt.tight_layout()
plt.show()

# Plot the DFT magnitude
spectrum = np.loadtxt("dft_output.csv", delimiter=",")
plt.figure(figsize=(10, 4))
plt.plot(spectrum[:, 0], spectrum[:, 1])
plt.title("DFT Magnitude Spectrum (Frequency Domain)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.grid(True)
plt.tight_layout()
plt.show()
