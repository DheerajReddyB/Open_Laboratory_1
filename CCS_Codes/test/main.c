// main.c
// Complex FFT + CSV output via polling SCIB for LAUNCHXL-F28379D

#include "F28x_Project.h"
#include <stdio.h>
#include <math.h>
#include "fpu_cfft.h"

// === FFT configuration ===
#define CFFT_STAGES  5
#define CFFT_SIZE    (1 << CFFT_STAGES)
#define RAD_STEP     0.1963495408494f

// === Buffers ===
float CFFTin1Buff[CFFT_SIZE*2];   // input
float CFFTin2Buff[CFFT_SIZE*2];   // phase
float CFFToutBuff[CFFT_SIZE*2];   // output
float CFFTF32Coef[CFFT_SIZE];     // twiddle factors

CFFT_F32_STRUCT cfft;
CFFT_F32_STRUCT_Handle hnd_cfft = &cfft;

// === SCIB polling TX ===
void scib_init(void)
{
    EALLOW;

    // Configure GPIO50/51 for SCIB
    GpioCtrlRegs.GPBMUX2.all &= ~0xFFFF0000;
    GpioCtrlRegs.GPBMUX2.all |= 0x55550000;  // SCIB function
    GpioCtrlRegs.GPBDIR.all &= ~0x00040000; // GPIO50 RX
    GpioCtrlRegs.GPBDIR.all |= 0x00080000;  // GPIO51 TX

    EDIS;

    // SCI-B setup
    ScibRegs.SCICCR.all = 0x0007;    // 1 stop, 8-bit, no parity
    ScibRegs.SCICTL1.all = 0x0003;   // enable TX/RX (reset)
    ScibRegs.SCIHBAUD.all = 0x0000;
    ScibRegs.SCILBAUD.all = 26;      // 115200 baud
    ScibRegs.SCICTL1.all = 0x0023;   // release from reset
}

// Send single char via SCIB polling
void scib_xmit(char c)
{
    while(ScibRegs.SCICTL2.bit.TXRDY == 0); // wait until TX ready
    ScibRegs.SCITXBUF.all = (Uint16)c;
}

// Send string
void scib_msg(const char *s)
{
    while(*s) scib_xmit(*s++);
}

// === Main ===
void main(void)
{
    Uint16 i;
    float Rad = 0.0f;
    char linebuf[128];

    // System init
    InitSysCtrl();
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    // Init SCIB
    scib_init();
    scib_msg("SCIB polling initialized. Streaming FFT CSV...\r\n");

    // Clear buffers
    for(i = 0; i < (CFFT_SIZE*2); i++)
    {
        CFFTin1Buff[i] = 0.0f;
        CFFTin2Buff[i] = 0.0f;
        CFFToutBuff[i] = 0.0f;
    }

    // Generate waveform
    for(i = 0; i < (CFFT_SIZE*2); i += 2)
    {
        CFFTin1Buff[i]   = sinf(Rad) + cosf(Rad*2.3567f);       // Real
        CFFTin1Buff[i+1] = cosf(Rad*8.345f) + sinf(Rad*5.789f); // Imag
        Rad += RAD_STEP;
    }

    // Configure FFT
    hnd_cfft->InPtr        = CFFTin1Buff;
    hnd_cfft->OutPtr       = CFFToutBuff;
    hnd_cfft->CoefPtr      = CFFTF32Coef;
    hnd_cfft->CurrentInPtr = CFFTin1Buff;
    hnd_cfft->CurrentOutPtr= CFFToutBuff;
    hnd_cfft->Stages       = CFFT_STAGES;
    hnd_cfft->FFTSize      = CFFT_SIZE;

    // Generate twiddle factors
    CFFT_f32_sincostable(hnd_cfft);

    // Run FFT
    CFFT_f32(hnd_cfft);

    // Compute magnitude
    CFFT_f32_mag(hnd_cfft);

    // Compute phase into spare buffer
    hnd_cfft->CurrentOutPtr = CFFTin2Buff;
    CFFT_f32_phase(hnd_cfft);

    // CSV header
    scib_msg("Index,Real,Imag,Magnitude,Phase\r\n");

    float *complexPtr = hnd_cfft->CurrentInPtr;
    float *magPtr     = hnd_cfft->CurrentOutPtr;
    float *phasePtr   = CFFTin2Buff;

    for(i = 0; i < CFFT_SIZE; i++)
    {
        float real = complexPtr[2*i];
        float imag = complexPtr[2*i+1];
        float mag  = magPtr[i];
        float ph   = phasePtr[i];
        sprintf(linebuf, "%u,%f,%f,%f,%f\r\n", (unsigned)i, real, imag, mag, ph);
        scib_msg(linebuf);
    }

    scib_msg("CSV dump complete.\r\n");

    for(;;) asm(" NOP");
}
