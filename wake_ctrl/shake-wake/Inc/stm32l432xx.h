/* Minimal stub for stm32l432xx.h to satisfy IntelliSense and simple builds.
   This file does **not** contain full device definitions; it's only to make
   workspace analysis quiet. Real projects should use the vendor-provided CMSIS
   headers via the include path. */

#ifndef STM32L432XX_H
#define STM32L432XX_H

#include <stdint.h>

/* common type qualifiers */
#ifndef __IO
#define __IO volatile
#endif

/* --------------------------------------------------------------------------
   Core peripheral registers (very small subset used in this project)
   -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t PLLCFGR;
    __IO uint32_t CFGR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t APB2ENR;
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *)0x40021000U)

/* RCC register bit definitions used by this project */
#define RCC_CR_HSION        (1UL << 0)
#define RCC_CR_HSIRDY       (1UL << 1)
#define RCC_CR_PLLON        (1UL << 24)
#define RCC_CR_PLLRDY       (1UL << 25)

#define RCC_PLLCFGR_PLLSRC_HSI  (0UL)
#define RCC_PLLCFGR_PLLN_Pos    8
#define RCC_PLLCFGR_PLLM_Pos    4
#define RCC_PLLCFGR_PLLP_Pos    16

#define RCC_CFGR_SW_PLL      (0x2UL)
#define RCC_CFGR_SWS_Msk     (3UL << 2)
#define RCC_CFGR_SWS_PLL     (0x2UL << 2)

#define RCC_AHB2ENR_GPIOAEN  (1UL << 0)
#define RCC_AHB2ENR_GPIOBEN  (1UL << 1)
#define RCC_APB2ENR_SPI1EN   (1UL << 12)

/* FLASH registers (small portion) */
typedef struct {
    __IO uint32_t ACR;
} FLASH_TypeDef;

#define FLASH ((FLASH_TypeDef *)0x40022000U)
#define FLASH_ACR_LATENCY_2  (2U)

/* GPIO registers used by project */
typedef struct {
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t BRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0x48000000U)
#define GPIOB ((GPIO_TypeDef *)0x48000400U)

/* Minimal macros to keep code happy */
#define GPIO_MODER_MODE0_Msk  0
#define GPIO_MODER_MODE0_Pos  0
#define GPIO_MODER_MODE5_Msk  0
#define GPIO_MODER_MODE5_Pos  0
#define GPIO_MODER_MODE6_Msk  0
#define GPIO_MODER_MODE6_Pos  0
#define GPIO_MODER_MODE7_Msk  0
#define GPIO_MODER_MODE7_Pos  0
#define GPIO_MODER_MODE11_Msk 0
#define GPIO_MODER_MODE11_Pos 0

#define GPIO_OSPEEDR_OSPEED3_Pos   0
#define GPIO_OSPEEDR_OSPEED11_Pos  0
#define GPIO_PUPDR_PUPD6_Pos       0

/* --------------------------------------------------------------------------
   SPI peripheral (partial) -------------------------------------------------
   -------------------------------------------------------------------------- */
typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t CRCPR;
    __IO uint32_t RXCRCR;
    __IO uint32_t TXCRCR;
    __IO uint32_t I2SCFGR;
    __IO uint32_t I2SPR;
} SPI_TypeDef;

#define SPI1 ((SPI_TypeDef *)0x40013000U)

/* SPI register bit definitions used */
#define SPI_CR1_MSTR     (1UL << 2)
#define SPI_CR1_BR_Pos   3
#define SPI_CR1_SPE      (1UL << 6)

#define SPI_CR2_DS_Pos   8

#define SPI_SR_TXE       (1UL << 1)
#define SPI_SR_RXNE      (1UL << 0)

/* --------------------------------------------------------------------------
   IRQ & core helper macros --------------------------------------------------
   -------------------------------------------------------------------------- */
static inline void __disable_irq(void) { }
static inline void __enable_irq(void) { }

/* SysTick and core symbols */
extern uint32_t SystemCoreClock;
static inline uint32_t SysTick_Config(uint32_t ticks) { (void)ticks; return 0; }

#endif /* STM32L432XX_H */
