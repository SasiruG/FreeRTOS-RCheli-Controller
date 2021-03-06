#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"

/**
 * main.c
 */

// Blink LED Task
void BlinkLED(void *);

int main(void)
{
    // Set the clock rate to 80 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
     SYSCTL_XTAL_16MHZ);

    // Prototype for xTaskCreate:
    //
    // BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
    // const char * const pcName, /*lint !e971 Unqualified char types are allowed
    // for strings and single characters only. */
    // const configSTACK_DEPTH_TYPE usStackDepth, void * const pvParameters,
    // UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask );


    // For LED blinky task - initialize GPIO port F and then pin #1 (red) for output
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // activate internal bus clocking for GPIO port F
     while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) ; // busy-wait until GPIOF's bus clock is ready

     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF_1 as output
     // doesn't need too much drive strength as the RGB LEDs on the TM4C123 launchpad are switched via N-type
     //transistors
     GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // off by default

     if (pdTRUE != xTaskCreate(BlinkLED, "Blinker", 32, (void *)1, 4, NULL)) { // (void *)1 is our pvParameters for
                                                                               // our task func specifying PF_1
     while (1) ; // error creating task, out of memory?
     }


    vTaskStartScheduler(); // Start FreeRTOS!!
    // Should never get here since the RTOS should never "exit".
    while (1);
}

//Blinky function
//
void BlinkLED(void *pvParameters)
{
    /* While pvParameters is technically a pointer, a pointer
     is nothing more than an unsigned integer of size equal to the
     architecture's memory address bus width, which is 32-bits in ARM.
     We're abusing the parameter then to hold a simple integer value.
     Could also have used this as a pointer to a memory location holding the
     value, but our method uses less memory.
     */
    unsigned int whichLed = (unsigned int) pvParameters;
    const uint8_t whichBit = 1 << whichLed; // TivaWare GPIO calls require the pin# as a binary bitmask, not a
                                            //simple number.

    // Alternately, we could have passed the bitmask into pvParameters instead of a simple number.
    uint8_t currentValue = 0;
    while (1)
    {
        currentValue ^= whichBit; // XOR keeps flipping the bit on / off alternately each time this runs.
        GPIOPinWrite(GPIO_PORTF_BASE, whichBit, currentValue);
        vTaskDelay(125 / portTICK_RATE_MS); // Suspend this task (so others may run) for 125ms or as close as we
                                            // can get with the current RTOS tick setting.
    }
    // No way to kill this blinky task unless another task has an xTaskHandle reference to it and can use vTaskDelete() to purge it.
}

