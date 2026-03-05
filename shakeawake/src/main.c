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
#include <stdio.h>

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
    
    /* Test GPIO with direct register writes (bypassing HAL macros) */
    /* Set PA7 (OUTPUT) to HIGH using direct register */
    GPIOA->BSRR = GPIO_PIN_7;             /* Set PA7 HIGH */
    HAL_Delay(500);
    
    /* Set PA7 to LOW using direct register */
    GPIOA->BSRR = (GPIO_PIN_7 << 16);     /* Reset PA7 LOW */
    HAL_Delay(500);
    
    /* Set PA3 (EN) to HIGH using direct register */
    GPIOA->BSRR = GPIO_PIN_3;             /* Set PA3 HIGH */
    HAL_Delay(500);
    
    /* Set PA3 to LOW using direct register */
    GPIOA->BSRR = (GPIO_PIN_3 << 16);     /* Reset PA3 LOW */
    HAL_Delay(500);
    
    /* Now test the macro-based functions */
    for (int i = 0; i < 3; i++) {
        Output_Set_High();
        HAL_Delay(200);
        Output_Set_Low();
        HAL_Delay(200);
    }
    
    /* Initialize USART1 for terminal output (PA9=TX, PA10=RX) */
    MX_USART1_Init();
    
    /* Give UART time to stabilize */
    HAL_Delay(300);
    
    /* Send very simple test message */
    uint8_t test1[] = "UART TEST 1\r\n";
    HAL_UART_Transmit(&huart1, test1, sizeof(test1)-1, 5000);
    HAL_Delay(200);
    
    /* Initialize ADXL362 accelerometer with default wake threshold */
    uint8_t test2[] = "Starting ADXL362 init\r\n";
    HAL_UART_Transmit(&huart1, test2, sizeof(test2)-1, 5000);
    HAL_Delay(200);
    
    HAL_StatusTypeDef adxl_status = ADXL362_Init(DEFAULT_ACTIVITY_THRESHOLD_MG);
    
    if (adxl_status != HAL_OK) {
        uint8_t error_msg[] = "ERROR: ADXL362 init failed\r\n";
        HAL_UART_Transmit(&huart1, error_msg, sizeof(error_msg)-1, 5000);
        HAL_Delay(200);
        Error_Handler();
    } else {
        uint8_t ok_msg[] = "ADXL362 init success\r\n";
        HAL_UART_Transmit(&huart1, ok_msg, sizeof(ok_msg)-1, 5000);
        HAL_Delay(200);
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
        
        /* ===== Read and output IMU data to terminal ===== */
        /* Read acceleration data from ADXL362 - register address 0x08 (XDATA) */
        uint8_t accel_data[6];
        
        if (ADXL362_ReadBuffer(0x08, accel_data, 6) == HAL_OK) {
            /* Convert raw data to signed 16-bit values (little-endian) */
            int16_t x_raw = (int16_t)((accel_data[1] << 8) | accel_data[0]);
            int16_t y_raw = (int16_t)((accel_data[3] << 8) | accel_data[2]);
            int16_t z_raw = (int16_t)((accel_data[5] << 8) | accel_data[4]);
            
            /* ADXL362 outputs 12-bit data in 16-bit registers, shift to actual 12-bit value */
            x_raw >>= 4;
            y_raw >>= 4;
            z_raw >>= 4;
            
            /* Convert to G-force (1 mg/LSB in ±2g range) */
            float x_g = (float)x_raw / 1000.0f;
            float y_g = (float)y_raw / 1000.0f;
            float z_g = (float)z_raw / 1000.0f;
            
            /* Output every 500ms to prevent terminal spam */
            static uint32_t last_output_time = 0;
            uint32_t current_time = HAL_GetTick();
            
            if ((current_time - last_output_time) >= 500) {
                last_output_time = current_time;
                /* Use simple format without sqrt to avoid math library issues */
                UART_Printf("X:%6.2f | Y:%6.2f | Z:%6.2f (G)\r\n", x_g, y_g, z_g);
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

