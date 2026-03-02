#include "../Inc/mcu_init.h"

/**
 * @brief Initialize system clock to max frequency
 * NUCLEO-L432KC uses 16 MHz HSI
 * PLL multiplies to 80 MHz (max for L432)
 */
void SystemClock_Init(void)
{
    /* Enable HSI clock */
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    /* Set flash wait states for 80 MHz (2 wait states) */
    FLASH->ACR = FLASH_ACR_LATENCY_2;

    /* Configure PLL: HSI / 1 * 5 = 80 MHz */
    RCC->PLLCFGR = (RCC_PLLCFGR_PLLSRC_HSI |    /* HSI as source */
                    (4UL << RCC_PLLCFGR_PLLN_Pos) |  /* N = 5 */
                    (0UL << RCC_PLLCFGR_PLLM_Pos) |  /* M = 1 */
                    (0UL << RCC_PLLCFGR_PLLP_Pos));  /* P = 7 */

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    /* Switch to PLL as system clock */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL);

    /* Update SystemCoreClock */
    SystemCoreClock = 80000000;
}

/**
 * Initialize LED output used for wake indication.
 * We map this to D10 (PA11) and keep it active-high.
 * The onboard green LED on PA5 conflicts with SPI SCK so PA11 is chosen.
 */
void LED_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* Configure PA11 as general-purpose output (D10) */
    GPIOA->MODER &= ~(GPIO_MODER_MODE11_Msk);
    GPIOA->MODER |= (1UL << GPIO_MODER_MODE11_Pos);

    /* Set output speed to high for fast transitions */
    GPIOA->OSPEEDR |= (3UL << GPIO_OSPEEDR_OSPEED11_Pos);

    /* Ensure the pin is initially low (inactive) */
    GPIOA->BRR = (1UL << 11);
}

/**
 * @brief Turn on LED
 */
void LED_On(void)
{
    GPIOA->BSRR = (1UL << 11);
}

/**
 * @brief Turn off LED
 */
void LED_Off(void)
{
    GPIOA->BRR = (1UL << 11);
}

/**
 * @brief Toggle LED
 */
void LED_Toggle(void)
{
    GPIOA->ODR ^= (1UL << 11);
}

/* Update SystemCoreClock variable */
uint32_t SystemCoreClock = 16000000;  /* Will be updated to 80 MHz */

/**
 * @brief Configure system clock
 */
void SystemInit(void)
{
    /* Disable all interrupts during setup */
    __disable_irq();

    /* Initialize system clock */
    SystemClock_Init();

    /* Initialize LED */
    LED_Init();

    /* Enable interrupts */
    __enable_irq();
}
