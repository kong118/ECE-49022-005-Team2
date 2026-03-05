#ifndef CONFIG_H
#define CONFIG_H

#include "stm32l4xx_hal.h"

/* ========================================================================
 * System Configuration Structure
 * ======================================================================== */
typedef struct {
    uint16_t activity_threshold_mg;      /* Activity wake threshold in mg */
    uint16_t output_pulse_duration_ms;   /* Duration OUTPUT and EN stay HIGH (ms) */
} system_config_t;

/* ========================================================================
 * Extern Global Variables
 * ======================================================================== */
extern system_config_t sys_config;

/**
 * Wake event flag - set by EXTI interrupt handler
 * Indicates INT2 rising edge was detected
 */
extern volatile uint8_t wake_event_flag;

/* ========================================================================
 * Default Configuration Values
 * ======================================================================== */
#define DEFAULT_ACTIVITY_THRESHOLD_MG      500    /* 500 mg wake threshold */
#define DEFAULT_OUTPUT_PULSE_DURATION_MS   500    /* 500 ms OUTPUT/EN pulse */

/* ========================================================================
 * SPI Port/Pin Definitions
 * ======================================================================== */
#define SPI_PORT                GPIOB
#define SPI_SCLK_PIN            GPIO_PIN_3   /* PB3 - D13 */
#define SPI_MISO_PIN            GPIO_PIN_4   /* PB4 - D12 */
#define SPI_MOSI_PIN            GPIO_PIN_5   /* PB5 - D11 */

/* ========================================================================
 * ADXL362 Control Pin Definitions
 * ======================================================================== */
#define ADXL362_CS_PORT         GPIOB
#define ADXL362_CS_PIN          GPIO_PIN_0   /* PB0 - D3, Active LOW */

#define ADXL362_INT2_PORT       GPIOB
#define ADXL362_INT2_PIN        GPIO_PIN_6   /* PB6 - D5, Rising edge interrupt */

/* ========================================================================
 * Output/Enable Pin Definitions
 * ======================================================================== */
#define OUTPUT_PORT             GPIOA
#define OUTPUT_PIN              GPIO_PIN_7   /* PA7 - A6, Active HIGH */

#define ENABLE_PORT             GPIOA
#define ENABLE_PIN              GPIO_PIN_3   /* PA3 - A2, Active HIGH */

/* ========================================================================
 * Configuration Functions
 * ======================================================================== */

/**
 * @brief Initialize system configuration with default values
 */
void Config_Init(void);

/**
 * @brief Set the activity wake threshold at runtime
 * @param threshold_mg  Activity threshold in milliG
 */
void Config_SetActivityThreshold(uint16_t threshold_mg);

/**
 * @brief Get current activity threshold
 * @return Current threshold in mg
 */
uint16_t Config_GetActivityThreshold(void);

/**
 * @brief Set output pulse duration at runtime
 * @param duration_ms  Pulse duration in milliseconds
 */
void Config_SetOutputDuration(uint16_t duration_ms);

/**
 * @brief Get current output pulse duration
 * @return Current duration in ms
 */
uint16_t Config_GetOutputDuration(void);

#endif /* CONFIG_H */
