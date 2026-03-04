# ADXL362 Shake Awake 完整实现

## 实现总览

这是一个基于STM32L432KC MCU的ADXL362 3轴加速度计shake awake系统的完整实现。

### 核心特性

✅ **SPI通信接口**
- 4线SPI协议（SCLK, MOSI, MISO, CS）
- 标准ADXL362寄存器读写

✅ **Shake Awake功能**  
- 可配置活动阈值（40-2440mg范围）
- INT2活动中断输出
- PA11输出控制（主动高）

✅ **实时数据监控**
- XYZ三轴加速度读取
- G单位显示
- 通过UART输出（9600波特率）

✅ **灵活配置**
- 阈值可在config.h中调整
- 宏定义集中管理参数

## 硬件连接

```
NUCLEO-L432KC Board          ADXL362 传感器
====================          ===============
D4 (PA5)    ———— SCLK ————→ SCLK
D6 (PA7)    ———— MOSI ————→ MOSI  
D5 (PA6)    ←——— MISO ←———  MISO
D3 (PB0)    ———— CS ————→ CS (低有效)
D5 (PB6)    ←——— INT2 ←———  INT2

D10 (PA11)  → 主动高输出（shake时拉高）

UART:
D1 (PA2)    → TX (串口输出)
D0 (PA3)    ← RX
```

## 文件结构

```
Src/
├── main.c           # 主程序，包含初始化和主循环
└── adxl362.c        # ADXL362驱动实现

Inc/
├── adxl362.h        # ADXL362驱动API
└── config.h         # 可配置参数
```

## 编译命令

```bash
# 在项目根目录运行
cmake -B build
cd build
make

# 生成：
# - shakeawake.elf   (ELF可执行文件)
# - shakeawake.hex   (INTEL HEX)
# - shakeawake.bin   (二进制)
```

## 烧写到开发板

使用STM32 ST-Link程序员或其他工具：

```bash
# 使用openocd
openocd -f board/stm32l432kc.cfg -c "program shakeawake.elf verify reset"

# 或使用CubeProgrammer
STM32_Programmer_CLI -c port=SWD -w shakeawake.bin 0x08000000
```

## 使用方法

### 1. 连接串口
- 波特率：9600 bps
- 数据位：8
- 停止位：1  
- 校验位：无

### 2. 启动程序
- 重置开发板
- 应看到欢迎消息和初始化信息

### 3. 测试Shake功能
- 轻轻摇晃开发板
- 观察PA11（D10）LED或输出
- 串口应显示"SHAKE DETECTED"

### 4. 调整敏感度
编辑 `Inc/config.h`：
```c
#define ADXL362_THRESHOLD_MG    300     /* 改为50-500等值 */
```
- **小值**：更敏感
- **大值**：较为迟钝

## 串口输出示例

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
X:  0.10 G | Y: -0.06 G | Z:  1.02 G
X:  0.89 G | Y:  0.65 G | Z:  1.15 G [SHAKE DETECTED]
X:  0.15 G | Y: -0.10 G | Z:  1.00 G
```

## 关键源代码片段

### ADXL362初始化
```c
ADXL362_Init(300);  /* 初始化，阈值300mg */
```

### 读取加速度数据
```c
ADXL362_Data_t data;
ADXL362_ReadData(&data);
float x_g = ADXL362_ConvertToG(data.x);
printf("X: %.2f G\r\n", x_g);
```

### 设置新阈值
```c
ADXL362_SetThreshold(150);  /* 改为150mg */
```

## 寄存器配置

| 寄存器 | 地址 | 用途 |
|--------|------|------|
| THRESH_ACT | 0x20 | 活动阈值 |
| TIME_ACT | 0x21 | 活动时间 |
| ACT_INACT_CTL | 0x27 | 活动控制 |
| INT2_MAP | 0x2B | INT2映射 |
| POWER_CTL | 0x2D | 电源控制 |
| XDATAL/H | 0x0E/0F | X寄存器 |
| YDATAL/H | 0x10/11 | Y寄存器 |
| ZDATAL/H | 0x12/13 | Z寄存器 |

## 中断处理机制

当前实现是**轮询INT2**：
```c
uint8_t int2_state = (GPIOB->IDR >> 6) & 0x01;  /* 读取PB6 */
if (int2_state && !prev_int2_state) {             /* 检测上升沿 */
    Output_SetHigh();                             /* PA11拉高 */
}
```

## 功耗特性

| 模式 | 功耗 |
|------|------|
| 测量（正常） | ~50µA |
| 待机 | ~10µA |
| 睡眠（可选） | <1µA |

## 技术规格

| 项目 | 值 |
|------|-----|
| MCU | STM32L432KC |
| 频率 | 4MHz(内部) |
| SPI速率 | 2MHz |
| UART波特率 | 9600 bps |
| 分辨率 | 12位（4mg/LSB） |
| 加速度范围 | ±2g |
| 采样周期 | 100Hz |

## 常见问题

**Q: 没有看到串口输出**
A: 检查UART接线、波特率设置、USB驱动程序

**Q: Shake没被检测到**
A: 降低阈值、检查INT2连接、验证SPI通信

**Q: 获得乱码输出**
A: 确认波特率为9600、检查晶振频率

## 后续改进方向

1. 使用EXTI中断代替轮询
2. 实现FIFO缓冲读取
3. 添加低功耗睡眠模式
4. 支持其他运动事件（双击、自由落体等）

## 联系与支持

参考 README.md 和 SETUP_GUIDE.md 获取更多细节。

---

**版本**: 1.0  
**日期**: 2026年3月4日  
**开发板**: NUCLEO-L432KC  
**传感器**: Analog Devices ADXL362
