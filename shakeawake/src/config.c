#include "config.h"
#include "adxl362_lowpower.h"

/* ========================================================================
 * Global Configuration Instance
 * ======================================================================== */

system_config_t sys_config = {
    .activity_threshold_mg = DEFAULT_ACTIVITY_THRESHOLD_MG,
    .output_pulse_duration_ms = DEFAULT_OUTPUT_PULSE_DURATION_MS
};

/* ========================================================================
 * Global Wake Event Flag
 * ======================================================================== */

volatile uint8_t wake_event_flag = 0;

/* ========================================================================
 * Configuration Functions
 * ======================================================================== */

void Config_Init(void)
{
    /* Initialize with default values */
    sys_config.activity_threshold_mg = DEFAULT_ACTIVITY_THRESHOLD_MG;
    sys_config.output_pulse_duration_ms = DEFAULT_OUTPUT_PULSE_DURATION_MS;
}

void Config_SetActivityThreshold(uint16_t threshold_mg)
{
    sys_config.activity_threshold_mg = threshold_mg;
    /* Also update ADXL362 hardware configuration */
    ADXL362_SetWakeThreshold_mg(threshold_mg);
}

uint16_t Config_GetActivityThreshold(void)
{
    return sys_config.activity_threshold_mg;
}

void Config_SetOutputDuration(uint16_t duration_ms)
{
    sys_config.output_pulse_duration_ms = duration_ms;
}

uint16_t Config_GetOutputDuration(void)
{
    return sys_config.output_pulse_duration_ms;
}
