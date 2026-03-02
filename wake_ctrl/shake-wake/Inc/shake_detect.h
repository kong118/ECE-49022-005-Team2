#ifndef SHAKE_DETECT_H
#define SHAKE_DETECT_H

#include <stdint.h>
#include "adxl362.h"

typedef struct {
    uint16_t activity_threshold;  /* Activity threshold in mg */
    uint8_t activity_time;        /* Time for activity detection */
    uint8_t is_awake;             /* Flag indicating wake event */
    uint32_t last_wake_time;      /* Timestamp of last wake event */
} ShakeDetect_Config_t;

/* Function Prototypes */
void ShakeDetect_Init(uint16_t threshold);
void ShakeDetect_Process(void);
void ShakeDetect_SetThreshold(uint16_t threshold);
uint16_t ShakeDetect_GetThreshold(void);
uint8_t ShakeDetect_IsAwake(void);
void ShakeDetect_ClearAwake(void);
void ShakeDetect_PrintAccelData(ADXL362_Data_t *data);

#endif /* SHAKE_DETECT_H */
