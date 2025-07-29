#include <math.h>
#include <stdio.h>
#define PI 3.141592653589793f
#define MAX_SAMPLES 1024  // Adjust this based on your DSP memory

// Static global buffers
float x[MAX_SAMPLES];     // Input signal
float Re[MAX_SAMPLES];    // DFT real part
float Im[MAX_SAMPLES];    // DFT imaginary part
float Mag[MAX_SAMPLES];   // Magnitude spectrum

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
        Mag[k] = sqrtf(Re[k] * Re[k] + Im[k] * Im[k]);
    }
}

// Function: Optional display (for PC debug only)
// Comment out this function when running on the DSP
void print_dft_result(float Fs, int N, int max_bins) {
    for (int k = 0; k < max_bins && k < N / 2; k++) {
        float freq_bin = k * Fs / N;
        printf("Bin %d\tFreq: %.2f Hz\tMag: %.4f\n", k, freq_bin, Mag[k]);
    }
}

// Main function (DSP main loop entry point)
int main(void) {
    // ----- Tunable Parameters -----
    float amplitude = 1.0f;     // Amplitude of sine wave
    float freq = 100.0f;        // Frequency of sine wave (Hz)
    float Fs = 1000.0f;         // Sampling rate (Hz)
    float duration = 1.0f;      // Duration (seconds)
    int N = (int)(Fs * duration);  // Total number of samples

    // Limit to MAX_SAMPLES
    if (N > MAX_SAMPLES) {
        N = MAX_SAMPLES;
    }

    // ----- Processing -----
    generate_sine_wave(amplitude, freq, Fs, N);
    compute_dft(N);

    // ----- Debug output (for PC only) -----
    // You should comment/remove this block when running on DSP
    print_dft_result(Fs, N, 200);

    // ----- On DSP, you can send `Mag[]` via UART or store in shared memory -----
    FILE *fp_signal = fopen("sine_wave.csv", "w");
    for (int n = 0; n < N; n++) {
        fprintf(fp_signal, "%d,%.4f\n", n, x[n]);
    }
    fclose(fp_signal);
    printf("Sine wave data saved to sine_wave.csv\n");

    FILE *fp_dft = fopen("dft_output.csv", "w");
    for (int k = 0; k < N / 2; k++) {  // Only up to Nyquist
        float freq_bin = k * Fs / N;
        fprintf(fp_dft, "%.2f,%.4f\n", freq_bin, Mag[k]);
    }
    fclose(fp_dft);
    printf("DFT output saved to dft_output.csv\n");

    return 0;
}
