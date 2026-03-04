#include "config.h"
#include "adxl362_lowpower.h"

/* Global configuration variable */
system_config_t sys_config = {
    .activity_threshold_mg = DEFAULT_ACTIVITY_THRESHOLD_MG,
    .output_pulse_duration_ms = DEFAULT_OUTPUT_PULSE_DURATION_MS,
    .inactivity_timeout_sec = DEFAULT_INACTIVITY_TIMEOUT_SEC,
    .enable_stop2_mode = DEFAULT_ENABLE_STOP2_MODE,
    .data_rate_hz = DEFAULT_DATA_RATE_HZ
};

/* Global state variables */
volatile uint32_t activity_detected_flag = 0;
volatile uint32_t last_activity_time_ms = 0;
volatile uint32_t output_active_time_ms = 0;
volatile uint8_t output_is_active = 0;

/*===================== Configuration Functions =====================*/

void Config_Init(void)
{
    sys_config.activity_threshold_mg = DEFAULT_ACTIVITY_THRESHOLD_MG;
    sys_config.output_pulse_duration_ms = DEFAULT_OUTPUT_PULSE_DURATION_MS;
    sys_config.inactivity_timeout_sec = DEFAULT_INACTIVITY_TIMEOUT_SEC;
    sys_config.enable_stop2_mode = DEFAULT_ENABLE_STOP2_MODE;
    sys_config.data_rate_hz = DEFAULT_DATA_RATE_HZ;
}

void Set_Activity_Threshold(uint16_t threshold_mg)
{
    sys_config.activity_threshold_mg = threshold_mg;
    ADXL362_Set_Activity_Threshold(threshold_mg);
}

void Set_Output_Duration(uint16_t duration_ms)
{
    sys_config.output_pulse_duration_ms = duration_ms;
}

void Set_Inactivity_Timeout(uint16_t timeout_sec)
{
    sys_config.inactivity_timeout_sec = timeout_sec;
}

void Enable_STOP2_Mode(void)
{
    sys_config.enable_stop2_mode = 1;
}

void Disable_STOP2_Mode(void)
{
    sys_config.enable_stop2_mode = 0;
}
