# ADXL362 Shake Awake - 快速开始指南

## 硬件连接清单

在开始之前，请验证以下连接：

### NUCLEO-L432KC 引脚配置

| 功能 | 引脚编号 | STM32引脚 | 状态 |
|------|---------|----------|------|
| SPI CLK | D4 | PA5 | ✓ |
| SPI MISO | D5 | PA6 | ✓ |
| SPI MOSI | D6 | PA7 | ✓ |
| SPI CS | D3 | PB0 | ✓ (低有效) |
| INT2 | D5 | PB6 | ✓ |
| Output | D10 | PA11 | ✓ (主动高) |
| UART TX | D1 | PA2 | ✓ |
| UART RX | D0 | PA3 | ✓ |

## 快速编译步骤

### 方法1：使用VS Code + CMake插件

1. 打开项目文件夹
2. 选择Kit（如果提示）
3. 点击 CMake 侧边栏的构建按钮
4. 获得生成的 `.elf` 和 `.bin` 文件

### 方法2：手动CMake编译

```bash
# 从项目根目录运行
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make
```

### 方法3：使用STM32CubeIDE

1. File → New STM32 Project
2. 选择现有代码
3. 导入此项目
4. 右键 → Build Project

## 编程到NUCLEO板

### 使用STM32CubeProgrammer

```bash
STM32_Programmer_CLI -c port=SWD -w shakeawake.bin 0x08000000 -v
```

### 使用OpenOCD

```bash
# 创建 openocd.cfg (STM32L432)
openocd -f openocd.cfg
# 在gdb中：
load shakeawake.elf
```

### 使用ST-Link Utility

1. 连接ST-Link
2. 选择 STM32L432KC 设备
3. 加载 shakeawake.bin
4. 编程

## 设置串口终端

### Windows - PuTTY
1. Session Type: Serial
2. Serial line: COM? (设备管理器查看)
3. Speed: 9600
4. 点击 Open

### Windows - Tera Term
1. File → New Connection
2. Serial port: COM?
3. Setup → Serial port → 9600 bps, 8 bits, no parity
4. 连接

### Linux/Mac - minicom
```bash
minicom -D /dev/ttyUSB0 -b 9600
```

### Linux/Mac - screen
```bash
screen /dev/ttyUSB0 9600
```

## 测试ADXL362连接

1. 启动终端程序（波特率9600）
2. 物理重置NUCLEO板或按下复位按钮
3. 应该看到启动消息：
```
========================================
ADXL362 Shake Awake Demo
========================================
Initializing ADXL362...
ADXL362 Initialized
Threshold: 300mg
Waiting for shake events...
========================================
```

4. 轻轻摇晃开发板
5. 应该看到加速度数据更新，并在检测到shake时显示 `[SHAKE DETECTED]`

## 常见问题排除

### 问题：没有看到任何串口输出

**原因**：UART配置或连接问题

**解决方案**：
- 检查 TX/RX 串口线接线（crossed in breadboard designs)
- 验证波特率为 9600
- 检查USART2_Init() 中的AF设置（应为AF7）
- 重新编程板子

### 问题：看到乱码

**原因**：波特率不匹配

**解决方案**：
- 在终端软件中尝试其他波特率
- 检查 UART2->BRR 寄存器设置
- 验证系统时钟频率（代码假设4MHz）

### 问题：Shake没有被检测到

**原因**：阈值太高或INT2连接问题

**解决方案**：
- 在 config.h 中降低 ADXL362_THRESHOLD_MG（尝试100mg）
- 检查 INT2 (PB6) 的物理接线
- 在 ADXL362_Init() 中验证寄存器写入
- 使用示波器检查INT2信号

### 问题：SPI通信失败

**原因**：片选或时钟问题

**解决方案**：
- 检查 CS (PB0) 是否正确拉低/拉高
- 验证 SPI2 的 AF5 设置
- 检查 SPI CLK 波形（示波器）
- 尝试降低 SPI 速率（改变预分频器）

## 调试技巧

### 启用调试信息

在 adxl362.c 中的 ADXL362_ReadReg() 后添加：
```c
// 临时调试
printf("Read Reg 0x%02X: 0x%02X\r\n", reg, result);
```

### 检查设备ID

添加到 main.c：
```c
uint8_t devid = ADXL362_ReadReg(ADXL362_REG_DEVID);
printf("ADXL362 Device ID: 0x%02X (expected 0xF2)\r\n", devid);
```

### 使用逻辑分析仪

监测：
- SPI CLK (PA5)
- SPI MOSI (PA7)
- SPI MISO (PA6)
- CS (PB0)

## 性能指标

| 参数 | 值 |
|------|-----|
| 检测延迟 | < 100ms |
| 分辨率 | 4mg (12位读数) |
| 功耗 | < 50µA (测量模式) |
| 采样率 | 100Hz (内部) |

## 下一步

- 调整 ADXL362_THRESHOLD_MG 以满足应用需求
- 添加EEPROM存储用户设置
- 实现更高级的运动检测（FIFO, 中断驱动）
- 添加双击检测
- 集成到更大的应用
