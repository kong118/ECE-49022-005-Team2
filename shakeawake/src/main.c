/* ========================================================================
 * ShakeAwake Application - STM32L432KC with ADXL362
 * 
 * Functionality:
 *   - Monitors ADXL362 accelerometer for activity (shake) events
 *   - INT2 rising edge triggers OUTPUT (PA7) and EN (PA3) HIGH for 500ms
 *   - After 500ms, both pins are driven LOW
 *   - Wake threshold is configurable via ADXL362_SetWakeThreshold_mg()
 * 
 * pin assignments:
 *   - SPI1: PB3 (SCLK), PB5 (MOSI), PB4 (MISO)
 *   - ADXL362 CS: PB0 (GPIO output, active LOW)
 *   - ADXL362 INT2: PB6 (EXTI interrupt, rising edge)
 *   - OUTPUT: PA7 (GPIO output, active HIGH)
 *   - EN: PA3 (GPIO output, active HIGH)
 * ======================================================================== */

#include "stm32l4xx_hal.h"
#include "config.h"
#include "spi_gpio_config.h"
#include "adxl362_lowpower.h"

/* ========================================================================
 * Forward Declarations
 * ======================================================================== */
void Error_Handler(void);

/* ========================================================================
 * Global Variables - extern declarations
 * The actual definitions are in config.c
 * ======================================================================== */

extern volatile uint8_t wake_event_flag;  /* Set by EXTI interrupt handler */

/* ========================================================================
 * System Clock Configuration
 * ======================================================================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure the main internal regulator output voltage */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }
    
    /* Initializes the RCC Oscillators according to the specified parameters */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;  /* ~4 MHz */
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    /* Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/* ========================================================================
 * Error Handler
 * ======================================================================== */

void Error_Handler(void)
{
    /* Stop execution and signal error */
    while (1) {
        /* Toggle error LED, or enter infinite loop */
    }
}

/* ========================================================================
 * EXTI Callback - Called when INT2 (PB6) rising edge is detected
 * ======================================================================== */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_6) {  /* PB6 is INT2 */
        /* Set flag to process wake event in main loop */
        wake_event_flag = 1;
    }
}

/* ========================================================================
 * Main Application
 * ======================================================================== */

int main(void)
{
    uint8_t status_reg = 0;
    
    /* ===== Initialization Phase ===== */
    
    /* Initialize HAL library */
    HAL_Init();
    
    /* Configure system clock */
    SystemClock_Config();
    
    /* Initialize configuration with defaults */
    Config_Init();
    
    /* Initialize all GPIO pins and SPI */
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    /* Initialize ADXL362 accelerometer with default wake threshold */
    if (ADXL362_Init(DEFAULT_ACTIVITY_THRESHOLD_MG) != HAL_OK) {
        Error_Handler();
    }
    
    /* Enable EXTI interrupt for INT2 (PB6) - pins 4-15 in same handler */
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);    /* Pin 6 is in 5-9 range */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    
    /*
     * Optional: Test startup sequence by pulsing OUTPUT/EN
     * Uncomment to verify pins work correctly
     */
    // {
    //     Output_Set_High();
    //     Enable_Set_High();
    //     HAL_Delay(500);
    //     Output_Set_Low();
    //     Enable_Set_Low();
    //     HAL_Delay(500);
    // }
    
    /* ===== Main Loop ===== */
    
    while (1)
    {
        /*
         * Wait for INT2 rising edge (activity detected)
         * The EXTI interrupt handler sets wake_event_flag
         */
        if (wake_event_flag == 1) {
            wake_event_flag = 0;
            
            /*
             * Read ADXL362 STATUS register to confirm activity event.
             * Per datasheet, reading STATUS also clears the interrupt flag.
             */
            if (ADXL362_ReadStatus(&status_reg) == HAL_OK) {
                /*
                 * Check ACT bit (bit 4) in STATUS register.
                 * Only proceed if activity is confirmed.
                 */
                if ((status_reg & ADXL362_STATUS_ACT) != 0) {
                    /*
                     * Real activity event detected.
                     * Pulse sequence for OUTPUT (PA7) and EN (PA3):
                     *   1. OUTPUT HIGH for 50ms
                     *   2. Then EN HIGH
                     *   3. Keep both HIGH for remaining 450ms (500ms total)
                     *   4. Both LOW
                     * 
                     * IMPORTANT:
                     * - OUTPUT is asserted 50ms BEFORE EN
                     * - Total time from OUTPUT high to both low is 500ms
                     * - System waits for next wake event
                     */
                    
                    /* Step 1: OUTPUT HIGH first */
                    Output_Set_High();
                    
                    /* Step 2: Wait 50ms before enabling EN */
                    HAL_Delay(50);
                    
                    /* Step 3: EN HIGH (OUTPUT already HIGH) */
                    Enable_Set_High();
                    
                    /* Step 4: Keep both HIGH for remaining 450ms (500 - 50 = 450) */
                    HAL_Delay(450);
                    
                    /* Step 5: Drive both LOW to end pulse */
                    Output_Set_Low();
                    Enable_Set_Low();
                }
            }
        }
        
        /* Small delay to prevent CPU spinning */
        HAL_Delay(50);
    }
    
    return 0;
}

/* ========================================================================
 * EXTI9_5 Interrupt Handler
 * This handles external interrupts on pins 5-9 (including PB6 = INT2)
 * ======================================================================== */

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}

