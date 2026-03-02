#ifndef MCU_INIT_H
#define MCU_INIT_H

#include "stm32l432xx.h"

void SystemInit(void);
void SystemClock_Init(void);
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif /* MCU_INIT_H */
