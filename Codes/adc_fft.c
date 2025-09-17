#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793f
#define MAX_SAMPLES 2048  // Upper bound for FFT size

// Buffers
float x[MAX_SAMPLES];
float X_re[MAX_SAMPLES];
float X_im[MAX_SAMPLES];
float Mag[MAX_SAMPLES];
float Phase[MAX_SAMPLES];

// --- Find next power of 2 ---
int next_power_of_two(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

// --- FFT ---
void compute_fft(int N) {
    for (int i = 0; i < N; i++) {
        X_re[i] = x[i];
        X_im[i] = 0.0f;
    }

    // Bit-reversal
    int j = 0;
    for (int i = 1; i < N; i++) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            float tr = X_re[i], ti = X_im[i];
            X_re[i] = X_re[j]; X_im[i] = X_im[j];
            X_re[j] = tr;      X_im[j] = ti;
        }
    }

    // FFT computation
    for (int len = 2; len <= N; len <<= 1) {
        float angle = -2.0f * PI / len;
        float wlen_re = cosf(angle);
        float wlen_im = sinf(angle);
        for (int i = 0; i < N; i += len) {
            float w_re = 1.0f, w_im = 0.0f;
            for (int j = 0; j < len/2; j++) {
                int u = i + j, v = i + j + len/2;
                float re_v = X_re[v] * w_re - X_im[v] * w_im;
                float im_v = X_re[v] * w_im + X_im[v] * w_re;
                float tr = X_re[u], ti = X_im[u];
                X_re[u] = tr + re_v;  X_im[u] = ti + im_v;
                X_re[v] = tr - re_v;  X_im[v] = ti - im_v;
                float next_w_re = w_re * wlen_re - w_im * wlen_im;
                float next_w_im = w_re * wlen_im + w_im * wlen_re;
                w_re = next_w_re; w_im = next_w_im;
            }
        }
    }

    for (int k = 0; k < N; k++) {
        float re = X_re[k], im = X_im[k];
        Mag[k] = sqrtf(re*re + im*im) / (N/2);
        Phase[k] = atan2f(im, re);
    }
}

int main(void) {
    FILE *fp = fopen("adc_read.csv", "r");
    if (!fp) { printf("Error opening adc_read.csv\n"); return 1; }

    int N_raw = 0;
    char line[128];

    // Skip header
    fgets(line, sizeof(line), fp);

    // Read "Index,ADC_Value"
    while (fgets(line, sizeof(line), fp) && N_raw < MAX_SAMPLES) {
        int idx;
        float val;
        if (sscanf(line, "%d,%f", &idx, &val) == 2) {
            x[N_raw++] = val;
        }
    }
    fclose(fp);

    printf("Read %d samples from adc_read.csv\n", N_raw);
    if (N_raw == 0) return 1;

    // Pad to next power of 2
    int N = next_power_of_two(N_raw);
    for (int i = N_raw; i < N; i++) x[i] = 0.0f;

    float Fs = 1000.0f; // adjust to your sampling rate
    compute_fft(N);

    FILE *fp_fft = fopen("adc_fft_output.csv", "w");
    fprintf(fp_fft, "Frequency(Hz),Magnitude,Phase(rad)\n");
    for (int k = 0; k < N/2; k++) {
        float freq = k * Fs / N;
        fprintf(fp_fft, "%.2f,%.4f,%.4f\n", freq, Mag[k], Phase[k]);
    }
    fclose(fp_fft);

    printf("Saved adc_fft_output.csv with %d bins\n", N/2);
    return 0;
}
