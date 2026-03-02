/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Team
 * @brief          : ADXL362 Shake Awake Detection for NUCLEO-L432KC
 ******************************************************************************
 */

#include <stdint.h>
#include "../Inc/stm32l432xx.h"
#include "../Inc/mcu_init.h"
#include "../Inc/adxl362.h"
#include "../Inc/shake_detect.h"

/* Configuration: Activity threshold in mg (adjustable) */
#define ACTIVITY_THRESHOLD 150  /* Can be changed to 50-500 mg */

/* Debounce counter for shake detection */
static uint32_t shake_debounce = 0;
static uint32_t wake_event_count = 0;

/**
 * @brief Main entry point
 */
int main(void)
{
    /* Initialize MCU and peripherals */
    SystemInit();

    /* Initialize shake detection with threshold */
    ShakeDetect_Init(ACTIVITY_THRESHOLD);

    /* Main loop */
    while(1)
    {
        /* Process shake detection */
        ShakeDetect_Process();

        /* Check if wake event detected */
        if(ShakeDetect_IsAwake())
        {
            shake_debounce++;

            /* Debounce filter: require multiple consecutive detections */
            if(shake_debounce > 2)
            {
                /* Wake event confirmed - toggle LED */
                LED_Toggle();
                wake_event_count++;

                /* Clear wake flag */
                ShakeDetect_ClearAwake();
                shake_debounce = 0;

                /* Optional: Can add custom wake-up action here */
                /* e.g., wake up main system, send signal, etc. */
            }
        }
        else
        {
            shake_debounce = 0;
        }

        /* Delay for processing */
        for(uint32_t i = 0; i < 100000; i++);
    }

    return 0;
}

/**
 * @brief Override weak SystemInit to customize it
 * This ensures our custom initialization is used
 */
void SystemInit_User(void)
{
    SystemInit();
}

/**
 * @brief Set activity threshold dynamically
 * Call this to change the shake detection sensitivity
 * @param threshold Threshold in mg (typical range 50-500)
 */
void Set_ActivityThreshold(uint16_t threshold)
{
    ShakeDetect_SetThreshold(threshold);
}

/**
 * @brief Get current activity threshold
 * @return Current threshold in mg
 */
uint16_t Get_ActivityThreshold(void)
{
    return ShakeDetect_GetThreshold();
}

/**
 * @brief Get wake event count
 * @return Number of shake awake events detected
 */
uint32_t Get_WakeEventCount(void)
{
    return wake_event_count;
}
