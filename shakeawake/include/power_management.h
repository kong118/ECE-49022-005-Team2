#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include "stm32l4xx_hal.h"

/* ========================================================================
 * System Clock Configuration
 * ======================================================================== */

/**
 * @brief Configure system clock for normal operation
 *   - MSI clock source
 *   - Suitable frequency for NUCLEO-L432KC
 */
void SystemClock_Config(void);

/* ========================================================================
 * Error Handler
 * ======================================================================== */

/**
 * @brief Error handler function
 * Called when a critical error occurs
 */
void Error_Handler(void);

#endif /* POWER_MANAGEMENT_H */
