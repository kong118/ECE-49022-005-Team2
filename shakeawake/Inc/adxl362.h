#ifndef ADXL362_H
#define ADXL362_H

#include <stdint.h>
#include <stdbool.h>

/* ADXL362 Register Addresses */
#define ADXL362_REG_DEVID           0x00
#define ADXL362_REG_THRESH_ACT      0x20
#define ADXL362_REG_TIME_ACT        0x21
#define ADXL362_REG_THRESH_INACT    0x23
#define ADXL362_REG_TIME_INACT      0x24
#define ADXL362_REG_ACT_INACT_CTL   0x27
#define ADXL362_REG_FIFO_CONTROL    0x28
#define ADXL362_REG_INT1_MAP        0x2A
#define ADXL362_REG_INT2_MAP        0x2B
#define ADXL362_REG_FILTER_CTL      0x2C
#define ADXL362_REG_POWER_CTL       0x2D
#define ADXL362_REG_STATUS          0x0B
#define ADXL362_REG_XDATA           0x08
#define ADXL362_REG_YDATA           0x09
#define ADXL362_REG_ZDATA           0x0A
#define ADXL362_REG_XDATAL          0x0E
#define ADXL362_REG_XDATAH          0x0F
#define ADXL362_REG_YDATAL          0x10
#define ADXL362_REG_YDATAH          0x11
#define ADXL362_REG_ZDATAL          0x12
#define ADXL362_REG_ZDATAH          0x13

/* ADXL362 Commands */
#define ADXL362_CMD_READ            0x0B
#define ADXL362_CMD_WRITE           0x0A

/* Device ID */
#define ADXL362_DEVID               0xF2

/* Data structure for XYZ acceleration */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} ADXL362_Data_t;

/* Function prototypes */
void ADXL362_Init(uint16_t threshold_mg);
void ADXL362_ReadData(ADXL362_Data_t *data);
uint8_t ADXL362_GetStatus(void);
void ADXL362_WriteReg(uint8_t reg, uint8_t value);
uint8_t ADXL362_ReadReg(uint8_t reg);
float ADXL362_ConvertToG(int16_t raw);
void ADXL362_SetThreshold(uint16_t threshold_mg);

#endif /* ADXL362_H */
