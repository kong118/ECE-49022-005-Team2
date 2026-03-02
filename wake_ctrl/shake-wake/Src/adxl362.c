#include "../Inc/adxl362.h"
#include "../Inc/stm32l432xx.h"
#include <string.h>

/* Global variables */
static volatile uint8_t spi_tx_buffer[64];
static volatile uint8_t spi_rx_buffer[64];
static volatile uint32_t activity_threshold = 100;

/* SPI Configuration */
#define CS_PORT GPIOB
#define CS_PIN 0

#define SCK_PORT GPIOA
#define SCK_PIN 5

#define MOSI_PORT GPIOA
#define MOSI_PIN 7

#define MISO_PORT GPIOA
#define MISO_PIN 6

#define INT2_PORT GPIOB
#define INT2_PIN 6

/* Forward declarations */
static void SPI_SendByte(uint8_t byte);
static uint8_t SPI_ReceiveByte(void);
static void SPI_Init(void);
static void GPIO_Init(void);

/**
 * @brief Initialize ADXL362
 */
void ADXL362_Init(void)
{
    uint8_t data;

    GPIO_Init();
    SPI_Init();

    /* Delay for power up */
    for(uint32_t i = 0; i < 1000000; i++);

    /* Soft reset */
    data = 0x52;
    ADXL362_WriteReg(ADXL362_REG_SOFT_RESET, &data, 1);

    /* Delay after reset */
    for(uint32_t i = 0; i < 1000000; i++);

    /* Set to measurement mode */
    data = ADXL362_POWER_MEASURE;
    ADXL362_WriteReg(ADXL362_REG_POWER_CTL, &data, 1);

    /* Configure activity threshold (mg) */
    ADXL362_SetActivityThreshold(activity_threshold);

    /* Configure activity time */
    data = 10;
    ADXL362_WriteReg(ADXL362_REG_TIME_ACT, &data, 1);

    /* Enable activity interrupt on INT2 */
    ADXL362_ConfigureINT2();

    /* Enable activity detection */
    data = ADXL362_INT_ACTIVITY;
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
}

/**
 * @brief Initialize GPIO
 */
static void GPIO_Init(void)
{
    /* Enable GPIOA and GPIOB clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

    /* Configure CS (PB0) as GPIO output */
    GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk);
    GPIOB->MODER |= (1UL << GPIO_MODER_MODE0_Pos);
    GPIOB->BSRR = (1UL << CS_PIN);  /* Set CS high initially */

    /* Configure SCK (PA5) as alternate function */
    GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk);
    GPIOA->MODER |= (2UL << GPIO_MODER_MODE5_Pos);
    GPIOA->AFR[0] |= (0UL << (SCK_PIN * 4));  /* AF0 for SPI1 */

    /* Configure MOSI (PA7) as alternate function */
    GPIOA->MODER &= ~(GPIO_MODER_MODE7_Msk);
    GPIOA->MODER |= (2UL << GPIO_MODER_MODE7_Pos);
    GPIOA->AFR[0] |= (0UL << (MOSI_PIN * 4));  /* AF0 for SPI1 */

    /* Configure MISO (PA6) as alternate function */
    GPIOA->MODER &= ~(GPIO_MODER_MODE6_Msk);
    GPIOA->MODER |= (2UL << GPIO_MODER_MODE6_Pos);
    GPIOA->AFR[0] |= (0UL << (MISO_PIN * 4));  /* AF0 for SPI1 */

    /* Configure INT2 (PB6) as input */
    GPIOB->MODER &= ~(GPIO_MODER_MODE6_Msk);
    GPIOB->PUPDR |= (2UL << GPIO_PUPDR_PUPD6_Pos);  /* Pull-down */
}

/**
 * @brief Initialize SPI1
 */
static void SPI_Init(void)
{
    /* Enable SPI1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* Configure SPI1:
       - Master mode
       - Clock polarity = 0, Clock phase = 0 (CPOL=0, CPHA=0)
       - Baud rate = Fclk/4 (4 MHz for 16 MHz clock)
       - MSB first
    */
    SPI1->CR1 = 0;
    SPI1->CR1 |= (SPI_CR1_MSTR |          /* Master mode */
                  (1UL << SPI_CR1_BR_Pos) | /* BR = 1 (divide by 4) */
                  SPI_CR1_SPE);            /* Enable SPI */

    /* Set frame size to 8-bit */
    SPI1->CR2 |= (7UL << SPI_CR2_DS_Pos);
}

/**
 * @brief Send byte via SPI
 */
static void SPI_SendByte(uint8_t byte)
{
    SPI1->DR = byte;
    while(!(SPI1->SR & SPI_SR_TXE));
}

/**
 * @brief Receive byte via SPI
 */
static uint8_t SPI_ReceiveByte(void)
{
    SPI1->DR = 0xFF;
    while(!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

/**
 * @brief Read register from ADXL362
 */
void ADXL362_ReadReg(uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;

    /* Pull CS low */
    GPIOB->BRR = (1UL << CS_PIN);

    /* Send read command and address */
    SPI_SendByte(ADXL362_CMD_READ);
    SPI_SendByte(reg);

    /* Read data */
    for(i = 0; i < len; i++)
    {
        data[i] = SPI_ReceiveByte();
    }

    /* Pull CS high */
    GPIOB->BSRR = (1UL << CS_PIN);
}

/**
 * @brief Write register to ADXL362
 */
void ADXL362_WriteReg(uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;

    /* Pull CS low */
    GPIOB->BRR = (1UL << CS_PIN);

    /* Send write command and address */
    SPI_SendByte(ADXL362_CMD_WRITE);
    SPI_SendByte(reg);

    /* Write data */
    for(i = 0; i < len; i++)
    {
        SPI_SendByte(data[i]);
    }

    /* Pull CS high */
    GPIOB->BSRR = (1UL << CS_PIN);
}

/**
 * @brief Read acceleration data
 */
void ADXL362_ReadAccel(ADXL362_Data_t *accel_data)
{
    uint8_t data[6];

    ADXL362_ReadReg(ADXL362_REG_XDATA_L, data, 6);

    accel_data->x = (int16_t)((data[1] << 8) | data[0]);
    accel_data->y = (int16_t)((data[3] << 8) | data[2]);
    accel_data->z = (int16_t)((data[5] << 8) | data[4]);
}

/**
 * @brief Read status register
 */
uint8_t ADXL362_ReadStatus(void)
{
    uint8_t status;
    ADXL362_ReadReg(ADXL362_REG_STATUS, &status, 1);
    return status;
}

/**
 * @brief Set activity threshold
 */
void ADXL362_SetActivityThreshold(uint16_t threshold)
{
    uint8_t data[2];

    activity_threshold = threshold;

    /* Convert mg to register value (11-bit, LSB = 1 mg) */
    data[0] = (uint8_t)(threshold & 0xFF);
    data[1] = (uint8_t)((threshold >> 8) & 0x07);

    ADXL362_WriteReg(ADXL362_REG_THRESH_ACT_L, data, 2);
}

/**
 * @brief Set inactivity threshold
 */
void ADXL362_SetInactivityThreshold(uint16_t threshold)
{
    uint8_t data[2];

    /* Convert mg to register value (11-bit, LSB = 1 mg) */
    data[0] = (uint8_t)(threshold & 0xFF);
    data[1] = (uint8_t)((threshold >> 8) & 0x07);

    ADXL362_WriteReg(ADXL362_REG_THRESH_INACT_L, data, 2);
}

/**
 * @brief Set activity time
 */
void ADXL362_SetActivityTime(uint8_t time)
{
    ADXL362_WriteReg(ADXL362_REG_TIME_ACT, &time, 1);
}

/**
 * @brief Set inactivity time
 */
void ADXL362_SetInactivityTime(uint16_t time)
{
    uint8_t data[2];

    data[0] = (uint8_t)(time & 0xFF);
    data[1] = (uint8_t)((time >> 8) & 0xFF);

    ADXL362_WriteReg(ADXL362_REG_TIME_INACT_L, data, 2);
}

/**
 * @brief Enable activity interrupt
 */
void ADXL362_EnableActivityInt(void)
{
    uint8_t data;
    ADXL362_ReadReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
    data |= ADXL362_INT_ACTIVITY;
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
}

/**
 * @brief Disable activity interrupt
 */
void ADXL362_DisableActivityInt(void)
{
    uint8_t data;
    ADXL362_ReadReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
    data &= ~ADXL362_INT_ACTIVITY;
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
}

/**
 * @brief Enable inactivity interrupt
 */
void ADXL362_EnableInactivityInt(void)
{
    uint8_t data;
    ADXL362_ReadReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
    data |= ADXL362_INT_INACTIVITY;
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
}

/**
 * @brief Disable inactivity interrupt
 */
void ADXL362_DisableInactivityInt(void)
{
    uint8_t data;
    ADXL362_ReadReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
    data &= ~ADXL362_INT_INACTIVITY;
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, &data, 1);
}

/**
 * @brief Configure INT2 for activity interrupt
 */
void ADXL362_ConfigureINT2(void)
{
    uint8_t data;

    /* Read current INT2 mapping */
    ADXL362_ReadReg(ADXL362_REG_INTMAP2, &data, 1);

    /* Set INT2 for activity */
    data |= ADXL362_INT2_ACT;

    /* Write back */
    ADXL362_WriteReg(ADXL362_REG_INTMAP2, &data, 1);
}

/**
 * @brief Get current activity threshold
 */
uint32_t ADXL362_GetActivityThreshold(void)
{
    return activity_threshold;
}
