#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <stdint.h>
#include "stm32l432xx.h"

/* ============== Power Modes ============== */
typedef enum {
    POWER_MODE_RUN,         /* Normal operation */
    POWER_MODE_STOP2        /* STOP2 Sleep Mode - Ultra low power */
} power_mode_t;

/* ============== Sleep Configuration ============== */
typedef struct {
    uint8_t enable_sleep;           /* Enable STOP2 mode */
    uint16_t inactivity_timeout_sec; /* Seconds before sleep */
    uint16_t wake_threshold_mg;     /* Activity threshold to wake */
} sleep_config_t;

/* ============== Function Prototypes ============== */

/* System Clock Configuration */
void SystemClock_Config(void);
void SystemClock_Config_Stop2(void);

/* Power Management */
void Power_Management_Init(void);
void Enter_STOP2_Mode(void);
void Exit_STOP2_Mode(void);
void Enter_Run_Mode(void);

/* Sleep/Wake Control */
void Enable_Sleep_Mode(uint16_t inactivity_timeout_sec);
void Disable_Sleep_Mode(void);
void Check_Sleep_Timeout(void);

/* Interrupt Handling */
void EXTI4_15_IRQ_Handler(void);

/* Wake-up Control */
uint8_t Is_System_Awake(void);
void System_Wakeup_From_INT2(void);

#endif // POWER_MANAGEMENT_H
