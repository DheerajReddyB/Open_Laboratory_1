// TFT_SPI_ST7735_F2837xD_final.c
// Basic 1.8" TFT (128x160) driver for F2837xD using SPIA (SPISIMOA / SPICLKA).
// Minimal init + fillScreen + drawPixel + sine demo.
//
// Wiring assumed (change GPIO numbers if different):
//  - SPI MOSI -> GPIO17 (SPISIMOA)
//  - SPI SCLK -> GPIO16 (SPICLKA)
//  - TFT DC   -> GPIO12
//  - TFT RST  -> GPIO13
//  - TFT CS   -> GPIO19
//
// NOTES:
// - Uses stdint.h for fixed-width types.
// - Uses SpiaRegs.* fields that are present in the F2837xD headers.
// - If you still get "undefined uint8_t", double-check that compiler includes standard headers and the
//   file is compiled as C (not C++).

#include "F28x_Project.h"
#include <stdint.h>
#include <math.h>

// ---------------------- PIN MACROS -------------------------
#define TFT_CS_LOW()     (GpioDataRegs.GPACLEAR.bit.GPIO19 = 1)
#define TFT_CS_HIGH()    (GpioDataRegs.GPASET.bit.GPIO19 = 1)

#define TFT_DC_LOW()     (GpioDataRegs.GPACLEAR.bit.GPIO12 = 1)
#define TFT_DC_HIGH()    (GpioDataRegs.GPASET.bit.GPIO12 = 1)

#define TFT_RST_LOW()    (GpioDataRegs.GPACLEAR.bit.GPIO13 = 1)
#define TFT_RST_HIGH()   (GpioDataRegs.GPASET.bit.GPIO13 = 1)

// Forward prototypes
static void initSPIA(void);
static void tft_init(void);
static void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void fillScreen(uint16_t color);
static void drawPixel(uint16_t x, uint16_t y, uint16_t color);
static void plotSine(const float samples[], int N);
static void spiWrite8(uint8_t d);
static void writeCmd(uint8_t c);
static void writeData(uint8_t d);
static void delayMs(uint32_t ms);

// ---------------------- low-level SPI -----------------------
static void spiWrite8(uint8_t d)
{
    // Place data in upper 8-bits of 16-bit SPITXBUF as TI SPI examples use.
    SpiaRegs.SPITXBUF = (uint16_t)(d << 8);

    // Wait until a response is in RX FIFO (transfer completed)
    while (SpiaRegs.SPIFFRX.bit.RXFFST == 0) { }
    // Read and discard to clear RX FIFO
    (void)SpiaRegs.SPIRXBUF;
}

static void writeCmd(uint8_t c)
{
    TFT_DC_LOW();
    TFT_CS_LOW();
    spiWrite8(c);
    TFT_CS_HIGH();
}

static void writeData(uint8_t d)
{
    TFT_DC_HIGH();
    TFT_CS_LOW();
    spiWrite8(d);
    TFT_CS_HIGH();
}

static void delayMs(uint32_t ms)
{
    // DELAY_US provided by F28x_Project.h
    DELAY_US(ms * 1000U);
}

// ---------------------- TFT init & helpers -------------------
static void tft_init(void)
{
    // Reset pulse
    TFT_RST_LOW();
    delayMs(50);
    TFT_RST_HIGH();
    delayMs(120);

    // Software reset
    writeCmd(0x01);
    delayMs(150);

    // Sleep out
    writeCmd(0x11);
    delayMs(120);

    // Color mode: 16-bit/pixel (RGB565)
    writeCmd(0x3A);
    writeData(0x05);

    // MADCTL: memory data access control (orientation)
    writeCmd(0x36);
    writeData(0xC0); // choose orientation; change if needed

    // Display ON
    writeCmd(0x29);
    delayMs(20);
}

static void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // Column address set
    writeCmd(0x2A);
    TFT_DC_HIGH();
    TFT_CS_LOW();
    spiWrite8(0x00);
    spiWrite8(x0);
    spiWrite8(0x00);
    spiWrite8(x1);
    TFT_CS_HIGH();

    // Row address set
    writeCmd(0x2B);
    TFT_DC_HIGH();
    TFT_CS_LOW();
    spiWrite8(0x00);
    spiWrite8(y0);
    spiWrite8(0x00);
    spiWrite8(y1);
    TFT_CS_HIGH();

    // Memory write
    writeCmd(0x2C);
}

static void fillScreen(uint16_t color)
{
    setAddrWindow(0, 0, 127, 159);

    TFT_DC_HIGH();
    TFT_CS_LOW();

    uint32_t pixels = 128U * 160U;
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    while (pixels--)
    {
        spiWrite8(hi);
        spiWrite8(lo);
    }

    TFT_CS_HIGH();
}

static void drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= 128U || y >= 160U) return;

    setAddrWindow((uint8_t)x, (uint8_t)y, (uint8_t)x, (uint8_t)y);

    TFT_DC_HIGH();
    TFT_CS_LOW();
    spiWrite8((uint8_t)(color >> 8));
    spiWrite8((uint8_t)(color & 0xFF));
    TFT_CS_HIGH();
}

static void plotSine(const float samples[], int N)
{
    fillScreen(0x0000);

    int drawN = (N < 128) ? N : 128;
    for (int xx = 0; xx < drawN; ++xx)
    {
        int y = (int)((samples[xx] + 1.0f) * 79.5f); // scale -1..1 to 0..159
        if (y < 0) y = 0;
        if (y > 159) y = 159;
        drawPixel((uint16_t)xx, (uint16_t)y, 0xFFFF);
    }
}

// ---------------------- SPIA init ----------------------------
static void initSPIA(void)
{
    EALLOW;

    // Configure SPI pins for CPU1 SPIA function (use mux=1)
    GPIO_SetupPinMux(16, GPIO_MUX_CPU1, 1); // SPICLKA
    GPIO_SetupPinMux(17, GPIO_MUX_CPU1, 1); // SPISIMOA (MOSI)

    // Configure as outputs (push-pull)
#ifdef GPIO_PUSHPULL
    GPIO_SetupPinOptions(16, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinOptions(17, GPIO_OUTPUT, GPIO_PUSHPULL);
#else
    // If macro not defined, use simple 0 for options (older headers)
    GPIO_SetupPinOptions(16, GPIO_OUTPUT, 0);
    GPIO_SetupPinOptions(17, GPIO_OUTPUT, 0);
#endif

    // TFT control pins as GPIO outputs (mux=0)
    GPIO_SetupPinMux(12, GPIO_MUX_CPU1, 0); // DC
    GPIO_SetupPinOptions(12, GPIO_OUTPUT, 0);

    GPIO_SetupPinMux(13, GPIO_MUX_CPU1, 0); // RST
    GPIO_SetupPinOptions(13, GPIO_OUTPUT, 0);

    GPIO_SetupPinMux(19, GPIO_MUX_CPU1, 0); // CS
    GPIO_SetupPinOptions(19, GPIO_OUTPUT, 0);

    EDIS;

    // Reset SPI while configuring
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;

    // 8-bit char
    SpiaRegs.SPICCR.bit.SPICHAR = 7;

    // Use a safe generic SPICTL setup: set master and enable talk later.
    // This .all value is common in TI examples; if your headers provide symbolic defs prefer them.
    SpiaRegs.SPICTL.all = 0x0006;

    // Set SPI baud (use .all to avoid union type mismatch)
    SpiaRegs.SPIBRR.all = 9; // adjust to get the desired bitrate for your LSPCLK

    // Bring SPI out of reset
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;

    // Enable transmit
    SpiaRegs.SPICTL.bit.TALK = 1;

    // Clear any RX FIFO contents
    while (SpiaRegs.SPIFFRX.bit.RXFFST != 0)
    {
        (void)SpiaRegs.SPIRXBUF;
    }
}

// ========================== MAIN ===============================
static float testSamples[128];

int main(void)
{
    // Initialize system control, clocks and default GPIO states
    InitSysCtrl();
    InitGpio();

    // Basic interrupt & PIE setup (optional for this demo)
    DINT;
    InitPieCtrl();
    InitPieVectTable();

    // Setup SPI and display pins
    initSPIA();

    // Default control pin states
    TFT_CS_HIGH();
    TFT_DC_HIGH();
    TFT_RST_HIGH();

    // Initialize display
    tft_init();

    // Clear screen
    fillScreen(0x0000);

    // Generate sine samples
    for (int i = 0; i < 128; ++i)
    {
        testSamples[i] = sinf(2.0f * 3.14159265358979323846f * (float)i / 128.0f);
    }

    // Plot the sine wave
    plotSine(testSamples, 128);

    // Optionally enable interrupts for your app
    EINT;
    ERTM;

    // Main loop
    for (;;)
    {
        // idle or add animations here
    }
}
