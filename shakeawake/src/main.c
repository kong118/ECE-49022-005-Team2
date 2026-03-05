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
    char dbg[80];
    
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
    
    /* Initialize ADXL362 in wake-up mode with default activity threshold */
    UART_SendString("Initializing ADXL362 wake mode...\r\n");
    
    if (ADXL362_Init(DEFAULT_ACTIVITY_THRESHOLD_MG) != HAL_OK) {
        UART_SendString("ERROR: ADXL362 init failed!\r\n");
        /* Stay in error loop - blink can be added here */
        while (1) { HAL_Delay(1000); }
    }
    
    /* ===== Debug: Read back key registers ===== */
    {
        uint8_t reg_val;
        ADXL362_ReadByte(0x20, &reg_val);
        snprintf(dbg, sizeof(dbg), "  THRESH_ACT_L = 0x%02X (%d)\r\n", reg_val, reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x21, &reg_val);
        snprintf(dbg, sizeof(dbg), "  THRESH_ACT_H = 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x22, &reg_val);
        snprintf(dbg, sizeof(dbg), "  TIME_ACT     = 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x27, &reg_val);
        snprintf(dbg, sizeof(dbg), "  ACT_INACT_CTL= 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x2B, &reg_val);
        snprintf(dbg, sizeof(dbg), "  INTMAP2      = 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x2D, &reg_val);
        snprintf(dbg, sizeof(dbg), "  POWER_CTL    = 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
        
        ADXL362_ReadByte(0x0B, &reg_val);
        snprintf(dbg, sizeof(dbg), "  STATUS       = 0x%02X\r\n", reg_val);
        UART_SendString(dbg);
    }
    
    UART_SendString("ADXL362 ready. Waiting for motion...\r\n");
    
    /* ===== Clear stale ACT from boot ===== */
    /* After entering wake-up mode, ACT may already be set.
     * Reset state machine so first detection is real motion. */
    ADXL362_WriteByte(0x2D, 0x00);  /* standby */
    HAL_Delay(10);
    ADXL362_ReadStatus(&status_reg); /* clear STATUS */
    ADXL362_WriteByte(0x2D, 0x0A);  /* wake-up mode */
    HAL_Delay(200);                  /* let reference sample settle */
    ADXL362_ReadStatus(&status_reg); /* clear any boot-up ACT */
    
    /* Enable EXTI interrupt for INT2 (PB6) */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    /* ===== Main Loop ===== */
    /* Use STATUS polling as primary detection method.
     * EXTI interrupt is kept as backup but STATUS polling is more reliable
     * since it doesn't depend on which INT pin is physically wired. */
    
    while (1)
    {
        /* Poll STATUS register every 100ms */
        if (ADXL362_ReadStatus(&status_reg) == HAL_OK) {
            
            /* Check ACT bit (0x10) - set when activity exceeds threshold */
            if ((status_reg & ADXL362_STATUS_ACT) != 0) {
                
                UART_SendString("Motion detected!\r\n");
                
                /* Step 1: OUT HIGH */
                Output_Set_High();
                HAL_Delay(50);
                
                /* Step 2: EN HIGH (OUT stays HIGH) */
                Enable_Set_High();
                HAL_Delay(100);
                
                /* Step 3: Both LOW */
                Enable_Set_Low();
                Output_Set_Low();
                
                /* Read STATUS to clear ACT */
                ADXL362_ReadStatus(&status_reg);
                
                /* Reset ADXL362 state machine: standby → wake-up mode. */
                ADXL362_WriteByte(0x2D, 0x00);  /* POWER_CTL = standby */
                HAL_Delay(10);
                ADXL362_WriteByte(0x2D, 0x0A);  /* POWER_CTL = wake-up mode */
                HAL_Delay(100);  /* Wait for reference sample to settle */
                
                /* Clear STATUS after restart */
                ADXL362_ReadStatus(&status_reg);
                
                /* Clear EXTI flag and event flag */
                __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
                wake_event_flag = 0;
                
                /* Debounce delay */
                HAL_Delay(500);
                
                /* Clear one more time after debounce */
                ADXL362_ReadStatus(&status_reg);
            }
        }
        
        /* Poll interval: 100ms (ADXL362 wake-up mode ODR is 6Hz = 167ms) */
        HAL_Delay(100);
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

