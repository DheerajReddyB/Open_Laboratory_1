#include "D:\SEM-5\Open_Laboratory_1\CCS_Codes\blink_led\device\device.h"
#include "D:\SEM-5\Open_Laboratory_1\CCS_Codes\blink_led\device\driverlib.h"
void main(void)
{
    Device_init();
    Device_initGPIO();

    // Configure LED1 (Red, GPIO31)
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);

    for(;;)
    {
        // LED ON
        GPIO_writePin(DEVICE_GPIO_PIN_LED2, 0);
        DEVICE_DELAY_US(500000);  // 500 ms

        // LED OFF
        GPIO_writePin(DEVICE_GPIO_PIN_LED2, 1);
        DEVICE_DELAY_US(500000);  // 500 ms
    }
}
