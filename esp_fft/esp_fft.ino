#include <arduinoFFT.h>

#define SAMPLES 1024               // Use 512 for faster refresh (1024 is heavier on ESP32)
#define SAMPLING_FREQUENCY 20000  // Hz
#define ADC_PIN 34                // GPIO34 (input from Red Pitaya)

double vReal[SAMPLES];
double vImag[SAMPLES];
double timeBuffer[SAMPLES];       // Store raw samples separately

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

unsigned int sampling_period_us;
unsigned long microseconds;

void setup() {
  Serial.begin(115200);
  sampling_period_us = round(1000000.0 / SAMPLING_FREQUENCY);
}

void loop() {
  // --- 1. Collect samples (time domain) ---
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    int adcValue = analogRead(ADC_PIN);

    timeBuffer[i] = (double)adcValue;  // Save original waveform
    vReal[i] = (double)adcValue;       // Copy to FFT buffer
    vImag[i] = 0;

    while (micros() - microseconds < sampling_period_us) {
      // Wait for precise sampling
    }
  }

  // --- 2. Run FFT ---
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // --- 3. Stream to Serial Plotter ---
  // Print: time-domain (raw ADC), frequency-domain (FFT)
  // Arduino Serial Plotter will plot both curves

  for (int i = 0; i < SAMPLES / 2; i++) {
    double timeSample = timeBuffer[i];   // time domain
    double freqSample = vReal[i];        // FFT magnitude

    Serial.print(timeSample);
    Serial.print(",");
    Serial.println(freqSample);
  }
}
