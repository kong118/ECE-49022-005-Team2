#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"
#include "adxl362_lowpower.h"
#include "spi_gpio_config.h"
#include "power_management.h"
#include "config.h"

/*===================== Output Management Functions =====================*/

void Output_Check_And_Update(void)
{
    if (output_is_active) {
        uint32_t elapsed = HAL_GetTick() - output_active_time_ms;
        
        if (elapsed >= sys_config.output_pulse_duration_ms) {
            Output_Set_Low();
            output_is_active = 0;
        }
    }
}

void Output_Trigger(void)
{
    Output_Set_High();
    output_active_time_ms = HAL_GetTick();
    output_is_active = 1;
}

/*===================== Main Application =====================*/

int main(void)
{
    accel_data_t accel_data;
    uint32_t magnitude;
    
    /* ===== Initialization Phase ===== */
    
    /* Initialize HAL */
    HAL_Init();
    
    /* Configure system clock */
    SystemClock_Config();
    
    /* Initialize configuration */
    Config_Init();
    
    /* Initialize GPIO (INT2 and output pin) */
    GPIO_Init();
    Output_Pin_Init();
    
    /* Initialize SPI interface */
    SPI_Init();
    
    /* Initialize ADXL362 */
    ADXL362_Init();
    
    /* Initialize ADXL362 in Wake-up mode (100Hz, activity detection) */
    ADXL362_Init_Wakeup_Mode();
    
    /* Initialize power management and STOP2 mode */
    Power_Management_Init();
    
    /* Initialize INT2 interrupt for wake-up */
    INT2_Interrupt_Init();
    
    /* ===== Startup LED Blink Test (3 times to confirm MCU is working) ===== */
    {
        uint8_t blink_count;
        for (blink_count = 0; blink_count < 3; blink_count++) {
            Output_Set_High();
            HAL_Delay(2000);  /* 2 seconds high - longer for testing */
            Output_Set_Low();
            HAL_Delay(1000);  /* 1 second low */
        }
    }
    
    /* Enable STOP2 sleep mode with 30 second inactivity timeout */
    Enable_Sleep_Mode(30);
    
    /* Quick Configuration Options (Uncomment as needed) */
    
    /* Modify activity threshold if needed */
    Set_Activity_Threshold(120);      //Set to 120 mg
    
    /* Modify output duration if needed */
    Set_Output_Duration(800);          //Set to 800 ms
    
    /* Modify inactivity timeout if needed */
    /* Set_Inactivity_Timeout(60);        Set to 60 seconds */
    
    /* Main Loop */
    
    while (1)
    {
        /* Check if activity was detected via INT2 interrupt */
        if (activity_detected_flag) {
            activity_detected_flag = 0;
            
            /* Only read sensor if system is awake */
            if (Is_System_Awake()) {
                ADXL362_Get_Acceleration(&accel_data);
                magnitude = ADXL362_Get_Magnitude(&accel_data);
                
                /* Check if magnitude exceeds threshold */
                if (magnitude >= sys_config.activity_threshold_mg) {
                    /* Trigger output pulse on PA7 */
                    Output_Trigger();
                }
            }
        }
        
        /* Update output state (turn off after duration expires) */
        Output_Check_And_Update();
        
        /* Check for inactivity timeout and enter STOP2 if needed */
        Check_Sleep_Timeout();
        
        /* Small delay to prevent excessive CPU usage */
        HAL_Delay(10);  /* 10ms delay */
    }
    
    return 0;
}

