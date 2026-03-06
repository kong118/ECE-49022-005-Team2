/* ========================================================================
 * ADXL362 Low-Power Accelerometer Driver
 * 
 * This module provides SPI communication with ADXL362 accelerometer
 * configured for activity (shake) wake detection.
 * 
 * Key Features:
 *   - Soft reset on initialization
 *   - Activity detection with configurable threshold
 *   - INT2 interrupt mapped to activity event
 *   - Measurement mode for continuous monitoring
 *   - Runtime threshold adjustment API
 * ======================================================================== */

#include "adxl362_lowpower.h"
#include "spi_gpio_config.h"

/* ========================================================================
 * Internal SPI Helper Functions
 * ======================================================================== */

/**
 * @brief Perform SPI transaction with manual CS control
 * Per ADXL362 datasheet, SPI format is:
 *   [Command Byte] [Address Byte] [Data Bytes...]
 *   CS must be held LOW throughout entire transaction
 */
static HAL_StatusTypeDef SPI_Transaction(uint8_t *tx_data, uint8_t *rx_data, uint16_t length)
{
    ADXL362_CS_Low();
    
    /* Send data and receive response */
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, length, HAL_MAX_DELAY);
    
    ADXL362_CS_High();
    
    return status;
}

/* ========================================================================
 * Register Access Functions
 * ======================================================================== */

HAL_StatusTypeDef ADXL362_WriteByte(uint8_t regAddr, uint8_t regValue)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    
    /* ADXL362 SPI write format: [0x0A=WRITE_CMD] [RegisterAddress] [Data] */
    tx_buf[0] = ADXL362_WRITE_CMD;
    tx_buf[1] = regAddr;
    tx_buf[2] = regValue;
    
    return SPI_Transaction(tx_buf, rx_buf, 3);
}

HAL_StatusTypeDef ADXL362_ReadByte(uint8_t regAddr, uint8_t *pRegValue)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    
    /* ADXL362 SPI read format: [0x0B=READ_CMD] [RegisterAddress] [0x00] */
    /* Received data is in rx_buf[2] */
    tx_buf[0] = ADXL362_READ_CMD;
    tx_buf[1] = regAddr;
    tx_buf[2] = 0x00;
    
    HAL_StatusTypeDef status = SPI_Transaction(tx_buf, rx_buf, 3);
    
    if (status == HAL_OK) {
        *pRegValue = rx_buf[2];
    }
    
    return status;
}

HAL_StatusTypeDef ADXL362_WriteBuffer(uint8_t regAddr, uint8_t *pData, uint8_t len)
{
    uint8_t tx_buf[258];
    uint8_t rx_buf[258];
    uint8_t i;
    
    if (len > 256) {
        return HAL_ERROR;
    }
    
    /* ADXL362 SPI write format: [0x0A=WRITE_CMD] [RegisterAddress] [Data1] [Data2] ... */
    tx_buf[0] = ADXL362_WRITE_CMD;
    tx_buf[1] = regAddr;
    
    for (i = 0; i < len; i++) {
        tx_buf[2 + i] = pData[i];
    }
    
    return SPI_Transaction(tx_buf, rx_buf, (2 + len));
}

HAL_StatusTypeDef ADXL362_ReadBuffer(uint8_t regAddr, uint8_t *pData, uint8_t len)
{
    uint8_t tx_buf[258];
    uint8_t rx_buf[258];
    uint8_t i;
    
    if (len > 256) {
        return HAL_ERROR;
    }
    
    /* ADXL362 SPI read format: [0x0B=READ_CMD] [RegisterAddress] [0x00] [0x00] ... */
    /* Received data starts at rx_buf[2] */
    tx_buf[0] = ADXL362_READ_CMD;
    tx_buf[1] = regAddr;
    
    for (i = 0; i < len; i++) {
        tx_buf[2 + i] = 0x00;
    }
    
    HAL_StatusTypeDef status = SPI_Transaction(tx_buf, rx_buf, (2 + len));
    
    if (status == HAL_OK) {
        for (i = 0; i < len; i++) {
            pData[i] = rx_buf[2 + i];
        }
    }
    
    return status;
}

/* ========================================================================
 * ADXL362 Initialization and Configuration
 * ======================================================================== */

/**
 * @brief Convert milliG threshold to ADXL362 register value
 * 
 * ADXL362 ACT_THRESH register resolution (per datasheet Table 1):
 *   In +/-2g range: 1 mg/LSB
 *   Register is 11-bit (0-2047)
 * 
 * So threshold_mg maps DIRECTLY to register value (1:1).
 * 
 * @param threshold_mg  Threshold in milliG
 * @return Register value (0-2047)
 */
static uint16_t Convert_mg_to_register(uint16_t threshold_mg)
{
    /* 1 mg/LSB in +/-2g range, so register value = threshold_mg directly */
    uint16_t reg_val = threshold_mg;
    
    /* Clamp to 11-bit range (0-2047) */
    if (reg_val > 2047) {
        reg_val = 2047;
    }
    
    return reg_val;
}

HAL_StatusTypeDef ADXL362_Init(uint16_t threshold_mg)
{
    uint8_t device_id = 0;
    uint8_t readback = 0;
    HAL_StatusTypeDef status;
    uint16_t reg_value;
    uint8_t thresh_low, thresh_high;
    
    /* ===== Step 1: Soft Reset ===== */
    status = ADXL362_WriteByte(ADXL362_REG_SOFT_RESET, 0x52);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }
    
    /* Wait for reset to complete (datasheet says 0.5ms, use 10ms for safety) */
    HAL_Delay(10);
    
    /* ===== Step 2: Verify Part ID ===== */
    status = ADXL362_ReadByte(ADXL362_REG_PARTID, &device_id);
    if (status != HAL_OK || device_id != 0xF2) {
        return HAL_ERROR;
    }
    
    /* ===== All configuration done in STANDBY mode ===== */
    /* Device is in standby after reset, so no need to explicitly set it */
    
    /* ===== Step 3: Set Activity Threshold ===== */
    /* In +/-2g range: 1 mg/LSB, direct mapping */
    reg_value = Convert_mg_to_register(threshold_mg);
    thresh_low = (uint8_t)(reg_value & 0xFF);
    thresh_high = (uint8_t)((reg_value >> 8) & 0x07);
    
    status = ADXL362_WriteByte(ADXL362_REG_THRESH_ACT, thresh_low);
    if (status != HAL_OK) return HAL_ERROR;
    
    status = ADXL362_WriteByte(ADXL362_REG_THRESH_ACT + 1, thresh_high);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* Verify threshold was written correctly */
    ADXL362_ReadByte(ADXL362_REG_THRESH_ACT, &readback);
    if (readback != thresh_low) {
        return HAL_ERROR;
    }
    
    /* ===== Step 4: Set Activity Time (1 sample) ===== */
    status = ADXL362_WriteByte(ADXL362_REG_TIME_ACT, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* ===== Step 5: Configure Activity/Inactivity Control ===== */
    /* ACT_EN (bit 0) + ACT_REF (bit 1) = 0x03: referenced activity detection */
    status = ADXL362_WriteByte(ADXL362_REG_ACT_INACT_CTL, 
                               ADXL362_ACT_ENABLE | ADXL362_ACT_REF);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* ===== Step 6: Map ACTIVITY interrupt to BOTH INT1 and INT2 pins ===== */
    /* Map to both in case user wired INT1 instead of INT2 */
    status = ADXL362_WriteByte(ADXL362_REG_INTMAP1, ADXL362_INT_ACT);
    if (status != HAL_OK) return HAL_ERROR;
    status = ADXL362_WriteByte(ADXL362_REG_INTMAP2, ADXL362_INT_ACT);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* ===== Step 7: Enter Wake-Up Mode ===== */
    /* POWER_CTL: bit 3 = WAKEUP, bit 1 = MEASURE = 0x0A */
    status = ADXL362_WriteByte(ADXL362_REG_POWER_CTL, ADXL362_POWER_WAKEUP);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* Wait for measurement to start */
    HAL_Delay(20);
    
    /* Read and display STATUS to clear any pending interrupts */
    ADXL362_ReadByte(ADXL362_REG_STATUS, &readback);
    
    return HAL_OK;
}

HAL_StatusTypeDef ADXL362_SetWakeThreshold_mg(uint16_t threshold_mg)
{
    uint16_t reg_value;
    uint8_t thresh_low, thresh_high;
    HAL_StatusTypeDef status;
    
    /* Convert mg threshold to register value (1:1 in +/-2g range) */
    reg_value = Convert_mg_to_register(threshold_mg);
    
    thresh_low = (uint8_t)(reg_value & 0xFF);
    thresh_high = (uint8_t)((reg_value >> 8) & 0x07);
    
    /* Put device in standby before updating threshold */
    status = ADXL362_WriteByte(ADXL362_REG_POWER_CTL, ADXL362_POWER_STANDBY);
    if (status != HAL_OK) return HAL_ERROR;
    HAL_Delay(5);
    
    /* Write activity threshold */
    status = ADXL362_WriteByte(ADXL362_REG_THRESH_ACT, thresh_low);
    if (status != HAL_OK) return HAL_ERROR;
    
    status = ADXL362_WriteByte(ADXL362_REG_THRESH_ACT + 1, thresh_high);
    if (status != HAL_OK) return HAL_ERROR;
    
    /* Return to wake-up mode */
    status = ADXL362_WriteByte(ADXL362_REG_POWER_CTL, ADXL362_POWER_WAKEUP);
    if (status != HAL_OK) return HAL_ERROR;
    HAL_Delay(5);
    
    return HAL_OK;
}

HAL_StatusTypeDef ADXL362_ReadStatus(uint8_t *pStatus)
{
    /* Read STATUS register to check for activity event
     * Per ADXL362 datasheet, reading STATUS register also clears the interrupt
     */
    return ADXL362_ReadByte(ADXL362_REG_STATUS, pStatus);
}

HAL_StatusTypeDef ADXL362_ClearInterrupt(void)
{
    uint8_t status;
    
    /* 
     * Per ADXL362 datasheet section on interrupt clearing:
     * Reading the STATUS register clears ACTIVITY and other interrupt flags.
     * This is already done implicitly when we read STATUS to check the ACT bit.
     */
    return ADXL362_ReadByte(ADXL362_REG_STATUS, &status);
}
