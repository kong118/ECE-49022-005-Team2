#include "../Inc/shake_detect.h"
#include "../Inc/stm32l432xx.h"

static ShakeDetect_Config_t shake_config = {
    .activity_threshold = 100,
    .activity_time = 10,
    .is_awake = 0,
    .last_wake_time = 0
};

static volatile uint32_t systick_counter = 0;

/**
 * @brief SysTick interrupt handler
 */
void SysTick_Handler(void)
{
    systick_counter++;
}

/**
 * @brief Get system time in milliseconds
 */
static uint32_t GetSysTime(void)
{
    return systick_counter;
}

/**
 * @brief Initialize shake detection
 */
void ShakeDetect_Init(uint16_t threshold)
{
    /* Initialize ADXL362 */
    ADXL362_Init();

    /* Set activity threshold */
    shake_config.activity_threshold = threshold;
    ADXL362_SetActivityThreshold(threshold);

    /* Set activity time (in 10ms units, so 10 = 100ms) */
    shake_config.activity_time = 10;
    ADXL362_SetActivityTime(shake_config.activity_time);

    /* Configure INT2 for activity interrupt */
    ADXL362_ConfigureINT2();

    /* Enable activity interrupt */
    ADXL362_EnableActivityInt();

    /* Clear wake flag */
    shake_config.is_awake = 0;

    /* Setup SysTick for timing (1ms tick) */
    SysTick_Config(SystemCoreClock / 1000);
}

/**
 * @brief Process shake detection
 * This function should be called regularly (e.g., in main loop)
 */
void ShakeDetect_Process(void)
{
    uint8_t status;
    ADXL362_Data_t accel_data;

    /* Read status register */
    status = ADXL362_ReadStatus();

    /* Check if activity interrupt occurred */
    if(status & 0x01)  /* Activity bit */
    {
        /* Read acceleration data */
        ADXL362_ReadAccel(&accel_data);

        /* Calculate magnitude of acceleration */
        int32_t accel_mag = 0;
        accel_mag += (int32_t)accel_data.x * (int32_t)accel_data.x;
        accel_mag += (int32_t)accel_data.y * (int32_t)accel_data.y;
        accel_mag += (int32_t)accel_data.z * (int32_t)accel_data.z;

        /* If magnitude exceeds threshold, set wake flag */
        if(accel_mag > ((int32_t)shake_config.activity_threshold * (int32_t)shake_config.activity_threshold))
        {
            shake_config.is_awake = 1;
            shake_config.last_wake_time = GetSysTime();
        }
    }
}

/**
 * @brief Set activity threshold
 */
void ShakeDetect_SetThreshold(uint16_t threshold)
{
    shake_config.activity_threshold = threshold;
    ADXL362_SetActivityThreshold(threshold);
}

/**
 * @brief Get current activity threshold
 */
uint16_t ShakeDetect_GetThreshold(void)
{
    return shake_config.activity_threshold;
}

/**
 * @brief Check if shake awake detected
 */
uint8_t ShakeDetect_IsAwake(void)
{
    return shake_config.is_awake;
}

/**
 * @brief Clear awake flag
 */
void ShakeDetect_ClearAwake(void)
{
    shake_config.is_awake = 0;
}

/**
 * @brief Print acceleration data (for debugging)
 */
void ShakeDetect_PrintAccelData(ADXL362_Data_t *data)
{
    /* This is a placeholder for printing/logging
       In a real implementation, you might use UART for debugging
       For now, we just store the data */
    (void)data;
}
