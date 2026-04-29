/* ========================================================================
 * Shake-Awake Module Implementation for Verdin Power Controller
 * 
 * This module handles motion detection from ADXL362 accelerometer
 * and generates wake pulses for the Verdin iMX8M Plus module.
 * 
 * Hardware connections:
 *   - ADXL362 INT2 -> STM32 PB6 (EXTI interrupt)
 *   - Verdin wake -> STM32 PA6 -> Q1 2N7002 -> Verdin control line
 * 
 * ======================================================================== */

#include "shake_awake.h"
#include "spi_gpio_config.h"
#include "adxl362_lowpower.h"
#include "stm32l4xx_hal.h"

/* ========================================================================
 * Private Variables
 * ======================================================================== */

/* Flag indicating shake interrupt pending */
static volatile uint8_t shake_irq_pending = 0;

/* Timestamp of last wake pulse */
static uint32_t last_wake_pulse_tick = 0;

/* Flag indicating shake-awake system is armed */
static uint8_t shake_awake_armed = 0;

/* ========================================================================
 * ADXL362 SPI Helper Functions
 * ======================================================================== */

/**
 * @brief Set ADXL362 chip select LOW (active)
 */
void adxl362_cs_low(void)
{
    HAL_GPIO_WritePin(ADXL362_CS_GPIO_Port, ADXL362_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Set ADXL362 chip select HIGH (inactive)
 */
void adxl362_cs_high(void)
{
    HAL_GPIO_WritePin(ADXL362_CS_GPIO_Port, ADXL362_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Read single byte from ADXL362 register
 */
uint8_t adxl362_read_reg(uint8_t reg)
{
    uint8_t tx_buf[3] = {ADXL362_CMD_READ_REG, reg, 0x00};
    uint8_t rx_buf[3] = {0};
    
    adxl362_cs_low();
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    adxl362_cs_high();
    
    return rx_buf[2];
}

/**
 * @brief Write single byte to ADXL362 register
 */
void adxl362_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx_buf[3] = {ADXL362_CMD_WRITE_REG, reg, value};
    uint8_t rx_buf[3] = {0};
    
    adxl362_cs_low();
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    adxl362_cs_high();
}

/**
 * @brief Read multiple bytes from ADXL362 registers
 */
void adxl362_read_regs(uint8_t start_reg, uint8_t *buf, size_t len)
{
    uint8_t i;
    uint8_t tx_buf[2] = {ADXL362_CMD_READ_REG, start_reg};
    uint8_t rx_buf[2] = {0};
    
    adxl362_cs_low();
    
    /* Send command and address */
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    
    /* Read data */
    for (i = 0; i < len; i++) {
        tx_buf[0] = 0x00;  /* Dummy byte to clock out data */
        HAL_SPI_TransmitReceive(&hspi1, tx_buf, &buf[i], 1, HAL_MAX_DELAY);
    }
    
    adxl362_cs_high();
}

/* ========================================================================
 * ADXL362 Initialization for Shake-Awake
 * ======================================================================== */

/**
 * @brief Initialize ADXL362 for shake-awake operation
 * 
 * This function configures the ADXL362 for activity detection
 * with interrupt generation on INT2 (PB6).
 * 
 * Default settings:
 *   - ±2g range
 *   - Activity threshold: 300mg (ADXL362_ACTIVITY_THRESHOLD_MG)
 *   - Activity time: 2 samples (ADXL362_ACTIVITY_TIME_SAMPLES)
 *   - Map ACT interrupt to INT2
 *   - Wake-up mode enabled
 */
HAL_StatusTypeDef adxl362_init_for_shake_awake(void)
{
    uint8_t device_id = 0;
    uint8_t status = 0;
    uint16_t threshold_reg;
    uint8_t thresh_low, thresh_high;
    
    /* ===== Step 1: Verify Device ===== */
    /* Read PARTID to verify ADXL362 is present */
    device_id = adxl362_read_reg(ADXL362_REG_PARTID);
    if (device_id != 0xF2) {
        /* Device not found or wrong ID */
        return HAL_ERROR;
    }
    
    /* ===== Step 2: Soft Reset ===== */
    adxl362_write_reg(ADXL362_REG_SOFT_RESET, 0x52);
    HAL_Delay(10);  /* Wait for reset to complete */
    
    /* ===== Step 3: Configure Activity Threshold ===== */
    /* Convert mg to register value (62.5 mg per LSB) */
    threshold_reg = (ADXL362_ACTIVITY_THRESHOLD_MG * 16) / 1000;
    if (threshold_reg > 2047) threshold_reg = 2047;
    
    thresh_low = (uint8_t)(threshold_reg & 0xFF);
    thresh_high = (uint8_t)((threshold_reg >> 8) & 0x07);
    
    /* Put device in standby before updating threshold */
    adxl362_write_reg(ADXL362_REG_POWER_CTL, ADXL362_POWER_STANDBY);
    HAL_Delay(5);
    
    /* Write threshold registers */
    adxl362_write_reg(ADXL362_REG_THRESH_ACT_L, thresh_low);
    adxl362_write_reg(ADXL362_REG_THRESH_ACT_H, thresh_high);
    
    /* Write activity time */
    adxl362_write_reg(ADXL362_REG_TIME_ACT, ADXL362_ACTIVITY_TIME_SAMPLES);
    
    /* ===== Step 4: Configure Activity Detection ===== */
    /* Enable activity detection on X, Y, Z axes in absolute mode */
    adxl362_write_reg(ADXL362_REG_ACT_INACT_CTL, ADXL362_ACT_ENABLE | ADXL362_ACT_REF);
    
    /* ===== Step 5: Configure Filter (±2g range, 100Hz ODR) ===== */
    /* Filter control: 100Hz BW, ±2g range */
    adxl362_write_reg(ADXL362_REG_FILTER_CTL, 0x00);  /* 100Hz, ±2g */
    
    /* ===== Step 6: Map Activity Interrupt to INT2 (PB6) ===== */
    adxl362_write_reg(ADXL362_REG_INTMAP2, ADXL362_INT_ACT);
    
    /* ===== Step 7: Clear Any Pending Interrupts ===== */
    status = adxl362_read_reg(ADXL362_REG_STATUS);
    (void)status;  /* Clear by reading */
    
    /* ===== Step 8: Enable Wake-up Mode ===== */
    adxl362_write_reg(ADXL362_REG_POWER_CTL, ADXL362_POWER_WAKEUP);
    HAL_Delay(10);
    
    return HAL_OK;
}

/* ========================================================================
 * Shake-Awake Module Functions
 * ======================================================================== */

/**
 * @brief Initialize the shake-awake module
 * Configures ADXL362, GPIO, and EXTI for motion detection
 */
void shake_awake_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* ===== Step 1: Configure PA6 (Verdin Wake Control) ===== */
    /* CRITICAL: PA6 must default LOW immediately at boot */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = VERDIN_WAKE_CTRL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(VERDIN_WAKE_CTRL_GPIO_Port, &GPIO_InitStruct);
    
    /* Immediately drive PA6 LOW - never leave floating or HIGH at boot */
    HAL_GPIO_WritePin(VERDIN_WAKE_CTRL_GPIO_Port, VERDIN_WAKE_CTRL_Pin, GPIO_PIN_RESET);
    
    /* ===== Step 2: Configure PB6 as EXTI input for INT2 ===== */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = ADXL362_INT2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  /* Pull down for stable interrupt */
    HAL_GPIO_Init(ADXL362_INT2_GPIO_Port, &GPIO_InitStruct);
    
    /* Configure EXTI line 6 to use GPIOB */
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI6_Msk) | SYSCFG_EXTICR2_EXTI6_PB;
    
    /* Enable rising edge trigger */
    EXTI->RTSR1 |= (1 << 6);
    
    /* Enable EXTI line 6 in interrupt mask register */
    EXTI->IMR1 |= (1 << 6);
    
    /* Enable EXTI6 interrupt in NVIC (EXTI9_5_IRQn for pins 5-9) */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    /* ===== Step 3: Initialize ADXL362 ===== */
    if (adxl362_init_for_shake_awake() != HAL_OK) {
        /* ADXL362 initialization failed - stay in error loop */
        while (1) {
            HAL_Delay(1000);
        }
    }
    
    /* ===== Step 4: Clear any stale interrupts ===== */
    uint8_t status = adxl362_read_reg(ADXL362_REG_STATUS);
    (void)status;  /* Clear by reading */
    
    /* Clear any pending EXTI on PB6 */
    __HAL_GPIO_EXTI_CLEAR_IT(ADXL362_INT2_Pin);
    shake_irq_pending = 0;
    
    /* Initialize timestamps */
    last_wake_pulse_tick = HAL_GetTick();
    shake_awake_armed = 1;
}

/**
 * @brief Arm the shake-awake system after initialization
 */
void shake_awake_arm(void)
{
    /* Clear any pending interrupts */
    __HAL_GPIO_EXTI_CLEAR_IT(ADXL362_INT2_Pin);
    shake_irq_pending = 0;
    last_wake_pulse_tick = HAL_GetTick();
    shake_awake_armed = 1;
}

/**
 * @brief Handle EXTI interrupt from ADXL362
 * 
 * WARNING: Do not call blocking functions (HAL_Delay, etc.) in this callback!
 * Set flags only and handle processing in shake_awake_task()
 */
void shake_awake_on_exti(uint16_t GPIO_Pin)
{
    /* Check if interrupt is from INT2 (PB6) */
    if (GPIO_Pin == ADXL362_INT2_Pin) {
        /* Set pending flag - do NOT do blocking work here! */
        shake_irq_pending = 1;
    }
}

/**
 * @brief Main loop task for shake-awake processing
 */
void shake_awake_task(void)
{
    uint8_t status_reg;
    uint32_t current_tick;
    
    /* Check if shake interrupt pending */
    if (shake_irq_pending) {
        shake_irq_pending = 0;
        
        /* Read ADXL362 STATUS to confirm and clear interrupt */
        status_reg = adxl362_read_reg(ADXL362_REG_STATUS);
        
        /* Check if this was an activity interrupt */
        if ((status_reg & ADXL362_STATUS_ACT) != 0) {
            /* Check lockout timer */
            current_tick = HAL_GetTick();
            
            /* Handle wrap-around of HAL_GetTick() */
            if ((current_tick - last_wake_pulse_tick) >= SHAKE_AWAKE_LOCKOUT_MS) {
                /* Generate Verdin wake pulse */
                verdin_wake_pulse();
                last_wake_pulse_tick = current_tick;
            }
            /* Else: shake ignored due to lockout */
        }
        
        /* Clear any remaining pending EXTI */
        __HAL_GPIO_EXTI_CLEAR_IT(ADXL362_INT2_Pin);
    }
}

/**
 * @brief Generate Verdin wake pulse through PA6 -> Q1 circuit
 * 
 * WARNING: Verdin control pins are 1.8V domain!
 * The Q1 2N7002 transistor provides level-shifting from 3.3V STM32
 * to 1.8V Verdin input. PA6 HIGH turns Q1 on, pulling the Verdin
 * control line LOW (active-low signal).
 * 
 * Keep pulse below 5 seconds to avoid Verdin force-off behavior
 * on CTRL_PWR_BTN_MICO#.
 */
void verdin_wake_pulse(void)
{
    /* Set PA6 HIGH - turns on Q1, pulls Verdin control line low */
    HAL_GPIO_WritePin(VERDIN_WAKE_CTRL_GPIO_Port, VERDIN_WAKE_CTRL_Pin, GPIO_PIN_SET);
    
    /* Hold pulse for configured duration */
    HAL_Delay(VERDIN_WAKE_PULSE_MS);
    
    /* Set PA6 LOW - releases Q1, releases Verdin control line */
    HAL_GPIO_WritePin(VERDIN_WAKE_CTRL_GPIO_Port, VERDIN_WAKE_CTRL_Pin, GPIO_PIN_RESET);
}

/* ========================================================================
 * EXTI Interrupt Handler for PB6 (INT2)
 * ======================================================================== */

void EXTI9_5_IRQHandler(void)
{
    /* Check if INT2 (PB6) triggered the interrupt */
    if (EXTI->PR1 & (1 << 6)) {
        /* Clear the interrupt pending flag */
        EXTI->PR1 = (1 << 6);
        
        /* Call shake-awake handler */
        shake_awake_on_exti(GPIO_PIN_6);
    }
}