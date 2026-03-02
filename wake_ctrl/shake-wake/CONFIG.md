# ADXL362 Shake Awake Detection - NUCLEO-L432KC

## 概述
这是一个基于NUCLEO-L432KC MCU的ADXL362 IMU shake awake（摇晃唤醒）系统。

## 硬件连接

| 功能 | 管脚 | MCU管脚 |
|------|------|--------|
| SCK (时钟) | A4 | PA_5 |
| MISO (数据输入) | A5 | PA_6 |
| MOSI (数据输出) | A6 | PA_7 |
| CS (片选) | D3 | PB_0 |
| INT2 (中断) | D5 | PB_6 |
| GND | GND | GND |
| VDD | VDD | 3.3V |

## 文件结构

```
├── Inc/
│   ├── adxl362.h          # ADXL362驱动头文件
│   ├── shake_detect.h     # Shake检测模块头文件
│   └── mcu_init.h         # MCU初始化头文件
├── Src/
│   ├── main.c             # 主程序
│   ├── adxl362.c          # ADXL362驱动实现
│   ├── shake_detect.c     # Shake检测实现
│   └── mcu_init.c         # MCU初始化实现
└── CONFIG.md              # 本文件
```

## 主要功能

### 1. ADXL362驱动 (adxl362.c/h)
- SPI通信接口
- 寄存器读写函数
- 加速度传感器初始化
- 活动检测和中断配置

### 2. Shake检测模块 (shake_detect.c/h)
- 加速度计算和阈值比较
- 活动检测处理
- 可调整的灵敏度控制
- 系统时间管理

### 3. MCU初始化 (mcu_init.c/h)
- 系统时钟配置 (80 MHz PLL)
- GPIO初始化
- LED指示灯控制

## 使用方法

### 基础配置

在 `main.c` 中修改活动阈值：

```c
#define ACTIVITY_THRESHOLD 150  /* mg值，范围50-500 */
```

- **较低值** (50-100 mg): 灵敏度高，易检测到微小震动
- **中等值** (100-200 mg): 平衡灵敏度
- **较高值** (200-500 mg): 灵敏度低，只检测明显震动

### 动态调整阈值

在程序运行中动态调整阈值：

```c
Set_ActivityThreshold(150);  /* 设置新的阈值 */
uint16_t current = Get_ActivityThreshold();  /* 获取当前阈值 */
```

### 检测唤醒事件

主循环自动监控shake事件：

```c
if(ShakeDetect_IsAwake())
{
    /* 检测到shake唤醒 */
    LED_Toggle();  /* LED闪烁表示检测到事件 */
}
```

## 工作原理

1. **初始化阶段**：
   - 初始化MCU系统时钟到80 MHz
   - 配置SPI1用于与ADXL362通信
   - 配置GPIO管脚
   - 初始化ADXL362传感器
   - 设置活动检测阈值

2. **监测阶段**：
   - 连续读取加速度数据
   - 计算加速度幅值
   - 与阈值比较
   - 检测shake事件

3. **唤醒阶段**：
   - INT2中断引脚产生信号
   - 点亮LED指示灯
   - 可以扩展为唤醒主系统

## 技术参数

| 参数 | 值 |
|------|-----|
| MCU | STM32L432KC |
| 传感器 | ADXL362 |
| SPI速率 | 4 MHz |
| 系统时钟 | 80 MHz |
| 采样率 | 实时 |
| 阈值范围 | 50-500 mg |

## 调试和故障排除

### 症状1：未检测到shake
- 检查硬件连接，特别是SPI管脚和CS管脚
- 增大ACTIVITY_THRESHOLD值（降低灵敏度）
- 确认ADXL362芯片工作正常

### 症状2：误触发频繁
- 减小ACTIVITY_THRESHOLD值（增加灵敏度）
- 增加shake_debounce阈值以提高防抖

### 症状3：SPI通信失败
- 验证SPI时钟速率是否正确
- 检查CS信号是否正确拉高/拉低
- 确认MOSI/MISO/SCK连接正确

## LED指示

- LED每次检测到shake事件时会闪烁一次
- 可在main.c中修改LED行为

## 扩展功能

可以在main.c的`wake_event_count`后添加：
- UART调试输出
- 外部信号控制
- 多传感器融合
- 低功耗模式
- 自定义唤醒动作

## 编译和烧写

使用STM32CubeIDE或ARM mbed工具链编译本项目。

## 许可证

本项目基于STM32微控制器开发。
