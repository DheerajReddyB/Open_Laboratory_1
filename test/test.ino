#include <arduinoFFT.h>

#define SAMPLES 1024              // More samples = better frequency resolution
#define SAMPLING_FREQUENCY 20000  // Hz
#define ADC_PIN 34                // GPIO34

double vReal[SAMPLES];
double vImag[SAMPLES];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

unsigned int sampling_period_us;
unsigned long microseconds;

void setup() {
  Serial.begin(115200);
  sampling_period_us = round(1000000.0 / SAMPLING_FREQUENCY);
}

void loop() {
  // --- 1. Collect samples ---
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    int adcValue = analogRead(ADC_PIN);
    vReal[i] = (double)adcValue;
    vImag[i] = 0;
    while (micros() - microseconds < sampling_period_us) {
      // Wait
    }
  }

  // --- 2. Run FFT ---
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  // --- 3. Print FFT only (frequency vs magnitude) ---
  for (int i = 1; i < (SAMPLES / 2); i++) {  // ignore DC at index 0
    double freq = (i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES;
    double magnitude = vReal[i];

    Serial.print(freq);      // X-axis = frequency in Hz
    Serial.print(",");
    Serial.println(magnitude);  // Y-axis = FFT magnitude
  }
}