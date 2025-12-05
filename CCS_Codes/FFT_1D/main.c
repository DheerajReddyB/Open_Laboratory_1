#include "D:\SEM-5\Open_Laboratory_1\CCS_Codes\FFT_1D\device\driverlib.h"
#include "D:\SEM-5\Open_Laboratory_1\CCS_Codes\FFT_1D\device\device.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

// -------------------------
// Constants
// -------------------------
#define FFT_SIZE 1024
#define MY_PI 3.14159265358979323846f

typedef float float32;

// -------------------------
// Complex type
// -------------------------
typedef struct {
    float32 re;
    float32 im;
} complex_t;

// -------------------------
// Buffers
// -------------------------
complex_t fft_buffer[FFT_SIZE];
float32 fft_input[FFT_SIZE];

// -------------------------
// UART helper (SCI-A)
// -------------------------
void sciA_init(void)
{
    SCI_performSoftwareReset(SCIA_BASE);

    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 115200,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));

    SCI_resetChannels(SCIA_BASE);
    SCI_resetRxFIFO(SCIA_BASE);
    SCI_resetTxFIFO(SCIA_BASE);

    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(SCIA_BASE);
    SCI_enableModule(SCIA_BASE);
    SCI_performSoftwareReset(SCIA_BASE);
}

void sciA_writeChar(uint16_t ch)
{
    SCI_writeCharBlockingFIFO(SCIA_BASE, ch);
}

void sciA_writeString(const char *msg)
{
    while(*msg)
    {
        sciA_writeChar(*msg++);
    }
}

// -------------------------
// FFT helper functions
// -------------------------
unsigned int reverseBits(unsigned int x, int n)
{
    unsigned int rev = 0;
    int i;
    for(i=0; i<n; i++)
    {
        rev <<= 1;
        rev |= (x & 1);
        x >>= 1;
    }
    return rev;
}

void fft(complex_t *x, int N)
{
    int logN = 0;
    int i, j, k, s;
    float32 tmp_re, tmp_im;

    int tmp = N;
    while(tmp > 1)
    {
        logN++;
        tmp >>= 1;
    }

    // Bit reversal
    for(i=0; i<N; i++)
    {
        j = reverseBits(i, logN);
        if(j > i)
        {
            complex_t tmpc = x[i];
            x[i] = x[j];
            x[j] = tmpc;
        }
    }

    // Cooley-Tukey radix-2
    for(s=1; s<=logN; s++)
    {
        int m = 1 << s;
        float32 angle = -2.0f * MY_PI / m;
        complex_t wm = { cosf(angle), sinf(angle) };

        for(k=0; k<N; k+=m)
        {
            complex_t w = {1.0f, 0.0f};
            for(j=0; j<m/2; j++)
            {
                complex_t t;
                t.re = w.re * x[k+j+m/2].re - w.im * x[k+j+m/2].im;
                t.im = w.re * x[k+j+m/2].im + w.im * x[k+j+m/2].re;

                complex_t u = x[k+j];

                x[k+j].re = u.re + t.re;
                x[k+j].im = u.im + t.im;
                x[k+j+m/2].re = u.re - t.re;
                x[k+j+m/2].im = u.im - t.im;

                // w *= wm
                tmp_re = w.re*wm.re - w.im*wm.im;
                tmp_im = w.re*wm.im + w.im*wm.re;
                w.re = tmp_re;
                w.im = tmp_im;
            }
        }
    }
}

// -------------------------
// Main
// -------------------------
void main(void)
{
    int i;
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();

    sciA_init();
    sciA_writeString("UART Ready...\r\n");

    // Generate test sine wave (freq = 50 Hz, fs = 1000 Hz)
    for(i=0; i<FFT_SIZE; i++)
    {
        fft_input[i] = sinf(2.0f * MY_PI * 50.0f * ((float32)i / 1000.0f));
        fft_buffer[i].re = fft_input[i];
        fft_buffer[i].im = 0.0f;
    }

    // Run FFT
    fft(fft_buffer, FFT_SIZE);

    // Send magnitude results over UART
    for(i=0; i<FFT_SIZE/2; i++)
    {
        char buffer[64];
        float32 mag = sqrtf(fft_buffer[i].re*fft_buffer[i].re + fft_buffer[i].im*fft_buffer[i].im);
        snprintf(buffer, sizeof(buffer), "%d, %f\r\n", i, mag);
        sciA_writeString(buffer);
    }

    while(1);  // Infinite loop at end
}
