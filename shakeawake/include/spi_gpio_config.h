#ifndef SPI_GPIO_CONFIG_H
#define SPI_GPIO_CONFIG_H

#include "stm32l4xx_hal.h"

/* ========================================================================
 * SPI and GPIO Configuration
 * ======================================================================== */

/* SPI1 Handle (to be used in main.c and spi_gpio_config.c) */
extern SPI_HandleTypeDef hspi1;

/* ========================================================================
 * GPIO Initialization Functions
 * ======================================================================== */

/**
 * @brief Initialize all GPIO pins
 *   - SPI pins (SCLK, MOSI, MISO) in AF mode
 *   - ADXL362 CS (PB0) as GPIO output (initially HIGH)
 *   - ADXL362 INT2 (PB6) as EXTI input (rising edge)
 *   - OUTPUT (PA7) as GPIO output (initially LOW)
 *   - EN (PA3) as GPIO output (initially LOW)
 */
void MX_GPIO_Init(void);

/**
 * @brief Initialize SPI1 peripheral
 *   - Master mode, 8-bit, CPOL=0, CPHA=0
 *   - Software NSS (CS handled manually)
 *   - Prescaler set for appropriate clock speed
 */
void MX_SPI1_Init(void);

/* ========================================================================
 * Helper Functions for ADXL362 CS Control
 * ======================================================================== */

/**
 * @brief Drive CS LOW for SPI transaction
 */
static inline void ADXL362_CS_Low(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

/**
 * @brief Drive CS HIGH after SPI transaction
 */
static inline void ADXL362_CS_High(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

/* ========================================================================
 * Helper Functions for OUTPUT Pin Control
 * ======================================================================== */

/**
 * @brief Set OUTPUT (PA7) HIGH
 * WARNING: OUTPUT is asserted HIGH only for confirmed activity wake events
 */
static inline void Output_Set_High(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
}

/**
 * @brief Set OUTPUT (PA7) LOW
 */
static inline void Output_Set_Low(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
}

/* ========================================================================
 * Helper Functions for EN Pin Control
 * ======================================================================== */

/**
 * @brief Set EN (PA3) HIGH
 * WARNING: EN is asserted HIGH only for confirmed activity wake events
 */
static inline void Enable_Set_High(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

/**
 * @brief Set EN (PA3) LOW
 */
static inline void Enable_Set_Low(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

#endif /* SPI_GPIO_CONFIG_H */
