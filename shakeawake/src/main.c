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
 *   - ADXL362 INT1: PB1 / D6 (EXTI interrupt, rising edge)
 *   - OUTPUT: PA7 (GPIO output, active HIGH)
 *   - EN: PA6 (GPIO output, active HIGH)
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
    /* Use Scale 2 (low power) - sufficient for MSI 4MHz */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
    
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
    if (GPIO_Pin == GPIO_PIN_1) {  /* PB1 (D6) is INT1 */
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
    
    /* Initialize HAL library (sets up SysTick, calls HAL_MspInit) */
    HAL_Init();
    
    /* Configure system clock - MSI 4MHz */
    SystemClock_Config();
    
    /* Initialize configuration with defaults */
    Config_Init();
    
    /* Initialize all GPIO pins and SPI */
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    /* ===== CRITICAL: Disable debug in STOP/Standby modes ===== */
    /* ST-Link DBGMCU keeps all clocks running in STOP2 by default,
     * preventing the MCU from truly entering low-power (~3mA vs ~1uA) */
    DBGMCU->CR &= ~(DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY | DBGMCU_CR_DBG_SLEEP);
    
    /* Small delay for peripherals to stabilize */
    HAL_Delay(100);
    
    /* Initialize ADXL362 in wake-up mode with default activity threshold */
    if (ADXL362_Init(DEFAULT_ACTIVITY_THRESHOLD_MG) != HAL_OK) {
        /* Init failed - stay in error loop */
        while (1) { HAL_Delay(1000); }
    }
    
    /* ===== Clear stale ACT from boot ===== */
    ADXL362_WriteByte(0x2D, 0x00);  /* standby */
    HAL_Delay(10);
    ADXL362_ReadStatus(&status_reg); /* clear STATUS */
    ADXL362_WriteByte(0x2D, 0x0A);  /* wake-up mode */
    HAL_Delay(200);                  /* let reference sample settle */
    ADXL362_ReadStatus(&status_reg); /* clear any boot-up ACT */
    
    /* Clear any pending EXTI on PB1 */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    wake_event_flag = 0;
    
    /* Enable EXTI1 interrupt for INT1 (PB1 = D6) */
    HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    
    /* ===== Main Loop: STOP2 Low-Power with EXTI Wake ===== */
    
    while (1)
    {
        /* ---- Prepare for STOP2 ---- */
        /* Disable peripheral clocks (EXTI works without GPIO clock) */
        __HAL_RCC_SPI1_CLK_DISABLE();
        __HAL_RCC_GPIOA_CLK_DISABLE();
        __HAL_RCC_GPIOB_CLK_DISABLE();
        __HAL_RCC_SYSCFG_CLK_DISABLE();
        
        /* Suspend SysTick to avoid waking up every 1ms */
        HAL_SuspendTick();
        
        /* Enable Flash power-down in STOP mode */
        SET_BIT(FLASH->ACR, FLASH_ACR_SLEEP_PD);
        
        /* Enter STOP2 mode, wake on EXTI interrupt */
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
        
        /* ---- Woke up from STOP2 ---- */
        /* Restore system clock (MSI resets after STOP2) */
        SystemClock_Config();
        
        /* Resume SysTick */
        HAL_ResumeTick();
        
        /* Re-enable peripheral clocks */
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_SPI1_CLK_ENABLE();
        
        /* Check if we woke from ADXL362 INT1 */
        if (wake_event_flag == 1) {
            wake_event_flag = 0;
            
            /* Read STATUS to confirm activity */
            if (ADXL362_ReadStatus(&status_reg) == HAL_OK) {
                if ((status_reg & ADXL362_STATUS_ACT) != 0) {
                    
                    /* Step 1: OUT HIGH */
                    Output_Set_High();
                    HAL_Delay(50);
                    
                    /* Step 2: EN HIGH (OUT stays HIGH) */
                    Enable_Set_High();
                    HAL_Delay(100);
                    
                    /* Step 3: Both LOW */
                    Enable_Set_Low();
                    Output_Set_Low();
                }
            }
            
            /* Reset ADXL362 state machine */
            ADXL362_ReadStatus(&status_reg);  /* clear ACT */
            ADXL362_WriteByte(0x2D, 0x00);    /* standby */
            HAL_Delay(10);
            ADXL362_WriteByte(0x2D, 0x0A);    /* wake-up mode */
            HAL_Delay(100);                    /* reference settle */
            ADXL362_ReadStatus(&status_reg);  /* clear boot ACT */
            
            /* Clear EXTI and debounce */
            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
            wake_event_flag = 0;
            HAL_Delay(500);
            ADXL362_ReadStatus(&status_reg);
        }
        
        /* Clear any pending EXTI before re-entering STOP2 */
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        wake_event_flag = 0;
    }
    
    return 0;
}

/* ========================================================================
 * EXTI9_5 Interrupt Handler
 * This handles external interrupts on pins 5-9 (including PB6 = INT2)
 * ======================================================================== */

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

