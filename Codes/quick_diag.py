#!/usr/bin/env python3
# diag_offset.py
import numpy as np, re, os, sys
import matplotlib.pyplot as plt

INFILE = "data.bin"
HAS_HEADER_FOOTER = True
HDR = b"CAPTURE_DONE\n"
END = b"\nCAPTURE_END\n"
# sampling period used in your firmware (DELAY_US(50))
SAMPLE_PERIOD_S = 50e-6

# expected signal (what you intended)
EXPECTED_OFFSET = 0.5   # V
EXPECTED_AMP = 0.3      # V
EXPECTED_PEAK = EXPECTED_OFFSET + EXPECTED_AMP

# assumed VREF used by your converter script (change if you know different)
ASSUME_VREF = 3.3
ADC_BITS = 12
FS_COUNTS = (1 << ADC_BITS) - 1

def load_strip(path):
    raw = open(path, "rb").read()
    if not HAS_HEADER_FOOTER: return raw
    s = raw.find(HDR)
    if s!=-1: s += len(HDR)
    else:
        m = re.search(b"CAPTURE[_ ]?DONE\\r?\\n", raw)
        s = m.end() if m else 0
    e = raw.rfind(END)
    if e == -1:
        m2 = re.search(b"\\nCAPTURE[_ ]?END\\r?\\n", raw)
        e = m2.start() if m2 else len(raw)
    return raw[s:e]

def to_uint16_le(b):
    if len(b)%2!=0: b=b[:-1]
    return np.frombuffer(b, dtype='<u2').astype(np.uint16)

payload = load_strip(INFILE)
arr16 = to_uint16_le(payload)
if arr16.size == 0:
    print("No samples found in", INFILE); sys.exit(1)

# use mask12 mapping (lower 12 bits)
counts = (arr16 & 0x0FFF).astype(np.float64)
n = counts.size
t = np.arange(n) * SAMPLE_PERIOD_S

# stats
min_c = counts.min(); max_c = counts.max(); mean_c = counts.mean()
min_v = min_c * (ASSUME_VREF / FS_COUNTS)
max_v = max_c * (ASSUME_VREF / FS_COUNTS)
mean_v = mean_c * (ASSUME_VREF / FS_COUNTS)
p2p_v = max_v - min_v
print(f"Samples: {n}")
print(f"Counts: min={min_c:.1f}, max={max_c:.1f}, mean={mean_c:.3f}, p2p={max_c-min_c:.1f}")
print(f"Volts (using VREF={ASSUME_VREF}): min={min_v:.6f} V, max={max_v:.6f} V, mean={mean_v:.6f} V, p2p={p2p_v:.6f} V")

# implied VREF to make observed peak counts become EXPECTED_PEAK volts:
# Vref_implied = EXPECTED_PEAK * FS_COUNTS / observed_peak_count
if max_c > 0:
    vref_implied = EXPECTED_PEAK * FS_COUNTS / max_c
    print(f"Implied VREF to make observed peak == expected peak ({EXPECTED_PEAK:.3f} V): {vref_implied:.6f} V")
else:
    print("max_c==0 cannot compute implied VREF")

# implied offset in volts under current ASSUME_VREF
print(f"Implied observed DC offset (using ASSUME_VREF): {mean_v:.6f} V")
print(f"Expected DC offset (what you asked for): {EXPECTED_OFFSET:.6f} V")

# quick plot with expected lines
plt.figure(figsize=(10,4))
plt.plot(t, counts * (ASSUME_VREF / FS_COUNTS), linewidth=0.6)
plt.scatter(t, counts * (ASSUME_VREF / FS_COUNTS), s=8)
plt.axhline(EXPECTED_OFFSET, color='red', linestyle='--', label='expected offset 0.5 V')
plt.axhline(EXPECTED_PEAK, color='green', linestyle='--', label='expected peak 0.8 V')
plt.xlabel("Time (s)")
plt.ylabel("Voltage (V)")
plt.title("Decoded waveform (mask12) — compare to expected lines")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
