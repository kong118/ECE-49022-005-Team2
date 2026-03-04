#include "power_management.h"
#include "config.h"
#include "spi_gpio_config.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_pwr.h"
#include "stm32l4xx_hal_rcc.h"

/* Power management variables */
static volatile uint32_t last_activity_tick = 0;
static uint8_t system_in_stop2 = 0;

/*===================== System Clock Configuration =====================*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure the main internal regulator output voltage */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        while(1);
    }
    
    /* MSI Oscillators Initialization */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;  /* 4 MHz */
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1);
    }
    
    /* CPU, AHB, APB buses clocks configuration */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        while(1);
    }
    
    /* SysTick configuration for 1ms interrupt */
    if (HAL_SYSTICK_Config(SystemCoreClock / 1000)) {
        while(1);
    }
}

/*===================== STOP2 Configuration =====================*/
void SystemClock_Config_Stop2(void)
{
    /* Configuration for STOP2 mode - use lower clock */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;  /* 1 MHz for STOP2 */
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1);
    }
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        while(1);
    }
}

/*===================== Power Management Initialization =====================*/
void Power_Management_Init(void)
{
    /* Enable Power Clock */
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    
    /* Configure Power Control Register */
    /* Disable SRAM2 retention to save power in STOP2 */
    PWR->CR3 |= PWR_CR3_EIWUL;  /* Enable WKUP pin */
    
    last_activity_tick = HAL_GetTick();
}

/*===================== Enter STOP2 Mode =====================*/
void Enter_STOP2_Mode(void)
{
    /* Reduce clock for STOP2 */
    SystemClock_Config_Stop2();
    
    /* Configure Deep Sleep Mode */
    PWR->CR1 |= PWR_CR1_LPMS;  /* Set LPMS to STOP2 */
    
    /* Clear Power Control Flags */
    PWR->SCR |= PWR_SCR_CSBF;
    
    /* Set SLEEPDEEP bit */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    /* Set system in STOP2 flag */
    system_in_stop2 = 1;
    
    /* Wait For Interrupt - MCU enters STOP2, wakeup by INT2 */
    __WFI();
}

/*===================== Exit STOP2 Mode =====================*/
void Exit_STOP2_Mode(void)
{
    /* Clear SLEEPDEEP bit */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    
    /* Restore normal clock configuration */
    SystemClock_Config();
    
    system_in_stop2 = 0;
}

/*===================== Enter Run Mode =====================*/
void Enter_Run_Mode(void)
{
    /* Ensure system is in normal run mode */
    PWR->CR1 &= ~PWR_CR1_LPMS;
}

/*===================== Sleep/Wake Timeout Management =====================*/
void Enable_Sleep_Mode(uint16_t inactivity_timeout_sec)
{
    sys_config.inactivity_timeout_sec = inactivity_timeout_sec;
    sys_config.enable_stop2_mode = 1;
    last_activity_tick = HAL_GetTick();
}

void Disable_Sleep_Mode(void)
{
    sys_config.enable_stop2_mode = 0;
    Exit_STOP2_Mode();
}

void Check_Sleep_Timeout(void)
{
    if (sys_config.enable_stop2_mode) {
        uint32_t elapsed_ms = HAL_GetTick() - last_activity_tick;
        uint32_t timeout_ms = (uint32_t)sys_config.inactivity_timeout_sec * 1000;
        
        if (elapsed_ms > timeout_ms) {
            /* Enter STOP2 mode if no activity detected */
            if (!system_in_stop2) {
                Enter_STOP2_Mode();
            }
        }
    }
}

/*===================== System Wake-up from INT2 =====================*/
void System_Wakeup_From_INT2(void)
{
    /* Exit STOP2 if we were in it */
    if (system_in_stop2) {
        Exit_STOP2_Mode();
    }
    
    /* Update last activity time */
    last_activity_tick = HAL_GetTick();
    
    /* Set activity flag */
    activity_detected_flag = 1;
}

uint8_t Is_System_Awake(void)
{
    return !system_in_stop2;
}

/*===================== EXTI Interrupt Handler =====================*/
void EXTI15_10_IRQHandler(void)
{
    /* Check if INT2 (PB6) triggered the interrupt */
    if (EXTI->PR1 & (1 << INT2_PIN)) {
        /* Clear the interrupt pending flag */
        EXTI->PR1 = (1 << INT2_PIN);
        
        /* Wake up system and set activity detected flag */
        System_Wakeup_From_INT2();
    }
}

/*===================== SysTick Handler =====================*/
void SysTick_Handler(void)
{
    HAL_IncTick();
}
