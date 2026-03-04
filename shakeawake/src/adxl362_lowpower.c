#include "adxl362_lowpower.h"
#include "spi_gpio_config.h"
#include "config.h"
#include "stm32l4xx_hal.h"

/*===================== ADXL362 Initialization =====================*/
void ADXL362_Init(void)
{
    uint8_t chip_id;
    
    /* Read and verify device ID */
    ADXL362_Read_Register(ADXL362_DEVID_AD, &chip_id);
    if (chip_id != 0xAD) {
        /* Device ID mismatch - initialization failed */
        while(1);
    }
}

/*===================== Setup Wake-up Mode =====================*/
void ADXL362_Init_Wakeup_Mode(void)
{
    /* 1. Configure Filter Control Register - Set 100Hz data rate and 2g range */
    ADXL362_Write_Register(ADXL362_FILTER_CTL, 0x00 | ADXL362_ODR_100HZ);
    
    /* 2. Set Activity Threshold */
    ADXL362_Set_Activity_Threshold(sys_config.activity_threshold_mg);
    
    /* 3. Set Inactivity Threshold */
    ADXL362_Set_Inactivity_Threshold(500);  /* 500 mg */
    
    /* 4. Set Inactivity Time */
    ADXL362_Write_Register(ADXL362_TIME_INACT_L, 0x01);
    ADXL362_Write_Register(ADXL362_TIME_INACT_H, 0x00);  /* 1 second */
    
    /* 5. Configure Activity/Inactivity Control */
    ADXL362_Write_Register(ADXL362_ACT_INACT_CTL, 0x77);  /* Enable activity and inactivity on X,Y,Z */
    
    /* 6. Configure Interrupt Mapping - Activity to INT2 */
    ADXL362_Write_Register(ADXL362_INT2_MAP, 0x10);  /* Activity interrupt mapped to INT2 */
    
    /* 7. Enter Wake-up Mode (Low Power) */
    ADXL362_Write_Register(ADXL362_POWER_CTL, ADXL362_WAKEUP);  /* Enable Wake-up mode */
    
    /* Delay for stabilization */
    HAL_Delay(100);
}

/*===================== Register Read/Write =====================*/
void ADXL362_Read_Register(uint8_t addr, uint8_t *data)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    
    tx_buf[0] = ADXL362_CMD_READ;
    tx_buf[1] = addr;
    tx_buf[2] = 0x00;
    
    SPI_Write_Read(tx_buf, rx_buf, 3);
    *data = rx_buf[2];
}

void ADXL362_Write_Register(uint8_t addr, uint8_t data)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    
    tx_buf[0] = ADXL362_CMD_WRITE;
    tx_buf[1] = addr;
    tx_buf[2] = data;
    
    SPI_Write_Read(tx_buf, rx_buf, 3);
}

void ADXL362_Read_Multiple(uint8_t addr, uint8_t *data, uint8_t count)
{
    uint8_t i;
    uint8_t tx_buf[258];
    uint8_t rx_buf[258];
    
    tx_buf[0] = ADXL362_CMD_READ;
    tx_buf[1] = addr;
    
    for (i = 0; i < count; i++) {
        tx_buf[2 + i] = 0x00;
    }
    
    SPI_Write_Read(tx_buf, rx_buf, count + 2);
    
    for (i = 0; i < count; i++) {
        data[i] = rx_buf[2 + i];
    }
}

/*===================== Data Reading =====================*/
void ADXL362_Get_Acceleration(accel_data_t *accel)
{
    uint8_t data[6];
    
    /* Read 6 bytes starting from XDATA_L */
    ADXL362_Read_Multiple(ADXL362_XDATA_L, data, 6);
    
    /* Combine bytes to form 16-bit signed values */
    accel->x = (int16_t)((data[1] << 8) | data[0]);
    accel->y = (int16_t)((data[3] << 8) | data[2]);
    accel->z = (int16_t)((data[5] << 8) | data[4]);
    
    /* Shift to 12-bit format (ADXL362 uses 12-bit data) */
    accel->x = accel->x >> 4;
    accel->y = accel->y >> 4;
    accel->z = accel->z >> 4;
}

/*===================== Status Reading =====================*/
uint8_t ADXL362_Get_Status(void)
{
    uint8_t status;
    ADXL362_Read_Register(ADXL362_STATUS, &status);
    return status;
}

/*===================== Magnitude Calculation =====================*/
uint32_t ADXL362_Get_Magnitude(accel_data_t *accel)
{
    int32_t x = (int32_t)accel->x;
    int32_t y = (int32_t)accel->y;
    int32_t z = (int32_t)accel->z;
    
    /* Calculate magnitude: sqrt(x^2 + y^2 + z^2) */
    uint32_t mag_squared = (x * x) + (y * y) + (z * z);
    
    /* Integer square root approximation */
    uint32_t result = 0;
    uint32_t bit = 1 << 15;
    
    while (bit > mag_squared) {
        bit >>= 1;
    }
    
    while (bit != 0) {
        if (mag_squared >= result + bit) {
            mag_squared -= result + bit;
            result += (bit << 1);
        }
        bit >>= 1;
    }
    
    return (result >> 1);
}

/*===================== Threshold Configuration =====================*/
void ADXL362_Set_Activity_Threshold(uint16_t threshold_mg)
{
    uint8_t thresh_l = (uint8_t)(threshold_mg & 0xFF);
    uint8_t thresh_h = (uint8_t)((threshold_mg >> 8) & 0x0F);
    
    ADXL362_Write_Register(ADXL362_THRESH_ACT_L, thresh_l);
    ADXL362_Write_Register(ADXL362_THRESH_ACT_H, thresh_h);
}

void ADXL362_Set_Inactivity_Threshold(uint16_t threshold_mg)
{
    uint8_t thresh_l = (uint8_t)(threshold_mg & 0xFF);
    uint8_t thresh_h = (uint8_t)((threshold_mg >> 8) & 0x0F);
    
    ADXL362_Write_Register(ADXL362_THRESH_INACT_L, thresh_l);
    ADXL362_Write_Register(ADXL362_THRESH_INACT_H, thresh_h);
}

void ADXL362_Set_Data_Rate(uint8_t odr)
{
    uint8_t filter_val;
    ADXL362_Read_Register(ADXL362_FILTER_CTL, &filter_val);
    
    /* Clear ODR bits (bits 3-5) and set new ODR */
    filter_val &= 0xC7;  /* Clear bits 3-5 */
    filter_val |= (odr << 3);
    
    ADXL362_Write_Register(ADXL362_FILTER_CTL, filter_val);
}

void ADXL362_Configure_Wakeup_Mode(uint16_t activity_threshold_mg)
{
    ADXL362_Init_Wakeup_Mode();
    ADXL362_Set_Activity_Threshold(activity_threshold_mg);
}

/*===================== Power Mode Control =====================*/
void ADXL362_Enter_Wakeup_Mode(void)
{
    /* Set Wake-up mode in Power Control Register */
    ADXL362_Write_Register(ADXL362_POWER_CTL, ADXL362_WAKEUP);
}

void ADXL362_Exit_Wakeup_Mode(void)
{
    /* Enable full measurement mode */
    ADXL362_Write_Register(ADXL362_POWER_CTL, ADXL362_MEASURE);
}
