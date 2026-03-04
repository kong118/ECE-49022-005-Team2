#include "spi_gpio_config.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"

/*===================== SPI Initialization =====================*/
void SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIOB and SPI1 clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    /* Configure SPI pins (CLK, MOSI, MISO) */
    GPIO_InitStruct.Pin = (1 << SPI_CLK_PIN) | (1 << SPI_MOSI_PIN) | (1 << SPI_MISO_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    
    HAL_GPIO_Init(SPI_PORT, &GPIO_InitStruct);
    
    /* Configure CS pin as GPIO output */
    GPIO_InitStruct.Pin = (1 << SPI_CS_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(SPI_PORT, &GPIO_InitStruct);
    SPI_CS_High();  /* CS is active low, keep high initially */
    
    /* Configure SPI1 via registers */
    /* SPI Control Register 1 */
    SPI1->CR1 = 0;  /* Clear CR1 */
    SPI1->CR1 |= (0 << SPI_CR1_BIDIMODE_Pos);  /* 2-line bidirectional */
    SPI1->CR1 |= (0 << SPI_CR1_BIDIOE_Pos);    /* Receive mode */
    SPI1->CR1 |= (0 << SPI_CR1_CRCEN_Pos);     /* CRC disabled */
    SPI1->CR1 |= (0 << SPI_CR1_CRCNEXT_Pos);   /* CRC next disabled */
    /* SPI1->CR1 |= (0 << SPI_CR1_DFF_Pos); */   /* 8-bit data (default) */
    SPI1->CR1 |= (0 << SPI_CR1_RXONLY_Pos);    /* Full duplex */
    SPI1->CR1 |= (1 << SPI_CR1_SSM_Pos);       /* Software slave management */
    SPI1->CR1 |= (1 << SPI_CR1_SSI_Pos);       /* Internal slave select */
    /* SPI1->CR1 |= (0 << SPI_CR1_LSBFIRST_Pos); */ /* MSB first (default) */
    SPI1->CR1 |= (3 << SPI_CR1_BR_Pos);        /* Baud rate: PCLK/8 */
    SPI1->CR1 |= (0 << SPI_CR1_CPOL_Pos);      /* Clock polarity: low */
    SPI1->CR1 |= (0 << SPI_CR1_CPHA_Pos);      /* Clock phase: leading */
    SPI1->CR1 |= (1 << SPI_CR1_MSTR_Pos);      /* Master mode */
    
    /* SPI Control Register 2 */
    SPI1->CR2 = 0;  /* Clear CR2 */
    SPI1->CR2 |= (0 << SPI_CR2_TXEIE_Pos);     /* TX interrupt disabled */
    SPI1->CR2 |= (0 << SPI_CR2_RXNEIE_Pos);    /* RX interrupt disabled */
    
    /* Enable SPI */
    SPI1->CR1 |= SPI_CR1_SPE;
}

/*===================== CS Pin Control =====================*/
void SPI_CS_Low(void)
{
    HAL_GPIO_WritePin(SPI_PORT, (1 << SPI_CS_PIN), GPIO_PIN_RESET);
}

void SPI_CS_High(void)
{
    HAL_GPIO_WritePin(SPI_PORT, (1 << SPI_CS_PIN), GPIO_PIN_SET);
}

/*===================== SPI Read/Write =====================*/
void SPI_Write_Read(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t size)
{
    uint8_t i;
    
    SPI_CS_Low();
    
    for (i = 0; i < size; i++) {
        /* Wait for transmit buffer empty */
        while (!(SPI1->SR & SPI_SR_TXE));
        
        /* Send byte */
        SPI1->DR = tx_buf[i];
        
        /* Wait for receive buffer not empty */
        while (!(SPI1->SR & SPI_SR_RXNE));
        
        /* Read received byte */
        rx_buf[i] = SPI1->DR;
    }
    
    /* Wait for SPI to finish */
    while (SPI1->SR & SPI_SR_BSY);
    
    SPI_CS_High();
}

/*===================== GPIO Initialization =====================*/
void GPIO_Init(void)
{
    /* Enable GPIOB and GPIOA clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    /* Initialize INT2 pin as input */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = (1 << INT2_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(INT2_PORT, &GPIO_InitStruct);
}

/*===================== Output Pin Control =====================*/
void Output_Pin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    /* Configure PA7 as output */
    GPIO_InitStruct.Pin = (1 << OUTPUT_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(OUTPUT_PORT, &GPIO_InitStruct);
    Output_Set_Low();  /* Initialize as low */
}

void Output_Set_High(void)
{
    HAL_GPIO_WritePin(OUTPUT_PORT, (1 << OUTPUT_PIN), GPIO_PIN_SET);
}

void Output_Set_Low(void)
{
    HAL_GPIO_WritePin(OUTPUT_PORT, (1 << OUTPUT_PIN), GPIO_PIN_RESET);
}

/*===================== INT2 Interrupt Configuration =====================*/
void INT2_Interrupt_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    
    /* Enable SYSCFG clock for EXTI */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    
    /* Configure PB6 as external interrupt input */
    GPIO_InitStruct.Pin = (1 << INT2_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  /* Rising edge trigger */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(INT2_PORT, &GPIO_InitStruct);
    
    /* Configure EXTI line 6 */
    /* Set EXTI source to GPIOB for line 6 */
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI6_Msk) | SYSCFG_EXTICR2_EXTI6_PB;
    
    /* Enable rising edge trigger for EXTI6 */
    EXTI->RTSR1 |= (1 << INT2_PIN);
    
    /* Enable EXTI6 interrupt */
    EXTI->IMR1 |= (1 << INT2_PIN);
    
    /* Enable EXTI interrupt in NVIC */
    NVIC_SetPriority(INT2_IRQ, 0);
    NVIC_EnableIRQ(INT2_IRQ);
}
