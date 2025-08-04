#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793f
#define MAX_SAMPLES 2048  // Must be a power of 2 for FFT

// Signal and FFT buffers
float x[MAX_SAMPLES];       // Input signal
float X_re[MAX_SAMPLES];    // FFT real part
float X_im[MAX_SAMPLES];    // FFT imaginary part
float Mag[MAX_SAMPLES];     // Magnitude spectrum
float Phase[MAX_SAMPLES];   // Phase spectrum

// --- Check if N is a power of 2 ---
int is_power_of_two(int n) {
    return n && !(n & (n - 1));
}

// --- Generate Sine Wave ---
void generate_sine_wave(float amplitude, float freq, float Fs, int N) {
    for (int n = 0; n < N; n++) {
        x[n] = amplitude * sinf(2.0f * PI * freq * n / Fs);
    }
}

// --- Radix-2 In-Place FFT ---
void compute_fft(int N) {
    // Copy input to FFT buffers
    for (int i = 0; i < N; i++) {
        X_re[i] = x[i];
        X_im[i] = 0.0f;
    }

    // Bit-reversal permutation
    int j = 0;
    for (int i = 1; i < N; i++) {
        int bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            float temp_re = X_re[i];
            float temp_im = X_im[i];
            X_re[i] = X_re[j];
            X_im[i] = X_im[j];
            X_re[j] = temp_re;
            X_im[j] = temp_im;
        }
    }

    // FFT computation
    for (int len = 2; len <= N; len <<= 1) {
        float angle = -2.0f * PI / len;
        float wlen_re = cosf(angle);
        float wlen_im = sinf(angle);

        for (int i = 0; i < N; i += len) {
            float w_re = 1.0f;
            float w_im = 0.0f;

            for (int j = 0; j < len / 2; j++) {
                int u = i + j;
                int v = i + j + len / 2;

                float re_v = X_re[v] * w_re - X_im[v] * w_im;
                float im_v = X_re[v] * w_im + X_im[v] * w_re;

                float temp_re = X_re[u];
                float temp_im = X_im[u];

                X_re[u] = temp_re + re_v;
                X_im[u] = temp_im + im_v;

                X_re[v] = temp_re - re_v;
                X_im[v] = temp_im - im_v;

                // Update w
                float next_w_re = w_re * wlen_re - w_im * wlen_im;
                float next_w_im = w_re * wlen_im + w_im * wlen_re;
                w_re = next_w_re;
                w_im = next_w_im;
            }
        }
    }

    // Compute magnitude and phase
    for (int k = 0; k < N; k++) {
        float real = X_re[k];
        float imag = X_im[k];
        Mag[k] = sqrtf(real * real + imag * imag) / (N / 2);  // FIXED scaling
        Phase[k] = atan2f(imag, real);
    }
}

// --- Print FFT Bins (first few) ---
void print_fft_result(float Fs, int N, int max_bins) {
    printf("Bin\tFreq(Hz)\tMag\t\tPhase(rad)\n");
    for (int k = 0; k < max_bins && k < N / 2; k++) {
        float freq_bin = k * Fs / N;
        if (Mag[k] > 1e-4f) {  // Print only non-zero magnitude bins
            printf("%d\t%.2f\t\t%.4f\t\t%.4f\n", k, freq_bin, Mag[k], Phase[k]);
        }
    }
}

// --- Main ---
int main(void) {
    // Tunable Parameters
    float amplitude = 1.0f;
    float freq = 100.0f;
    float Fs = 10*freq;
    float duration = 1.0f;

    int N = 2048;
    if (N > MAX_SAMPLES) N = MAX_SAMPLES;
    if (!is_power_of_two(N)) {
        printf("Error: N = %d is not a power of 2\n", N);
        return 1;
    }

    // Generate signal and compute FFT
    generate_sine_wave(amplitude, freq, Fs, N);

    // Debug: Print first 10 samples
    printf("First 10 samples of x[n]:\n");
    for (int i = 0; i < 10; i++) {
        float t = i / Fs;
        printf("t=%.4f\tx=%.4f\n", t, x[i]);
    }

    compute_fft(N);
    print_fft_result(Fs, N, 50);

    // Save sine wave
    FILE *fp_signal = fopen("sine_wave.csv", "w");
    if (fp_signal != NULL) {
        for (int n = 0; n < N; n++) {
            float t = n / Fs;
            fprintf(fp_signal, "%.6f,%.4f\n", t, x[n]);
        }
        fclose(fp_signal);
        printf("Saved sine_wave.csv\n");
    }

    // Save FFT output
    FILE *fp_fft = fopen("fft_output.csv", "w");
    if (fp_fft != NULL) {
        fprintf(fp_fft, "Frequency(Hz),Magnitude,Phase(rad)\n");
        for (int k = 0; k < N / 2; k++) {
            float freq_bin = k * Fs / N;
            fprintf(fp_fft, "%.2f,%.4f,%.4f\n", freq_bin, Mag[k], Phase[k]);
        }
        fclose(fp_fft);
        printf("Saved fft_output.csv\n");
    }

    return 0;
}
