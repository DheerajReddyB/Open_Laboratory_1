# serial_capture_safe.py
import serial
import time

PORT = "COM1"            # change your COM port
BAUD = 115200
OUTFILE = "data.bin"
TIMEOUT = 5

end_marker = b"\nCAPTURE_END\n"

with serial.Serial(PORT, BAUD, timeout=TIMEOUT) as s, open(OUTFILE, "wb") as f:
    print("Waiting for data...")

    buffer = bytearray()

    while True:
        chunk = s.read(256)
        if chunk:
            buffer.extend(chunk)
            # stop when CAPTURE_END appears
            if end_marker in buffer:
                print("Found CAPTURE_END.")
                break
        else:
            break

    f.write(buffer)
    print("Saved", len(buffer), "bytes to data.bin")
