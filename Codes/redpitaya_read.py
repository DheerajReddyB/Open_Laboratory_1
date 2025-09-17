import redpitaya_scpi as scpi
import numpy as np

# Connect to Red Pitaya (replace with your RP IP)
rp = scpi.scpi("192.168.1.100")

# Stop, reset, and start acquisition
rp.tx_txt("ACQ:RESET")
rp.tx_txt("ACQ:START")
rp.tx_txt("ACQ:TRIG NOW")

# Wait for data to be ready
rp.tx_txt("ACQ:TRIG:STAT?")
while rp.rx_txt() != "TD":
    rp.tx_txt("ACQ:TRIG:STAT?")

# Read channel 1 data
rp.tx_txt("ACQ:SOUR1:DATA?")
data_str = rp.rx_txt()
data = np.fromstring(data_str.strip('{}\n\r').replace('  ', ''), sep=',')

# Save to CSV
np.savetxt("adc_data.csv", data, delimiter=",")
print("Saved ADC data to adc_data.csv")

