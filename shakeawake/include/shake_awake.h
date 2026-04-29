/* ========================================================================
 * Shake-Awake Module for Verdin Power Controller
 * 
 * This module handles motion detection from ADXL362 accelerometer
 * and generates wake pulses for the Verdin iMX8M Plus module.
 * 
 * Hardware connections:
 *   - ADXL362 INT2 -> STM32 PB6 (EXTI interrupt)
 *   - Verdin wake -> STM32 PA6 -> Q1 2N7002 -> Verdin control line
 * 
 * ======================================================================== */

#ifndef SHAKE_AWAKE_H
#define SHAKE_AWAKE_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stddef.h>

/* ========================================================================
 * Board Configuration Macros
 * ======================================================================== */

/* ADXL362 SPI Chip Select */
#define ADXL362_CS_GPIO_Port          GPIOB
#define ADXL362_CS_Pin                GPIO_PIN_0

/* ADXL362 Interrupt Pins */
#define ADXL362_INT1_GPIO_Port        GPIOB
#define ADXL362_INT1_Pin              GPIO_PIN_1

#define ADXL362_INT2_GPIO_Port        GPIOB
#define ADXL362_INT2_Pin              GPIO_PIN_6

/* Verdin Wake Control (PA6 -> Q1 2N7002 -> Verdin control line) */
#define VERDIN_WAKE_CTRL_GPIO_Port    GPIOA
#define VERDIN_WAKE_CTRL_Pin          GPIO_PIN_6

/* Timing Configuration */
#define VERDIN_WAKE_PULSE_MS          500U    /* Pulse width for Verdin wake */
#define SHAKE_AWAKE_LOCKOUT_MS        2000U   /* Lockout between wake pulses */

/* ADXL362 Activity Detection Configuration */
#define ADXL362_ACTIVITY_THRESHOLD_MG 300U    /* Threshold in milliG (300mg default) */
#define ADXL362_ACTIVITY_TIME_SAMPLES 2U      /* Activity time in samples */

/* Low Power Configuration */
#define ENABLE_LOW_POWER_AFTER_SHAKE_AWAKE 1

/* ========================================================================
 * ADXL362 Register Definitions (per datasheet)
 * ======================================================================== */

/* Device ID Registers */
#define ADXL362_REG_DEVID_AD          0x00    /* Device ID (should be 0xAD) */
#define ADXL362_REG_DEVID_MST         0x01    /* MEMS Device ID (should be 0x1D) */
#define ADXL362_REG_PARTID            0x02    /* Part ID (should be 0xF2) */
#define ADXL362_REG_REVID             0x03    /* Revision ID */

/* Status Register */
#define ADXL362_REG_STATUS            0x0B

/* Soft Reset */
#define ADXL362_REG_SOFT_RESET        0x1F

/* Activity Threshold Registers (16-bit, LSB first) */
#define ADXL362_REG_THRESH_ACT_L      0x20    /* Activity threshold LOW byte */
#define ADXL362_REG_THRESH_ACT_H      0x21    /* Activity threshold HIGH byte */

/* Activity Time Register */
#define ADXL362_REG_TIME_ACT          0x22

/* Inactivity Threshold Registers (16-bit, LSB first) */
#define ADXL362_REG_THRESH_INACT_L    0x23    /* Inactivity threshold LOW byte */
#define ADXL362_REG_THRESH_INACT_H    0x24    /* Inactivity threshold HIGH byte */

/* Inactivity Time Registers (16-bit, LSB first) */
#define ADXL362_REG_TIME_INACT_L      0x25    /* Inactivity time LOW byte */
#define ADXL362_REG_TIME_INACT_H      0x26    /* Inactivity time HIGH byte */

/* Activity/Inactivity Control */
#define ADXL362_REG_ACT_INACT_CTL     0x27

/* Interrupt Mapping Registers */
#define ADXL362_REG_INTMAP1           0x2A    /* Interrupt Map 1 */
#define ADXL362_REG_INTMAP2           0x2B    /* Interrupt Map 2 */

/* Filter Control */
#define ADXL362_REG_FILTER_CTL        0x2C

/* Power Control */
#define ADXL362_REG_POWER_CTL         0x2D

/* ADXL362 SPI Commands */
#define ADXL362_CMD_WRITE_REG         0x0A    /* Register write command */
#define ADXL362_CMD_READ_REG          0x0B    /* Register read command */
#define ADXL362_CMD_READ_FIFO         0x0D    /* FIFO read command */

/* ADXL362 Status Bits */
#define ADXL362_STATUS_ACT            0x10    /* Activity event detected */
#define ADXL362_STATUS_INACT          0x08    /* Inactivity event detected */
#define ADXL362_STATUS_FIFO_RDY       0x01    /* FIFO ready */

/* ADXL362 Power Control Modes */
#define ADXL362_POWER_STANDBY         0x00
#define ADXL362_POWER_MEASUREMENT     0x02
#define ADXL362_POWER_WAKEUP          0x0A    /* Wake-up mode + measurement */

/* ADXL362 Activity Control Bits */
#define ADXL362_ACT_ENABLE            0x01    /* Enable activity detection */
#define ADXL362_ACT_REF               0x02    /* Referenced activity mode */
#define ADXL362_ACT_AC_MODE           0x00    /* AC-coupled mode */
#define ADXL362_INACT_ENABLE          0x04    /* Enable inactivity detection */

/* ADXL362 Interrupt Map Bits */
#define ADXL362_INT_ACT               0x10    /* Map activity to INT pin */
#define ADXL362_INT_AWAKE             0x40    /* Map AWAKE status to INT pin */

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Initialize the shake-awake module
 * Configures ADXL362, GPIO, and EXTI for motion detection
 */
void shake_awake_init(void);

/**
 * @brief Arm the shake-awake system after initialization
 * Enables interrupts and prepares for motion detection
 */
void shake_awake_arm(void);

/**
 * @brief Handle EXTI interrupt from ADXL362
 * @param GPIO_Pin  The GPIO pin that triggered the interrupt
 * 
 * WARNING: Do not call blocking functions (HAL_Delay, etc.) in this callback!
 * Set flags only and handle processing in shake_awake_task()
 */
void shake_awake_on_exti(uint16_t GPIO_Pin);

/**
 * @brief Main loop task for shake-awake processing
 * Should be called periodically from main loop
 * Handles deferred interrupt processing and Verdin pulse generation
 */
void shake_awake_task(void);

/**
 * @brief Generate Verdin wake pulse through PA6 -> Q1 circuit
 * 
 * WARNING: Verdin control pins are 1.8V domain!
 * The Q1 2N7002 transistor provides level-shifting from 3.3V STM32
 * to 1.8V Verdin input. PA6 HIGH turns Q1 on, pulling the Verdin
 * control line LOW (active-low signal).
 * 
 * Keep pulse below 5 seconds to avoid Verdin force-off behavior
 * on CTRL_PWR_BTN_MICO#.
 */
void verdin_wake_pulse(void);

/**
 * @brief Read single byte from ADXL362 register
 * @param reg  Register address
 * @return Register value
 */
uint8_t adxl362_read_reg(uint8_t reg);

/**
 * @brief Write single byte to ADXL362 register
 * @param reg   Register address
 * @param value Value to write
 */
void adxl362_write_reg(uint8_t reg, uint8_t value);

/**
 * @brief Read multiple bytes from ADXL362 registers
 * @param start_reg  Starting register address
 * @param buf        Buffer to store read data
 * @param len        Number of bytes to read
 */
void adxl362_read_regs(uint8_t start_reg, uint8_t *buf, size_t len);

/**
 * @brief Set ADXL362 chip select LOW (active)
 */
void adxl362_cs_low(void);

/**
 * @brief Set ADXL362 chip select HIGH (inactive)
 */
void adxl362_cs_high(void);

/**
 * @brief Initialize ADXL362 for shake-awake operation
 * Configures activity detection, interrupt mapping, and power mode
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef adxl362_init_for_shake_awake(void);

#endif /* SHAKE_AWAKE_H */