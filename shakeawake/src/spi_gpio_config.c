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
 * UART1 Handle for terminal output
 * ======================================================================== */

UART_HandleTypeDef huart1;

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
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    ADXL362_CS_High();  /* CS active LOW, so initialize HIGH */
    
    /* ===== Configure SPI1 pins (PB3=SCLK, PB4=MISO, PB5=MOSI) ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* ===== Configure ADXL362 INT2 (PB6) as EXTI input (rising edge) ===== */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  /* Rising edge interrupt */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Configure EXTI line 6 to use GPIOB (using register directly) */
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI6_Msk) | SYSCFG_EXTICR2_EXTI6_PB;
    
    /* Enable EXTI 5-9 interrupt (pin 6 is in this range) */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    /* ===== Configure OUTPUT (PA7) as GPIO output ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    Output_Set_Low();  /* Initialize LOW */
    
    /* ===== Configure EN (PA3) as GPIO output ===== */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    Enable_Set_Low();  /* Initialize LOW */
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
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  /* PCLK2/8 */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;            /* MSB first */
    hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hspi1.Init.CRCPolynomial = 10;
    
    /* Initialize the SPI */
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
 * MX_USART1_Init - Initialize USART1 for terminal output
 * PA9 = TX, PA10 = RX
 * Baud rate: 115200
 * ======================================================================== */

void MX_USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable USART1 and GPIO clocks */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA9 (TX) and PA10 (RX) as USART1 alternate function */
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure USART1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

/* ========================================================================
 * UART_SendString - Send a string via UART1
 * ======================================================================== */

void UART_SendString(const char *str)
{
    if (str == NULL) {
        return;
    }
    
    uint16_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t *)str, len, HAL_MAX_DELAY);
    }
}

/* ========================================================================
 * UART_Printf - Send formatted string via UART1 (like printf)
 * Simplified version using sprintf to avoid vsnprintf issues
 * ======================================================================== */

#include <stdio.h>
#include <stdarg.h>

void UART_Printf(const char *format, ...)
{
    char buffer[512];
    va_list args;
    
    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);
    
    if (len > 0 && len < (int)sizeof(buffer)) {
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, len, HAL_MAX_DELAY);
    }
}
