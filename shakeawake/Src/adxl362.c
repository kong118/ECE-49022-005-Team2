/* 
 * ADXL362 Driver Implementation
 */
#include "adxl362.h"
#include <stdint.h>

/* SPI相关定义 */
#define STM32L4xx

/* STM32L432 寄存器定义 */
#define RCC_BASE        0x40021000
#define GPIOA_BASE      0x48000000
#define GPIOB_BASE      0x48000400
#define SPI2_BASE       0x40003800
#define USART2_BASE     0x40004400

/* GPIO 模式 */
#define GPIO_MODE_INPUT     0x0
#define GPIO_MODE_OUTPUT    0x1
#define GPIO_MODE_AF        0x2
#define GPIO_MODE_ANALOG    0x3

/* SPI 相关寄存器 */
typedef struct {
    volatile uint32_t CR1;      /* Control Register 1 */
    volatile uint32_t CR2;      /* Control Register 2 */
    volatile uint32_t SR;       /* Status Register */
    volatile uint32_t DR;       /* Data Register */
    volatile uint32_t CRCPR;    /* CRC Polynomial Register */
    volatile uint32_t RXCRCR;   /* RX CRC Register */
    volatile uint32_t TXCRCR;   /* TX CRC Register */
    volatile uint32_t I2SCFGR;  /* I2S Configuration Register */
    volatile uint32_t I2SPR;    /* I2S Prescaler Register */
} SPI_TypeDef;

/* GPIO 寄存器 */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_TypeDef;

/* USART 寄存器 */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
} USART_TypeDef;

/* RCC 寄存器 */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t ICSCR;
    volatile uint32_t CRRCR;
    volatile uint32_t CFGR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t PLLSAI1CFGR;
    volatile uint32_t PLLSAI2CFGR;
    volatile uint32_t CIER;
    volatile uint32_t CIFR;
    volatile uint32_t CICR;
    volatile uint32_t AHBSR;
    volatile uint32_t RESERVED;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    volatile uint32_t RESERVED2;
    volatile uint32_t APB1RSTR1;
    volatile uint32_t APB1RSTR2;
    volatile uint32_t APB2RSTR;
    volatile uint32_t RESERVED3;
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    volatile uint32_t RESERVED4;
    volatile uint32_t APB1ENR1;
    volatile uint32_t APB1ENR2;
    volatile uint32_t APB2ENR;
} RCC_TypeDef;

#define RCC         ((RCC_TypeDef *) RCC_BASE)
#define GPIOA       ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB       ((GPIO_TypeDef *) GPIOB_BASE)
#define SPI2        ((SPI_TypeDef *) SPI2_BASE)
#define USART2      ((USART_TypeDef *) USART2_BASE)

/* SPI基本操作 */
static void SPI2_Init(void) {
    /* 使能GPIOA, GPIOB, SPI2时钟 */
    RCC->AHB2ENR |= (1 << 0);  /* GPIOA */
    RCC->AHB2ENR |= (1 << 1);  /* GPIOB */
    RCC->APB1ENR1 |= (1 << 14); /* SPI2 */
    
    /* PA5 (SCLK), PA6 (MISO), PA7 (MOSI) 配置为AF01 */
    /* PA5 */
    GPIOA->MODER &= ~(0x3 << 10);
    GPIOA->MODER |= (0x2 << 10);  /* Alternate function */
    GPIOA->AFRL &= ~(0xF << 20);
    GPIOA->AFRL |= (0x5 << 20);   /* AF5 for SPI2 */
    
    /* PA6 */
    GPIOA->MODER &= ~(0x3 << 12);
    GPIOA->MODER |= (0x2 << 12);
    GPIOA->AFRL &= ~(0xF << 24);
    GPIOA->AFRL |= (0x5 << 24);
    
    /* PA7 */
    GPIOA->MODER &= ~(0x3 << 14);
    GPIOA->MODER |= (0x2 << 14);
    GPIOA->AFRL &= ~(0xF << 28);
    GPIOA->AFRL |= (0x5 << 28);
    
    /* PB0 (CS) 配置为输出 */
    GPIOB->MODER &= ~(0x3 << 0);
    GPIOB->MODER |= (0x1 << 0);  /* Output */
    GPIOB->ODR |= (1 << 0);      /* CS高电平 */
    
    /* PA11 (Output) 配置为输出 */
    GPIOA->MODER &= ~(0x3 << 22);
    GPIOA->MODER |= (0x1 << 22); /* Output */
    GPIOA->ODR &= ~(1 << 11);    /* 初始状态低 */
    
    /* PB6 (INT2) 配置为输入 */
    GPIOB->MODER &= ~(0x3 << 12);
    GPIOB->MODER |= (0x0 << 12); /* Input */
    
    /* SPI2配置 */
    SPI2->CR1 = 0x0000;
    SPI2->CR1 |= (0x4 << 3);    /* Prescaler = 32 (Fpclk/32) */
    SPI2->CR1 |= (1 << 2);      /* CPOL = 1 */
    SPI2->CR1 |= (1 << 1);      /* CPHA = 1 */
    SPI2->CR1 |= (1 << 0);      /* SPE - Enable SPI */
    SPI2->CR1 |= (1 << 15);     /* SPI 4-wire mode */
}

/* SPI 传输一个字节 */
static uint8_t SPI2_TransferByte(uint8_t data) {
    while (!(SPI2->SR & (1 << 1))) {} /* 等待 TXE */
    SPI2->DR = data;
    while (!(SPI2->SR & (1 << 0))) {} /* 等待 RXNE */
    return SPI2->DR;
}

/* 读取寄存器 */
uint8_t ADXL362_ReadReg(uint8_t reg) {
    uint8_t result;
    
    GPIOB->ODR &= ~(1 << 0);  /* CS低 */
    SPI2_TransferByte(ADXL362_CMD_READ);
    SPI2_TransferByte(reg);
    result = SPI2_TransferByte(0x00);
    GPIOB->ODR |= (1 << 0);   /* CS高 */
    
    return result;
}

/* 写入寄存器 */
void ADXL362_WriteReg(uint8_t reg, uint8_t value) {
    GPIOB->ODR &= ~(1 << 0);  /* CS低 */
    SPI2_TransferByte(ADXL362_CMD_WRITE);
    SPI2_TransferByte(reg);
    SPI2_TransferByte(value);
    GPIOB->ODR |= (1 << 0);   /* CS高 */
}

/* 读取XYZ数据（12位）*/
void ADXL362_ReadData(ADXL362_Data_t *data) {
    uint8_t xdatal, xdatah, ydatal, ydatah, zdatal, zdatah;
    
    GPIOB->ODR &= ~(1 << 0);  /* CS低 */
    SPI2_TransferByte(ADXL362_CMD_READ);
    SPI2_TransferByte(ADXL362_REG_XDATAL);
    
    xdatal = SPI2_TransferByte(0x00);
    xdatah = SPI2_TransferByte(0x00);
    ydatal = SPI2_TransferByte(0x00);
    ydatah = SPI2_TransferByte(0x00);
    zdatal = SPI2_TransferByte(0x00);
    zdatah = SPI2_TransferByte(0x00);
    
    GPIOB->ODR |= (1 << 0);   /* CS高 */
    
    /* 合并字节为12位数据 */
    data->x = ((int16_t)xdatah << 8) | xdatal;
    data->x >>= 4;  /* 12位数据在高位 */
    
    data->y = ((int16_t)ydatah << 8) | ydatal;
    data->y >>= 4;
    
    data->z = ((int16_t)zdatah << 8) | zdatal;
    data->z >>= 4;
}

/* 获取状态寄存器 */
uint8_t ADXL362_GetStatus(void) {
    return ADXL362_ReadReg(ADXL362_REG_STATUS);
}

/* 初始化ADXL362 */
void ADXL362_Init(uint16_t threshold_mg) {
    SPI2_Init();
    
    /* 验证设备ID */
    uint8_t devid = ADXL362_ReadReg(ADXL362_REG_DEVID);
    (void)devid;  /* 避免未使用警告 */
    
    /* 禁用测量 */
    ADXL362_WriteReg(ADXL362_REG_POWER_CTL, 0x00);
    
    /* 设置活动阈值 (8mg/LSB for ±2g range) */
    uint8_t thresh = (threshold_mg / 8) & 0xFF;
    ADXL362_WriteReg(ADXL362_REG_THRESH_ACT, thresh);
    
    /* 设置活动时间 */
    ADXL362_WriteReg(ADXL362_REG_TIME_ACT, 0x01);
    
    /* 配置活动/非活动检测 (ACT enable, referenced) */
    ADXL362_WriteReg(ADXL362_REG_ACT_INACT_CTL, 0x01);
    
    /* 配置INT2映射到活动中断 */
    ADXL362_WriteReg(ADXL362_REG_INT2_MAP, 0x01);  /* bit0 = ACT */
    
    /* 配置滤波器 */
    ADXL362_WriteReg(ADXL362_REG_FILTER_CTL, 0x03);
    
    /* 启用测量 (2g range, 4-wire SPI) */
    ADXL362_WriteReg(ADXL362_REG_POWER_CTL, 0x02);
}

/* 设置阈值 */
void ADXL362_SetThreshold(uint16_t threshold_mg) {
    uint8_t thresh = (threshold_mg / 8) & 0xFF;
    ADXL362_WriteReg(ADXL362_REG_THRESH_ACT, thresh);
}

/* 将原始数据转换为G (±2g范围) */
float ADXL362_ConvertToG(int16_t raw) {
    /* 12位数据，范围 -2048 到 2047，对应 ±2g */
    return (float)raw / 1024.0f;
}
