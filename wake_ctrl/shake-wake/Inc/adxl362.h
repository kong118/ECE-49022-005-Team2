#ifndef ADXL362_H
#define ADXL362_H

#include <stdint.h>

/* ADXL362 Register Addresses */
#define ADXL362_REG_DEVID_AD    0x00
#define ADXL362_REG_DEVID_MST   0x01
#define ADXL362_REG_PARTID      0x02
#define ADXL362_REG_REVID       0x03
#define ADXL362_REG_XDATA       0x08
#define ADXL362_REG_YDATA       0x09
#define ADXL362_REG_ZDATA       0x0A
#define ADXL362_REG_STATUS      0x0B
#define ADXL362_REG_FIFO_L      0x0C
#define ADXL362_REG_FIFO_H      0x0D
#define ADXL362_REG_XDATA_L     0x0E
#define ADXL362_REG_XDATA_H     0x0F
#define ADXL362_REG_YDATA_L     0x10
#define ADXL362_REG_YDATA_H     0x11
#define ADXL362_REG_ZDATA_L     0x12
#define ADXL362_REG_ZDATA_H     0x13
#define ADXL362_REG_TEMP_L      0x14
#define ADXL362_REG_TEMP_H      0x15
#define ADXL362_REG_SOFT_RESET  0x1F
#define ADXL362_REG_THRESH_ACT_L    0x20
#define ADXL362_REG_THRESH_ACT_H    0x21
#define ADXL362_REG_TIME_ACT        0x22
#define ADXL362_REG_THRESH_INACT_L  0x23
#define ADXL362_REG_THRESH_INACT_H  0x24
#define ADXL362_REG_TIME_INACT_L    0x25
#define ADXL362_REG_TIME_INACT_H    0x26
#define ADXL362_REG_ACT_INACT_CTL   0x27
#define ADXL362_REG_FIFO_CTL        0x28
#define ADXL362_REG_FIFO_SAMPLES    0x29
#define ADXL362_REG_INTMAP1         0x2A
#define ADXL362_REG_INTMAP2         0x2B
#define ADXL362_REG_FILTER_CTL      0x2C
#define ADXL362_REG_POWER_CTL       0x2D

/* ADXL362 Commands */
#define ADXL362_CMD_WRITE   0x0A
#define ADXL362_CMD_READ    0x0B

/* ADXL362 Device IDs */
#define ADXL362_DEVID_AD    0xAD
#define ADXL362_DEVID_MST   0x1D
#define ADXL362_PARTID      0xF2

/* ACT_INACT Control Register Bits */
#define ADXL362_INT_ACTIVITY    0x01
#define ADXL362_INT_INACTIVITY  0x02

/* Intmap Register Bits */
#define ADXL362_INT2_ACT    0x01
#define ADXL362_INT2_INACT  0x02

/* Power Control Bits */
#define ADXL362_POWER_MEASURE   0x02

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} ADXL362_Data_t;

/* Function Prototypes */
void ADXL362_Init(void);
void ADXL362_ReadReg(uint8_t reg, uint8_t *data, uint16_t len);
void ADXL362_WriteReg(uint8_t reg, uint8_t *data, uint16_t len);
void ADXL362_ReadAccel(ADXL362_Data_t *accel_data);
uint8_t ADXL362_ReadStatus(void);
void ADXL362_SetActivityThreshold(uint16_t threshold);
void ADXL362_SetInactivityThreshold(uint16_t threshold);
void ADXL362_SetActivityTime(uint8_t time);
void ADXL362_SetInactivityTime(uint16_t time);
void ADXL362_EnableActivityInt(void);
void ADXL362_EnableInactivityInt(void);
void ADXL362_DisableActivityInt(void);
void ADXL362_DisableInactivityInt(void);
void ADXL362_ConfigureINT2(void);

#endif /* ADXL362_H */
