#include <math.h>
#include <stdio.h>

#define PI 3.141592653589793f
#define MAX_SAMPLES 1024  // Limit for DSP memory

// Static global buffers
float x[MAX_SAMPLES];       // Input signal
float Re[MAX_SAMPLES];      // DFT real part
float Im[MAX_SAMPLES];      // DFT imaginary part
float Mag[MAX_SAMPLES];     // Magnitude spectrum (actual amplitude)
float Phase[MAX_SAMPLES];   // Phase spectrum (radians)

// Function: Generate a sine wave
void generate_sine_wave(float amplitude, float freq, float Fs, int N) {
    for (int n = 0; n < N; n++) {
        x[n] = amplitude * sinf(2.0f * PI * freq * n / Fs);
    }
}

// Function: Compute DFT
void compute_dft(int N) {
    for (int k = 0; k < N; k++) {
        Re[k] = 0.0f;
        Im[k] = 0.0f;
        for (int n = 0; n < N; n++) {
            float angle = 2.0f * PI * k * n / N;
            Re[k] += x[n] * cosf(angle);
            Im[k] -= x[n] * sinf(angle);
        }
        // Scale to get actual amplitude (not raw FFT magnitude)
        Mag[k] = (2.0f / N) * sqrtf(Re[k] * Re[k] + Im[k] * Im[k]);
        Phase[k] = atan2f(Im[k], Re[k]);  // Phase in radians
    }
}

// Function: Optional debug output
void print_dft_result(float Fs, int N, int max_bins) {
    printf("Bin\tFreq(Hz)\tMag\t\tPhase(rad)\n");
    for (int k = 0; k < max_bins && k < N / 2; k++) {
        float freq_bin = k * Fs / N;
        printf("%d\t%.2f\t\t%.4f\t\t%.4f\n", k, freq_bin, Mag[k], Phase[k]);
    }
}

// Main function
int main(void) {
    // --- Tunable Parameters ---
    float amplitude = 1.0f;     // Sine wave amplitude
    float freq = 100.0f;        // Sine wave frequency (Hz)
    float Fs = 1000.0f;         // Sampling rate (Hz)
    float duration = 1.0f;      // Signal duration (s)

    int N = (int)(Fs * duration); // Number of samples
    if (N > MAX_SAMPLES) N = MAX_SAMPLES;

    // --- Signal Generation & DFT ---
    generate_sine_wave(amplitude, freq, Fs, N);
    compute_dft(N);

    // --- Debug: Print DFT bins ---
    print_dft_result(Fs, N, 200);

    // --- Output: Save sine wave (Time vs Amplitude) ---
    FILE *fp_signal = fopen("sine_wave.csv", "w");
    if (fp_signal != NULL) {
        for (int n = 0; n < N; n++) {
            float time = n / Fs;
            fprintf(fp_signal, "%.6f,%.4f\n", time, x[n]);
        }
        fclose(fp_signal);
        printf("Saved sine_wave.csv (Time,Amplitude)\n");
    }

    // --- Output: Save DFT result (Frequency vs Magnitude and Phase) ---
    FILE *fp_dft = fopen("dft_output.csv", "w");
    if (fp_dft != NULL) {
        fprintf(fp_dft, "Frequency(Hz),Magnitude,Phase(rad)\n");
        for (int k = 0; k < N / 2; k++) {  // Only up to Nyquist
            float freq_bin = k * Fs / N;
            fprintf(fp_dft, "%.2f,%.4f,%.4f\n", freq_bin, Mag[k], Phase[k]);
        }
        fclose(fp_dft);
        printf("Saved dft_output.csv (Frequency,Magnitude,Phase)\n");
    }

    return 0;
}
