#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* ============== Global Configuration Structure ============== */
typedef struct {
    uint16_t activity_threshold_mg;      /* Activity detection threshold (mg) */
    uint16_t output_pulse_duration_ms;   /* How long to keep PA7 high (ms) */
    uint16_t inactivity_timeout_sec;     /* Time before entering STOP2 (seconds) */
    uint8_t enable_stop2_mode;           /* Enable STOP2 power saving mode */
    uint8_t data_rate_hz;                /* Output data rate: 100Hz */
} system_config_t;

/* ============== Extern Global Variables ============== */
extern system_config_t sys_config;
extern volatile uint32_t activity_detected_flag;
extern volatile uint32_t last_activity_time_ms;
extern volatile uint32_t output_active_time_ms;
extern volatile uint8_t output_is_active;

/* ============== Default Configuration Values ============== */
#define DEFAULT_ACTIVITY_THRESHOLD_MG        1500    /* 1500 mg */
#define DEFAULT_OUTPUT_PULSE_DURATION_MS     500     /* 500 ms */
#define DEFAULT_INACTIVITY_TIMEOUT_SEC       30      /* 30 seconds */
#define DEFAULT_ENABLE_STOP2_MODE            1       /* Enable STOP2 mode */
#define DEFAULT_DATA_RATE_HZ                 100     /* 100 Hz */

/* ============== Configuration Functions ============== */

void Config_Init(void);
void Set_Activity_Threshold(uint16_t threshold_mg);
void Set_Output_Duration(uint16_t duration_ms);
void Set_Inactivity_Timeout(uint16_t timeout_sec);
void Enable_STOP2_Mode(void);
void Disable_STOP2_Mode(void);

#endif // CONFIG_H
