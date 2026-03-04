#ifndef SPI_GPIO_CONFIG_H
#define SPI_GPIO_CONFIG_H

#include "stm32l432xx.h"

/* ============== Pin Definitions ============== */

/* SPI Pins (SPI1) */
#define SPI_PORT                GPIOB
#define SPI_CLK_PIN             3       /* PB3 - SCLK (D13) */
#define SPI_MOSI_PIN            5       /* PB5 - MOSI (D11) */
#define SPI_MISO_PIN            4       /* PB4 - MISO (D12) */
#define SPI_CS_PIN              0       /* PB0 - CS Active Low (D3) */

/* Output Pin */
#define OUTPUT_PORT             GPIOA
#define OUTPUT_PIN              7       /* PA7 - A6, Active High */

/* Interrupt Pin */
#define INT2_PORT               GPIOB
#define INT2_PIN                6       /* PB6 - D5 */
#define INT2_EXTI_LINE          6
#define INT2_IRQ                EXTI15_10_IRQn

/* SPI Instance */
#define SPI_INSTANCE            SPI1

/* ============== Function Prototypes ============== */

void SPI_Init(void);
void SPI_CS_Low(void);
void SPI_CS_High(void);
void SPI_Write_Read(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t size);

void GPIO_Init(void);
void Output_Pin_Init(void);
void Output_Set_High(void);
void Output_Set_Low(void);

void INT2_Interrupt_Init(void);

#endif // SPI_GPIO_CONFIG_H
