#ifndef CONFIG_H
#define CONFIG_H

/* ADXL362配置参数 */

/* 活动阈值（毫G），8mg/LSB */
/* 可调范围：40-2440mg（5-305 LSBs） */
#define ADXL362_THRESHOLD_MG    300     /* 300mg */

/* UART波特率 */
#define UART_BAUDRATE           9600

/* 延迟系数（用于Delay函数）*/
#define DELAY_MULTIPLIER        4000

#endif /* CONFIG_H */
