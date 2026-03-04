#ifndef ADXL362_LOWPOWER_H
#define ADXL362_LOWPOWER_H

#include <stdint.h>
#include "stm32l432xx.h"

/*============== ADXL362 Registers ==============*/
#define ADXL362_DEVID_AD        0x00
#define ADXL362_DEVID_MST       0x01
#define ADXL362_PARTID          0x02
#define ADXL362_REVID           0x03
#define ADXL362_STATUS          0x0B
#define ADXL362_FIFO_STATUS     0x0C
#define ADXL362_XDATA_L         0x08
#define ADXL362_XDATA_H         0x09
#define ADXL362_YDATA_L         0x0A
#define ADXL362_YDATA_H         0x0B
#define ADXL362_ZDATA_L         0x0C
#define ADXL362_ZDATA_H         0x0D
#define ADXL362_TEMP_L          0x12
#define ADXL362_TEMP_H          0x13
#define ADXL362_POWER_CTL       0x2D
#define ADXL362_THRESH_ACT_L    0x20
#define ADXL362_THRESH_ACT_H    0x21
#define ADXL362_THRESH_INACT_L  0x23
#define ADXL362_THRESH_INACT_H  0x24
#define ADXL362_TIME_INACT_L    0x25
#define ADXL362_TIME_INACT_H    0x26
#define ADXL362_ACT_INACT_CTL   0x27
#define ADXL362_FIFO_CTL        0x28
#define ADXL362_INT1_MAP        0x2A
#define ADXL362_INT2_MAP        0x2B
#define ADXL362_FILTER_CTL      0x2C
#define ADXL362_RANGE_SEL       0x2C

/* Power Control Register Bits */
#define ADXL362_MEASURE         (1 << 1)      /* Measurement Mode */
#define ADXL362_AUTOSLEEP       (1 << 2)      /* Autosleep Enable */
#define ADXL362_WAKEUP          (1 << 3)      /* Wake-up Mode */

/* Filter Control Register */
#define ADXL362_RANGE_2G        0x00
#define ADXL362_RANGE_4G        0x01
#define ADXL362_RANGE_8G        0x02
#define ADXL362_ODR_100HZ       0x03          /* 100 Hz Output Data Rate */
#define ADXL362_ODR_200HZ       0x04
#define ADXL362_ODR_400HZ       0x05

/* Status Bits */
#define ADXL362_STATUS_ACT      (1 << 4)
#define ADXL362_STATUS_ERR      (1 << 0)

/* Activity/Inactivity Control */
#define ADXL362_ACT_ENABLE      0x07          /* Enable activity on X,Y,Z */
#define ADXL362_INACT_ENABLE    0x70          /* Enable inactivity on X,Y,Z */

/* SPI Commands */
#define ADXL362_CMD_WRITE       0x0A
#define ADXL362_CMD_READ        0x0B
#define ADXL362_CMD_FIFO_READ   0x0D

/*============== Data Structures ==============*/
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_data_t;

typedef struct {
    uint16_t activity_threshold_mg;    /* Activity threshold in mg */
    uint16_t inactivity_threshold_mg;  /* Inactivity threshold in mg */
    uint8_t inactivity_time_sec;       /* Inactivity time in seconds */
    uint16_t output_pulse_duration_ms; /* Output pulse duration */
} adxl362_config_t;

/*============== Function Prototypes ==============*/

/* Initialization */
void ADXL362_Init(void);
void ADXL362_Init_Wakeup_Mode(void);

/* Register Operations */
void ADXL362_Read_Register(uint8_t addr, uint8_t *data);
void ADXL362_Write_Register(uint8_t addr, uint8_t data);
void ADXL362_Read_Multiple(uint8_t addr, uint8_t *data, uint8_t count);

/* Data Reading */
void ADXL362_Get_Acceleration(accel_data_t *accel);
uint8_t ADXL362_Get_Status(void);
uint32_t ADXL362_Get_Magnitude(accel_data_t *accel);

/* Configuration */
void ADXL362_Set_Activity_Threshold(uint16_t threshold_mg);
void ADXL362_Set_Inactivity_Threshold(uint16_t threshold_mg);
void ADXL362_Set_Data_Rate(uint8_t odr);
void ADXL362_Configure_Wakeup_Mode(uint16_t activity_threshold_mg);

/* Power Mode */
void ADXL362_Enter_Wakeup_Mode(void);
void ADXL362_Exit_Wakeup_Mode(void);

#endif // ADXL362_LOWPOWER_H
