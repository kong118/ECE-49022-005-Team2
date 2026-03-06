#include "spi_gpio_config.h"

/* ========================================================================
 * Forward Declarations
 * ======================================================================== */

void Error_Handler(void);

/* ========================================================================
 * SPI1 Handle
 * ======================================================================== */

SPI_HandleTypeDef hspi1;

/* ========================================================================
 * MX_GPIO_Init - Initialize all GPIO pins
 * ======================================================================== */

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO port clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* ===== Configure ADXL362 CS (PB0) as GPIO output ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  /* Low speed = lower power */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    ADXL362_CS_High();  /* CS active LOW, so initialize HIGH */
    
    /* ===== Configure SPI1 pins (PB3=SCLK, PB4=MISO, PB5=MOSI) ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* ===== Configure ADXL362 INT1 (PB1 = D6) as EXTI input (rising edge) ===== */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    SYSCFG->EXTICR[0] = (SYSCFG->EXTICR[0] & ~SYSCFG_EXTICR1_EXTI1_Msk) | SYSCFG_EXTICR1_EXTI1_PB;
    
    HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    
    /* ===== Configure OUTPUT (PA7) as GPIO output ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    Output_Set_Low();
    
    /* ===== Configure EN (PA6) as GPIO output ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    Enable_Set_Low();
    
    /* ===== Set ALL unused pins to analog mode for minimum leakage ===== */
    GPIO_SetUnusedAnalog();
}

/* ========================================================================
 * GPIO_SetUnusedAnalog - Set unused pins to analog to minimize leakage
 * Used pins: PA6(EN), PA7(OUT), PA13(SWDIO), PA14(SWCLK)
 *            PB0(CS), PB1(INT1), PB3(SCLK), PB4(MISO), PB5(MOSI)
 * ======================================================================== */
void GPIO_SetUnusedAnalog(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* GPIOA unused: PA0,PA1,PA2,PA3,PA4,PA5,PA8,PA9,PA10,PA11,PA12,PA15 */
    /* Keep PA13(SWDIO), PA14(SWCLK) for debug - can disable if not needed */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 |
                          GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* GPIOB unused: PB2,PB6,PB7 */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* ========================================================================
 * MX_SPI1_Init - Initialize SPI1 peripheral
 * ======================================================================== */

void MX_SPI1_Init(void)
{
    /* Enable SPI1 clock */
    __HAL_RCC_SPI1_CLK_ENABLE();
    
    /* Configure SPI1 */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;         /* CPOL = 0 */
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;             /* CPHA = 0 */
    hspi1.Init.NSS = SPI_NSS_SOFT;                     /* Software NSS (manual CS) */
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hspi1.Init.CRCPolynomial = 10;
    
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

/* ========================================================================
 * HAL MSP Callback - Required for HAL_SPI_Init()
 * ======================================================================== */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    /* Already done in MX_GPIO_Init(), so this is minimal */
}

/* ========================================================================
 * UART stub - does nothing (UART disabled for low power)
 * ======================================================================== */
void UART_SendString(const char *str)
{
    (void)str;  /* unused */
}
