#ifndef ADXL362_LOWPOWER_H
#define ADXL362_LOWPOWER_H

#include "stm32l4xx_hal.h"

/* ========================================================================
 * ADXL362 SPI Commands (per ADXL362 datasheet)
 * ======================================================================== */
#define ADXL362_READ_CMD    0x0B  /* Read command byte */
#define ADXL362_WRITE_CMD   0x0A  /* Write command byte */
#define ADXL362_FIFO_READ   0x0D  /* FIFO read command byte */

/* ========================================================================
 * ADXL362 Register Addresses (per datasheet)
 * ======================================================================== */
#define ADXL362_REG_DEVID           0x00  /* Device AD ID (should be 0xAD) */
#define ADXL362_REG_DEVID_MST       0x01  /* MEMS Device ID (should be 0x1D) */
#define ADXL362_REG_PARTID          0x02  /* Part ID (should be 0xF2) */
#define ADXL362_REG_THRESH_ACT      0x20  /* Activity threshold (2 bytes) */
#define ADXL362_REG_THRESH_INACT    0x23  /* Inactivity threshold (2 bytes) */
#define ADXL362_REG_TIME_INACT      0x25  /* Inactivity time (2 bytes) */
#define ADXL362_REG_ACT_INACT_CTL   0x27  /* Activity/Inactivity control */
#define ADXL362_REG_INTMAP1         0x2A  /* Interrupt Map 1 */
#define ADXL362_REG_INTMAP2         0x2B  /* Interrupt Map 2 */
#define ADXL362_REG_FILTER_CTL      0x2C  /* Filter control */
#define ADXL362_REG_POWER_CTL       0x2D  /* Power control */
#define ADXL362_REG_STATUS          0x0B  /* Status register */
#define ADXL362_REG_SOFT_RESET      0x1F  /* Soft Reset */

/* ========================================================================
 * ADXL362 Status Register bits
 * ======================================================================== */
#define ADXL362_STATUS_ACT          0x10  /* Activity event detected */
#define ADXL362_STATUS_INACT        0x08  /* Inactivity event detected */
#define ADXL362_STATUS_FIFO_RDY     0x01  /* FIFO ready */

/* ========================================================================
 * ADXL362 Power Control modes
 * ======================================================================== */
#define ADXL362_POWER_STANDBY       0x00
#define ADXL362_POWER_MEASUREMENT   0x02

/* ========================================================================
 * ADXL362 Activity/Inactivity Control bits
 * ======================================================================== */
#define ADXL362_ACT_ENABLE          0x01  /* Enable activity detection */
#define ADXL362_ACT_AC_MODE         0x00  /* AC-coupled (0) or DC-coupled (1) */
#define ADXL362_INACT_ENABLE        0x04  /* Enable inactivity detection */

/* ========================================================================
 * ADXL362 Interrupt Map Register bits
 * ======================================================================== */
#define ADXL362_INT_ACT             0x10  /* Map activity to INT pin */

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Initialize ADXL362 over SPI
 *   - Perform soft reset
 *   - Configure activity detection with wake threshold
 *   - Map ACTIVITY to INT2
 *   - Set INT2 as active-high push-pull
 *   - Enter measurement mode
 * @param threshold_mg  Activity threshold in milliG (e.g., 500 for 500 mg)
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_Init(uint16_t threshold_mg);

/**
 * @brief Set activity wake threshold at runtime
 *   - Converts mg to ADXL362 register units (LSB_ACT_THRESH ≈ 62.5 mg)
 *   - Places ADXL362 in standby, updates threshold, returns to measurement mode
 * @param threshold_mg  Activity threshold in milliG (e.g., 500 for 500 mg)
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_SetWakeThreshold_mg(uint16_t threshold_mg);

/**
 * @brief Read status register to confirm activity event
 * @param pStatus  Pointer to store status byte
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_ReadStatus(uint8_t *pStatus);

/**
 * @brief Acknowledge/clear INT2 interrupt on ADXL362
 *   - Per ADXL362 datasheet, reading STATUS clears the interrupt
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_ClearInterrupt(void);

/* ========================================================================
 * SPI Helper Functions (internal)
 * ======================================================================== */

/**
 * @brief Write a single byte to ADXL362 register
 * @param regAddr   Register address
 * @param regValue  Byte value to write
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_WriteByte(uint8_t regAddr, uint8_t regValue);

/**
 * @brief Read a single byte from ADXL362 register
 * @param regAddr   Register address
 * @param pRegValue Pointer to store read value
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_ReadByte(uint8_t regAddr, uint8_t *pRegValue);

/**
 * @brief Write multiple bytes to ADXL362 (follows SPI command format)
 * @param regAddr   Starting register address
 * @param pData     Pointer to data buffer
 * @param len       Number of bytes to write
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_WriteBuffer(uint8_t regAddr, uint8_t *pData, uint8_t len);

/**
 * @brief Read multiple bytes from ADXL362 (follows SPI command format)
 * @param regAddr   Starting register address
 * @param pData     Pointer to data buffer
 * @param len       Number of bytes to read
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef ADXL362_ReadBuffer(uint8_t regAddr, uint8_t *pData, uint8_t len);

#endif /* ADXL362_LOWPOWER_H */
