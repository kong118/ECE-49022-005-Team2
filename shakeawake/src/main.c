/* ========================================================================
 * Shake-Awake Application - STM32L432KC with ADXL362
 * 
 * Functionality:
 *   - Monitors ADXL362 accelerometer for activity (shake) events
 *   - INT2 rising edge triggers Verdin wake pulse through PA6 -> Q1
 *   - After VERDIN_WAKE_PULSE_MS (500ms default), PA6 returns LOW
 *   - Lockout prevents repeated pulses within SHAKE_AWAKE_LOCKOUT_MS (2000ms)
 * 
 * Hardware connections:
 *   - SPI1: PB3 (SCLK), PB5 (MOSI), PB4 (MISO)
 *   - ADXL362 CS: PB0 (GPIO output, active LOW)
 *   - ADXL362 INT2: PB6 (EXTI interrupt, rising edge)
 *   - Verdin wake: PA6 -> Q1 2N7002 -> Verdin control line
 * 
 * Verdin Control Pin Warning:
 *   - Verdin control pins are 1.8V domain!
 *   - The Q1 2N7002 transistor provides level-shifting from 3.3V STM32
 *   - to 1.8V Verdin input. PA6 HIGH turns Q1 on, pulling the Verdin
 *   - control line LOW (active-low signal).
 *   - Keep pulse below 5 seconds to avoid Verdin force-off behavior
 *   - on CTRL_PWR_BTN_MICO#.
 * ======================================================================== */

#include "stm32l4xx_hal.h"
#include "config.h"
#include "spi_gpio_config.h"
#include "adxl362_lowpower.h"
#include "shake_awake.h"

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
    /* Delegate to shake_awake module */
    shake_awake_on_exti(GPIO_Pin);
}

/* ========================================================================
 * Main Application
 * ======================================================================== */

int main(void)
{
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
    
    /* ===== Initialize Shake-Awake Module ===== */
    /* This configures:
     *   - PA6 as Verdin wake output (defaults LOW immediately)
     *   - PB6 as EXTI input for ADXL362 INT2
     *   - ADXL362 for activity detection with interrupt on INT2
     */
    shake_awake_init();
    
    /* Arm the shake-awake system */
    shake_awake_arm();
    
    /* ===== Main Loop: STOP2 Low-Power with EXTI Wake ===== */
    
    while (1)
    {
#if ENABLE_LOW_POWER_AFTER_SHAKE_AWAKE
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
#endif
        /* ---- Process shake-awake events ---- */
        shake_awake_task();
        
#if ENABLE_LOW_POWER_AFTER_SHAKE_AWAKE
        /* Clear any pending EXTI before re-entering STOP2 */
        __HAL_GPIO_EXTI_CLEAR_IT(ADXL362_INT2_Pin);
#endif
    }
    
    return 0;
}

