#include <arduinoFFT.h>

#define SAMPLES 1024               // Power of 2
#define SAMPLING_FREQUENCY 20000   // Hz
#define ADC_PIN 34                 // GPIO34 (input from Red Pitaya)
#define VREF 3.3                   // ESP32 ADC reference voltage
#define ADC_RES 4095.0             // 12-bit ADC

double vReal[SAMPLES];
double vImag[SAMPLES];
double timeBuffer[SAMPLES];       // Store raw samples (in volts)

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

    double voltage = (adcValue * VREF) / ADC_RES;  // Convert ADC counts → volts
    timeBuffer[i] = voltage;
    vReal[i] = voltage;
    vImag[i] = 0;

    while (micros() - microseconds < sampling_period_us) {
      // Wait for precise sampling
    }
  }

  // --- 2. Remove DC offset ---
  double mean = 0;
  for (int i = 0; i < SAMPLES; i++) mean += vReal[i];
  mean /= SAMPLES;
  for (int i = 0; i < SAMPLES; i++) vReal[i] -= mean;

  // --- 3. Run FFT ---
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // --- 4. Normalize FFT output to amplitude (Volts) ---
  // Coherent gain for Hamming window ≈ 0.54
  double CG = 0.54;
  for (int i = 0; i < SAMPLES / 2; i++) {
    vReal[i] = (vReal[i] * 2.0) / (SAMPLES * CG);  // Scale to amplitude spectrum
  }

  // --- 5. Print time-domain + FFT for Serial Plotter ---
  // Column 1: Time-domain (volts)
  // Column 2: FFT magnitude (volts)
  for (int i = 0; i < SAMPLES / 2; i++) {
    double timeSample = timeBuffer[i];   // waveform (V)
    double fftSample = vReal[i];         // spectrum (V)

    Serial.print(timeSample, 4);
    Serial.print(",");
    Serial.println(fftSample, 4);
  }
}
