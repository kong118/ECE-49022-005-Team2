# ADXL362 Shake Awake 实现

基于NUCLEO-L432KC的ADXL362加速度计shake awake功能实现

## 硬件连接

### SPI连接（SPI2）
- **SCLK**: PA5 (D4)
- **MISO**: PA6 (D5)
- **MOSI**: PA7 (D6)
- **CS**: PB0 (D3) - Active Low（低电平有效）

### 中断和输出
- **INT2**: PB6 (D5) - ADXL362活动中断输出
- **PA11 输出**: D10 - 主动高电平输出（shake事件时拉高）

### UART连接（用于数据输出）
- **TX**: PA2
- **RX**: PA3
- **波特率**: 9600 bps

## 项目结构

```
Inc/
├── adxl362.h       # ADXL362驱动头文件
└── config.h        # 配置参数文件

Src/
├── main.c          # 主程序
├── adxl362.c       # ADXL362驱动实现
└── ...其他系统文件...
```

## 功能说明

### ADXL362 Shake Awake功能

1. **活力检测（Activity Detection）**
   - 监测加速度超过设置的阈值
   - 默认阈值：300mg
   - 可在 `config.h` 中调整

2. **中断输出**
   - INT2引脚在检测到shake事件时输出高电平
   - PA11输出跟随INT2信号（活动高）

3. **实时数据输出**
   - 通过UART2（9600波特率）输出当前XYZ轴加速度值
   - 单位：g (重力加速度)
   - 显示shake事件

## 使用方法

### 编译

使用CMake构建项目：
```bash
cmake -B build
cd build
make
```

或使用STM32CubeIDE在VS Code中编译。

### 运行

1. 通过ST-Link连接NUCLEO-L432KC开发板
2. 编程并运行代码
3. 打开串口工具（如Tera Term, PuTTY）
4. 波特率：9600，数据位：8，停止位：1，校验位：无
5. 观察XYZ轴加速度数据输出

### 输出示例

```
========================================
ADXL362 Shake Awake Demo
========================================
Initializing ADXL362...
ADXL362 Initialized
Threshold: 300mg
Waiting for shake events...
========================================

X:  0.10 G | Y: -0.08 G | Z:  1.00 G
X:  0.12 G | Y: -0.05 G | Z:  0.98 G
X:  0.89 G | Y:  0.65 G | Z:  1.15 G [SHAKE DETECTED]
```

## 参数调整

### 修改活动阈值

编辑 `Inc/config.h` 中的 `ADXL362_THRESHOLD_MG`：

```c
#define ADXL362_THRESHOLD_MG    300     /* 可调范围：40-2440mg */
```

- **小值**（如50mg）：更敏感，容易触发
- **大值**（如500mg）：较为迟钝，需要更强的震动

### 修改UART波特率

编辑 `Inc/config.h` 中的 `UART_BAUDRATE`。

## ADXL362 寄存器配置说明

### 关键寄存器

1. **THRESH_ACT (0x20)**：活动阈值
   - 1 LSB = 8mg (±2g范围)
   - 设置范围：0-255（映射到0-2440mg）

2. **TIME_ACT (0x21)**：活动时间
   - 检测到活动需要的确认时间

3. **ACT_INACT_CTL (0x27)**：活动/非活动控制
   - 比特0：ACT_EN（使能活动检测）

4. **INT2_MAP (0x2B)**：INT2中断映射
   - 比特0：活动中断到INT2

5. **POWER_CTL (0x2D)**：电源控制
   - 比特1：测量使能

## 故障排除

### 无data输出
1. 检查UART接线（TX/RX是否接反）
2. 确认波特率设置为9600
3. 检查USART2初始化是否成功

### shake检测不工作
1. 验证SPI连接（CS、CLK、MISO、MOSI）
2. 检查INT2是否正确连接到PB6
3. 调整阈值，尝试较小的值（如100mg）

### SPI通信失败
1. 检查CS信号（初始化应该是高电平）
2. 验证SPI极性和相位设置
3. 确认CS在传输时拉低，传输后拉高

## 技术规格

- **MCU**: STM32L432KC
- **传感器**: ADI ADXL362
- **SPI速率**: 最高2MHz
- **加速度范围**: ±2g
- **分辨率**: 12位
- **检测范围**: 40-2440mg
