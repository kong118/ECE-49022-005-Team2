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

/**
 * @brief HAL MSP Init - Called by HAL_Init()
 * Enables required system clocks
 */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* PWR clock already enabled in HAL_MspInit */
    
    /* Configure the main internal regulator output voltage */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    
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
 * SysTick Handler - CRITICAL: Must call HAL_IncTick() every 1ms
 * Without this, HAL_Delay() and all HAL timeout functions hang forever!
 * ======================================================================== */

void SysTick_Handler(void)
{
    HAL_IncTick();
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
    uint8_t adxl362_ok = 0;  /* Flag: 1 if ADXL362 initialized successfully */
    
    /* ===== Initialization Phase ===== */
    
    /* Initialize HAL library (sets up SysTick, calls HAL_MspInit) */
    HAL_Init();
    
    /* Configure system clock - MSI 4MHz */
    SystemClock_Config();
    
    /* Initialize configuration with defaults */
    Config_Init();
    
    /* Initialize all GPIO pins and SPI */
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    /* Initialize USART2 for terminal output via ST-Link VCP (PA2=TX, PA15=RX) */
    MX_USART2_Init();
    
    /* Give UART time to stabilize */
    HAL_Delay(200);
    
    /* Send startup message */
    UART_SendString("\r\n=== ShakeAwake System Boot ===\r\n");
    
    /* Initialize ADXL362 accelerometer with default wake threshold */
    UART_SendString("Initializing ADXL362...\r\n");
    
    if (ADXL362_Init(DEFAULT_ACTIVITY_THRESHOLD_MG) != HAL_OK) {
        UART_SendString("WARNING: ADXL362 init failed (sensor may not be connected)\r\n");
        adxl362_ok = 0;
    } else {
        UART_SendString("ADXL362 init success\r\n");
        adxl362_ok = 1;
    }
    
    /* Enable EXTI interrupt for INT2 (PB6) - pins 5-9 in same handler */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
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
                    
                    /* Clear any pending EXTI interrupt that occurred during pulse */
                    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
                    wake_event_flag = 0;
                    
                    /* Debounce: ignore activity for 200ms after pulse ends */
                    HAL_Delay(200);
                }
            }
        }
        
        /* ===== Read and output IMU data to terminal ===== */
        if (adxl362_ok) {
            /* Read 12-bit acceleration data from ADXL362
             * Register 0x0E = XDATA_L, 0x0F = XDATA_H, 0x10 = YDATA_L, etc.
             * 6 bytes = X(L,H) + Y(L,H) + Z(L,H)
             */
            uint8_t accel_data[6];
            
            if (ADXL362_ReadBuffer(0x0E, accel_data, 6) == HAL_OK) {
                /* Convert raw data to signed 16-bit values (little-endian)
                 * ADXL362 12-bit data is right-justified, upper bits are sign-extended
                 * So casting to int16_t gives correct signed value directly
                 */
                int16_t x_raw = (int16_t)((accel_data[1] << 8) | accel_data[0]);
                int16_t y_raw = (int16_t)((accel_data[3] << 8) | accel_data[2]);
                int16_t z_raw = (int16_t)((accel_data[5] << 8) | accel_data[4]);
                
                /* Values are in mg (1 mg/LSB in +/-2g range)
                 * Display as G with integer math: e.g. 1023 mg -> "1.023 G"
                 * Note: newlib-nano doesn't support %%f, so use integer formatting
                 */
                
                /* Output every 500ms to prevent terminal spam */
                static uint32_t last_output_time = 0;
                uint32_t current_time = HAL_GetTick();
                
                if ((current_time - last_output_time) >= 500) {
                    last_output_time = current_time;
                    
                    /* Format: sign + integer part + 3 decimal places */
                    int16_t vals[3] = {x_raw, y_raw, z_raw};
                    const char *labels[3] = {"X:", "Y:", "Z:"};
                    char line[80];
                    int pos = 0;
                    
                    for (int i = 0; i < 3; i++) {
                        if (i > 0) {
                            line[pos++] = ' ';
                            line[pos++] = '|';
                            line[pos++] = ' ';
                        }
                        /* Copy label */
                        const char *l = labels[i];
                        while (*l) line[pos++] = *l++;
                        
                        int16_t v = vals[i];
                        int neg = 0;
                        if (v < 0) { neg = 1; v = -v; }
                        
                        int whole = v / 1000;
                        int frac = v % 1000;
                        
                        if (neg) line[pos++] = '-';
                        /* Integer part */
                        line[pos++] = '0' + (whole % 10);
                        line[pos++] = '.';
                        /* 3 decimal places */
                        line[pos++] = '0' + (frac / 100);
                        line[pos++] = '0' + ((frac / 10) % 10);
                        line[pos++] = '0' + (frac % 10);
                    }
                    line[pos++] = ' ';
                    line[pos++] = '(';
                    line[pos++] = 'G';
                    line[pos++] = ')';
                    line[pos++] = '\r';
                    line[pos++] = '\n';
                    line[pos] = '\0';
                    
                    UART_SendString(line);
                }
            }
        } else {
            /* ADXL362 not available, print heartbeat every 1 second */
            static uint32_t last_hb_time = 0;
            uint32_t current_time = HAL_GetTick();
            if ((current_time - last_hb_time) >= 1000) {
                last_hb_time = current_time;
                UART_SendString("Heartbeat (no ADXL362)\r\n");
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

