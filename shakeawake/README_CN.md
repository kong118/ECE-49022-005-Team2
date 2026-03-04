# ADXL362 Shake Awake 低功耗实现 - NUCLEO-L432KC

本项目在NUCLEO-L432KC微控制器上实现ADXL362加速度传感器的**低功耗Shake Awake**功能。

## 核心特性

### 硬件功能
✅ **SPI接口配置**
- SCLK: D13 (PB3)
- MISO: D12 (PB4)
- MOSI: D11 (PB5)
- CS: D3 (PB0) - 活跃低
- INT2: D5 (PB6) - 中断输入

✅ **输出控制**
- PA7 (A6) - 活跃高，可配置脉冲宽度

### 低功耗特性
✅ **ADXL362配置**
- **Wake-up Mode**: 超低功耗唤醒模式
- **100Hz数据率**: 电源优化
- **Activity Detection**: 活动自动检测中断

✅ **MCU电源管理**
- **STOP2模式**: 深度睡眠（功耗极低）
- **INT2唤醒**: 加速度检测时唤醒MCU
- **自动睡眠**: 可配置的不活动超时

## 快速开始

### 编译
```bash
cd /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake
pio run
```

### 上传
```bash
pio run -t upload
```

### 调试
```bash
pio device monitor --baud 115200
```

## 功能配置

### 默认配置值

在 `include/config.h` 中定义：

```c
#define DEFAULT_ACTIVITY_THRESHOLD_MG        1500    /* 1500 mg */
#define DEFAULT_OUTPUT_PULSE_DURATION_MS     500     /* 500 ms */
#define DEFAULT_INACTIVITY_TIMEOUT_SEC       30      /* 30秒后进入STOP2 */
#define DEFAULT_ENABLE_STOP2_MODE            1       /* 启用STOP2 */
#define DEFAULT_DATA_RATE_HZ                 100     /* 100 Hz */
```

### 运行时修改配置

在 `src/main.c` 初始化后添加：

```c
/* 修改活动阈值 */
Set_Activity_Threshold(1200);      /* 1200 mg */

/* 修改输出脉冲宽度 */
Set_Output_Duration(800);          /* 800 ms */

/* 修改不活动超时 */
Set_Inactivity_Timeout(60);        /* 60秒后睡眠 */

/* 禁用STOP2模式（保持MCU运行） */
Disable_STOP2_Mode();

/* 启用STOP2模式 */
Enable_STOP2_Mode();
```

## 工作流程

### 正常操作 (MCU唤醒)
1. MCU运行在正常模式 (4MHz)
2. ADXL362持续监测加速度
3. 超过阈值时触发INT2中断
4. INT2唤醒MCU（如果在STOP2）
5. MCU读取加速度数据并触发PA7输出脉冲
6. 超时后PA7返回低电平

### 低功耗操作 (STOP2睡眠)
1. 30秒无活动后，MCU进入STOP2模式
2. MCU消耗能量极低（μA级别）
3. ADXL362继续在Wake-up模式工作（mA级别）
4. 检测到活动时，INT2唤醒MCU
5. MCU恢复运行并处理加速度数据

## 功耗对比

| 模式 | 消耗 | 说明 |
|------|------|------|
| **Run Mode (4MHz)** | ~2 mA | 正常运行 |
| **STOP2 (RTC on)** | ~0.4 μA | 深度睡眠 |
| **ADXL362 Wakeup** | ~11 μA | 100Hz采样 |
| **总功耗 (睡眠中)** | ~11.4 μA | 主要来自传感器 |

## 硬件连接

```
NUCLEO-L432KC          ADXL362
───────────────        ─────────
D13 (PB3)     ◄────►  SCLK
D12 (PB4)     ◄────►  MISO
D11 (PB5)     ◄────►  MOSI
D3 (PB0)      ◄────►  CS (Active Low)
D5 (PB6)      ◄────►  INT2
3.3V          ◄────►  VCC
GND           ◄────►  GND

PA7 (A6)      ─────►  USB_5V → Relay/LED (Active High)
```

## API 参考

### 配置函数
```c
void Set_Activity_Threshold(uint16_t threshold_mg);     /* 设置活动阈值 */
void Set_Output_Duration(uint16_t duration_ms);         /* 设置输出脉冲宽度 */
void Set_Inactivity_Timeout(uint16_t timeout_sec);      /* 设置睡眠超时 */
void Enable_STOP2_Mode(void);                           /* 启用STOP2 */
void Disable_STOP2_Mode(void);                          /* 禁用STOP2 */
```

### 传感器函数
```c
void ADXL362_Init_Wakeup_Mode(void);                    /* 初始化Wake-up模式 */
void ADXL362_Get_Acceleration(accel_data_t *accel);     /* 读取XYZ数据 */
uint32_t ADXL362_Get_Magnitude(accel_data_t *accel);    /* 计算加速度幅值 */
void ADXL362_Set_Activity_Threshold(uint16_t threshold); /* 设置ADXL阈值 */
```

### 电源管理
```c
void Enter_STOP2_Mode(void);                            /* 进入STOP2 */
void Exit_STOP2_Mode(void);                             /* 退出STOP2 */
uint8_t Is_System_Awake(void);                          /* 检查MCU是否唤醒 */
```

## 中断处理

INT2中断（来自ADXL362）会：
1. 设置 `activity_detected_flag` 标志
2. 如果在STOP2中则唤醒MCU
3. 更新最后活动时间戳

使用全局标志而不是立即处理可避免在睡眠期间读取SPI。

## 调试建议

### 验证Wake-up模式
```c
uint8_t chip_id;
ADXL362_Read_Register(ADXL362_DEVID_AD, &chip_id);
if (chip_id == 0xAD) {
    /* 连接正常 */
}
```

### 监测STOP2进入
```c
/* 手动测试STOP2（禁用中断唤醒） */
Disable_Sleep_Mode();
Enter_STOP2_Mode();     /* 等待RESET按钮 */
```

### 检查INT2中断
```c
/* INT2应该在超过阈值时拉高 */
/* 使用示波器或逻辑分析仪监测PB6 */
```

## 优化建议

### 进一步降低功耗
1. 禁用UART/调试端口
2. 禁用未使用的外设时钟
3. 使用更低的SPI速率
4. 增加STOP2进入前的延迟

### 提高检测精度
1. 启用数据缓冲（多样本平均）
2. 增加ADXL362采样率
3. 调整阈值以适应环境

## 常见问题

**Q: STOP2模式消耗功率还是很高？**
A: 检查是否启用了不必要的外设时钟。使用 `HAL_PWREx_EnableLowPowerRegulator()` 优化稳压器。

**Q: INT2中断不触发？**
A: 验证：
- ADXL362是否正确初始化
- INT2_MAP寄存器设置是否正确
- EXTI中断是否启用
- 阈值是否合理

**Q: 输出脉冲宽度不准确？**
A: 检查HAL_GetTick()和HAL_Delay()的时基配置。

## 文件结构

```
shakeawake/
├── include/
│   ├── adxl362_lowpower.h      ← 传感器驱动
│   ├── spi_gpio_config.h       ← SPI/GPIO配置
│   ├── power_management.h      ← 电源管理
│   └── config.h                ← 系统配置
├── src/
│   ├── main.c                  ← 主程序
│   ├── adxl362_lowpower.c      ← 传感器实现
│   ├── spi_gpio_config.c       ← SPI/GPIO实现
│   ├── power_management.c      ← 电源管理实现
│   └── config.c                ← 配置实现
└── platformio.ini              ← 项目配置
```

## 性能指标

| 指标 | 值 |
|------|-----|
| **数据率** | 100 Hz |
| **加速度范围** | ±2g |
| **分辨率** | 1 mg/LSB |
| **INT2延迟** | < 10 ms |
| **STOP2唤醒延迟** | < 20 ms |
| **输出控制精度** | ±10 ms |

## 参考资源

- ADXL362数据手册
- STM32L432KC参考手册
- NUCLEO-L432KC用户指南
- STM32CubeL4 HAL库文档

---

**项目创建人**: ECE 49022 Team 2
**最后更新**: 2026年3月4日
