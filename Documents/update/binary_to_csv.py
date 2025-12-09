# bin_to_csv_with_time.py
# Usage:
#  python bin_to_csv_with_time.py
#
# Edit the top-section parameters if needed:
#  - INFILE : path to your binary capture (data.bin)
#  - OUTCSV : output CSV filename
#  - HAS_HEADER_FOOTER : True if the binary file contains ASCII "CAPTURE_DONE" / "CAPTURE_END"
#  - USE_EPWM_RATE : if True will compute sample_rate from SYSCLK and TBPRD (see below)
#  - If not using ePWM (software forcing with DELAY_US), set USE_EPWM_RATE=False and set SAMPLE_PERIOD_S manually.
#
# By default this assumes:
#  SYSCLK = 200e6 (200 MHz)
#  TBCLK = SYSCLK / 4  (typical when you used /4 prescale)
#  TBPRD = 99  -> sample_rate = TBCLK / (TBPRD + 1) = 500000 Hz
#
# The script will:
#  - strip ASCII header/footer if present
#  - decode little-endian uint16 samples
#  - create time column starting at 0 with step = 1/sample_rate (seconds)
#  - save CSV and show a quick plot

import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import re

# ---------- EDIT THESE ----------
INFILE = "data.bin"
OUTCSV = "samples_with_time.csv"

HAS_HEADER_FOOTER = True   # True if file contains "CAPTURE_DONE" and "CAPTURE_END" ascii markers
USE_EPWM_RATE = True       # True => compute from SYSCLK/TBPRD; False => use SAMPLE_PERIOD_S
SYSCLK = 200e6             # CPU SYSCLK (Hz). Default 200 MHz typical for LaunchPad examples.
TBCLK_DIV = 4              # effective TBCLK divider (SYSCLK/TBCLK_DIV)
TBPRD = 99                 # your EPWM_PERIOD_TICKS (TBPRD). Default 99 -> 500kS/s
# If you forced samples with DELAY_US in code (software trigger), set USE_EPWM_RATE = False
# and set SAMPLE_PERIOD_S to the effective period between samples (in seconds).
SAMPLE_PERIOD_S = 50e-6    # e.g., DELAY_US(50) -> 50 microseconds between forced conversions
# --------------------------------

def load_and_strip(path, has_header_footer=True):
    with open(path, "rb") as f:
        raw = f.read()
    if not has_header_footer:
        return raw
    # find header and footer markers (ASCII)
    start = raw.find(b"CAPTURE_DONE\n")
    if start >= 0:
        start += len(b"CAPTURE_DONE\n")
    else:
        # try alternative header patterns
        m = re.search(b"CAPTURE[_ ]?DONE\\r?\\n", raw)
        if m:
            start = m.end()
        else:
            start = 0
    end = raw.rfind(b"\nCAPTURE_END\n")
    if end == -1:
        # try alternative
        m2 = re.search(b"\\nCAPTURE[_ ]?END\\r?\\n", raw)
        end = m2.start() if m2 else len(raw)
    return raw[start:end]

def bytes_to_uint16_le(payload):
    # If odd length, drop last byte
    if len(payload) % 2 != 0:
        payload = payload[:-1]
    arr = np.frombuffer(payload, dtype=np.uint16)  # default little-endian on x86
    return arr

def compute_sample_rate_from_epwm(sysclk_hz, tbclk_div, tbprd):
    tbclk = sysclk_hz / float(tbclk_div)
    sr = tbclk / (tbprd + 1.0)
    return sr

def main():
    if not os.path.exists(INFILE):
        print("Input file not found:", INFILE)
        sys.exit(1)

    raw_payload = load_and_strip(INFILE, HAS_HEADER_FOOTER)
    samples = bytes_to_uint16_le(raw_payload)
    n = samples.size
    print(f"Decoded {n} samples (uint16).")

    if n == 0:
        print("No samples found. Check INFILE / header/footer settings.")
        sys.exit(1)

    if USE_EPWM_RATE:
        sample_rate = compute_sample_rate_from_epwm(SYSCLK, TBCLK_DIV, TBPRD)
        print(f"Using ePWM-derived sample rate: {sample_rate:.3f} Hz (SYSCLK={SYSCLK}, TBCLK_DIV={TBCLK_DIV}, TBPRD={TBPRD})")
        sample_period = 1.0 / sample_rate
    else:
        sample_period = SAMPLE_PERIOD_S
        sample_rate = 1.0 / sample_period
        print(f"Using manual sample period: {sample_period:.6e} s -> sample_rate={sample_rate:.3f} Hz")

    # Build time vector (seconds)
    t = np.arange(n) * sample_period

    # Save CSV (time,sample)
    data2save = np.column_stack((t, samples))
    header = "time_seconds,sample"
    np.savetxt(OUTCSV, data2save, delimiter=",", header=header, fmt="%.9e,%u", comments="")
    print(f"Wrote {OUTCSV} with {n} rows.")

       # -----------------------------
    # Combined LINE + POINT plot
    # -----------------------------
    plt.figure(figsize=(12,5))

    # Line plot
    plt.plot(t, samples, linewidth=0.8, label="Line")

    # Points plotted on top
    plt.scatter(t, samples, s=15, color="red", label="Points")

    plt.xlabel("Time (s)")
    plt.ylabel("ADC sample (uint16)")
    plt.title(f"ADC capture - Line + Points ({n} samples, fs={sample_rate:.0f} Hz)")

    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()




if __name__ == "__main__":
    main()
