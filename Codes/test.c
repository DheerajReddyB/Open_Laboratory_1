#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793

// Generate sine wave
void generate_sine_wave(double* x, int N, double amplitude, double freq, double Fs) {
    for (int n = 0; n < N; n++) {
        x[n] = amplitude * sin(2.0 * PI * freq * n / Fs);
    }
}

// Perform DFT
void compute_dft(double* x, int N, double* Re, double* Im, double* Mag) {
    for (int k = 0; k < N; k++) {
        Re[k] = 0;
        Im[k] = 0;
        for (int n = 0; n < N; n++) {
            double angle = 2.0 * PI * k * n / N;
            Re[k] += x[n] * cos(angle);
            Im[k] -= x[n] * sin(angle);
        }
        Mag[k] = sqrt(Re[k] * Re[k] + Im[k] * Im[k]);
    }
}

int main() {
    double amplitude, freq, Fs, duration;
    int num_bins_to_display = 200;

    // --- User Input ---
    printf("Enter signal amplitude: ");
    scanf("%lf", &amplitude);

    printf("Enter signal frequency (Hz): ");
    scanf("%lf", &freq);

    printf("Enter sampling rate (Hz): ");
    scanf("%lf", &Fs);

    printf("Enter signal duration (seconds): ");
    scanf("%lf", &duration);

    int N = (int)(Fs * duration); // Number of samples

    if (N <= 0) {
        printf("Invalid number of samples. Check your input.\n");
        return 1;
    }

    // Allocate memory
    double* x = (double*)malloc(N * sizeof(double));
    double* Re = (double*)malloc(N * sizeof(double));
    double* Im = (double*)malloc(N * sizeof(double));
    double* Mag = (double*)malloc(N * sizeof(double));

    if (!x || !Re || !Im || !Mag) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Generate signal
    generate_sine_wave(x, N, amplitude, freq, Fs);

    // Print first 10 samples
    printf("\n--- First 10 samples of generated sine wave ---\n");
    for (int i = 0; i < 10 && i < N; i++) {
        printf("x[%d] = %.4f\n", i, x[i]);
    }

    // Compute DFT
    compute_dft(x, N, Re, Im, Mag);

    // Output DFT result
    printf("\n--- DFT Result (showing first %d bins) ---\n", num_bins_to_display);
    printf("Bin\tFrequency(Hz)\tMagnitude\n");
    for (int k = 0; k < num_bins_to_display && k < N / 2; k++) {  // usually N/2 is enough for real signals
        double bin_freq = k * Fs / N;
        printf("%d\t%.2f\t\t%.4f\n", k, bin_freq, Mag[k]);
    }

    // Cleanup
    free(x);
    free(Re);
    free(Im);
    free(Mag);

    return 0;
}
