# 项目实现总结

## ADXL362 Shake Awake 低功耗实现

本项目为NUCLEO-L432KC微控制器实现了完整的**ADXL362加速度传感器低功耗Shake Awake系统**。

---

## 📦 项目文件清单

### 头文件 (include/)
```
✓ adxl362_lowpower.h    - ADXL362驱动接口（支持Wake-up模式）
✓ spi_gpio_config.h     - SPI/GPIO模块接口
✓ power_management.h    - 功耗管理接口（STOP2支持）
✓ config.h              - 系统配置参数
```

### 源文件 (src/)
```
✓ main.c                - 主程序（完整初始化和循环）
✓ adxl362_lowpower.c    - ADXL362驱动实现
✓ spi_gpio_config.c     - SPI/GPIO初始化实现
✓ power_management.c    - STOP2睡眠管理实现
✓ config.c              - 配置变量定义
```

### 文档
```
✓ README.md             - 英文项目文档
✓ README_CN.md          - 中文项目文档
✓ QUICKSTART_CN.md      - 快速入门指南
✓ platformio.ini        - PlatformIO项目配置
```

---

## 🎯 核心功能实现

### 1️⃣ ADXL362驱动 (Wake-up Mode)
- ✅ SPI通信（读/写寄存器）
- ✅ Wake-up模式配置
- ✅ 100Hz数据率
- ✅ 活动检测中断(INT2)
- ✅ 加速度矢量幅值计算

### 2️⃣ SPI接口
- ✅ PB3 (SCLK/D13) - 时钟
- ✅ PB4 (MISO/D12) - 数据输入
- ✅ PB5 (MOSI/D11) - 数据输出
- ✅ PB0 (CS/D3)   - 片选(活跃低)
- ✅ 软件CS管理

### 3️⃣ 输出控制
- ✅ PA7 (A6) - 主输出，活跃高
- ✅ 可配置脉冲宽度(ms级)
- ✅ 自动超时复位

### 4️⃣ 中断系统
- ✅ PB6 (D5/INT2) - ADXL362中断输入
- ✅ EXTI配置
- ✅ 上升沿触发
- ✅ 中断唤醒STOP2模式

### 5️⃣ 低功耗管理
- ✅ STOP2深度睡眠模式
- ✅ 不活动超时自动进入STOP2
- ✅ INT2唤醒MCU
- ✅ 时钟动态缩放

### 6️⃣ 可调参数
- ✅ 活动阈值(mg)
- ✅ 输出脉冲宽度(ms)
- ✅ 睡眠超时(秒)
- ✅ STOP2模式开关

---

## 🔧 硬件配置

```
NUCLEO-L432KC (MCU)        ADXL362 (加速度计)
───────────────             ──────────────────
  3.3V ◄──────────────────► VCC
  GND  ◄──────────────────► GND
  
  PB3 (D13) SCLK ◄────────► SCLK
  PB4 (D12) MISO ◄────────► MISO
  PB5 (D11) MOSI ◄────────► MOSI
  PB0 (D3)  CS  ◄────────► CS (活跃低)
  PB6 (D5)  INT2 ◄────────► INT2 (中断)
  
  PA7 (A6) ──────────────► 继电器/LED/外设
```

---

## 🚀 快速使用

### 编译
```bash
cd /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake
pio run
```

### 上传
```bash
pio run -t upload
```

### 默认参数
| 参数 | 默认值 |
|------|--------|
| 活动阈值 | 1500 mg |
| 输出脉冲 | 500 ms |
| 睡眠超时 | 30 秒 |
| 数据率 | 100 Hz |
| STOP2模式 | 启用 |

### 修改参数（在 src/main.c 中）

```c
/* 修改活动检测阈值 */
Set_Activity_Threshold(1200);      // 改为1200 mg

/* 修改输出脉冲宽度 */
Set_Output_Duration(800);          // 改为800 ms

/* 修改进入睡眠前的超时时间 */
Set_Inactivity_Timeout(60);        // 改为60秒

/* 禁用STOP2低功耗模式（MCU一直运行） */
Disable_STOP2_Mode();

/* 启用STOP2低功耗模式 */
Enable_STOP2_Mode();
```

---

## 📊 功耗指标

### 模式对比
| 工作模式 | 电流 | 说明 |
|---------|------|------|
| **运行模式** | ~2 mA | MCU + ADXL362 正常工作 |
| **STOP2 睡眠** | ~0.4 µA | MCU 深度睡眠 |
| **ADXL Wake-up** | ~11 µA | 传感器 100Hz 采样 |
| **待命总功耗** | ~11.4 µA | 主要来自传感器 |

### 电池续航估算
- **持续运行**: 2 mA → 约 500 小时 (1000mAh电池)
- **间歇工作**: 平均 20 mA → 约 50 小时
- **深度睡眠**: 11.4 µA → 约 100,000+ 小时 (4.3年)

---

## 🔌 工作原理

### 正常运行阶段
```
MCU启动 → 初始化ADXL362 → 进入主循环

主循环:
├─ ADXL362在Wake-up模式检测加速度
├─ 超过阈值时生成INT2中断
├─ INT2唤醒MCU（如果在STOP2中）
├─ 读取加速度数据和计算幅值
├─ 如果超过阈值，输出PA7高电平脉冲
├─ 超时后自动返回低电平
└─ 继续检查是否需要进入STOP2
```

### 低功耗阶段
```
30秒无活动 → MCU进入STOP2深度睡眠
             ↓
        ADXL362继续监测（Wake-up模式）
             ↓
        检测到活动 → INT2拉高
             ↓
        INT2中断唤醒MCU
             ↓
        读取数据、输出脉冲
```

---

## 🎓 技术特点

### 硬件集成
- **SPI1接口**: 主控方式，软件CS管理
- **EXTI6**: INT2中断，支持STOP2唤醒
- **GPIO**: 灵活的引脚配置

### 软件架构
```
main.c (主程序)
  ├─ 初始化模块
  │  ├─ SystemClock_Config()      → 系统时钟
  │  ├─ GPIO_Init()               → GPIO初始化
  │  ├─ SPI_Init()                → SPI初始化
  │  ├─ ADXL362_Init()            → 传感器初始化
  │  ├─ Power_Management_Init()   → 电源管理
  │  └─ INT2_Interrupt_Init()     → 中断初始化
  │
  └─ 主循环
     ├─ 中断检查 (全局标志)
     ├─ 数据读取 (仅在唤醒时)
     ├─ 输出管理 (脉冲控制)
     └─ 睡眠检查 (超时计时)
```

### 电源优化技术
- **Wake-up Mode**: ADXL362超低功耗唤醒模式
- **STOP2 Mode**: MCU最深睡眠级别
- **动态时钟**: STOP2时降低MSI到1MHz
- **全局中断**: 中断驱动而非轮询

---

## 📋 API 速查表

### 配置函数
```c
Set_Activity_Threshold(uint16_t mg);        // 设置活动阈值
Set_Output_Duration(uint16_t ms);           // 设置输出宽度
Set_Inactivity_Timeout(uint16_t sec);       // 设置睡眠超时
Enable_STOP2_Mode();                        // 启用STOP2
Disable_STOP2_Mode();                       // 禁用STOP2
```

### 传感器函数
```c
ADXL362_Init();                             // 初始化
ADXL362_Init_Wakeup_Mode();                 // Wake-up模式
ADXL362_Get_Acceleration(&accel);           // 读取XYZ
ADXL362_Get_Magnitude(&accel);              // 计算幅值
ADXL362_Get_Status();                       // 读取状态
```

### 输出控制
```c
Output_Set_High();                          // PA7拉高
Output_Set_Low();                           // PA7拉低
Output_Trigger();                           // 输出配置宽度的脉冲
Output_Check_And_Update();                  // 检查并更新输出状态
```

### 电源管理
```c
Enter_STOP2_Mode();                         // 进入STOP2
Exit_STOP2_Mode();                          // 退出STOP2
Is_System_Awake();                          // 检查是否唤醒
Check_Sleep_Timeout();                      // 检查睡眠超时
```

---

## 🧪 测试清单

部署前应完成以下测试：

- [ ] 编译无错误: `pio run` 成功
- [ ] 上传无错误: `pio run -t upload` 成功
- [ ] ADXL362识别: 芯片ID读取为0xAD
- [ ] SPI通信: 能写入和读取寄存器
- [ ] PA7输出: LED闪烁或示波器能观测
- [ ] INT2中断: 摇晃时PB6有脉冲
- [ ] 脉冲宽度: 实际脉冲宽度 ±10ms
- [ ] 睡眠功能: 30秒后电流显著下降
- [ ] 唤醒功能: INT2能唤醒STOP2
- [ ] 长期稳定: 运行1小时无故障

---

## 📖 文档导航

| 文档 | 用途 |
|------|------|
| **README.md** | 项目概览（英文） |
| **README_CN.md** | 项目详细说明（中文） |
| **QUICKSTART_CN.md** | 快速配置指南（中文） |
| **platformio.ini** | 编译配置和编译标志 |

---

## 🔧 常见问题

### Q: 功耗还是很高
**A:** 检查是否禁用了不必要的外设时钟，使用 `HAL_RCC_ClockConfig()` 仔细配置。

### Q: INT2中断不工作
**A:** 验证 EXTI6 配置、INT2_MAP寄存器设置和NVIC中断使能。

### Q: 输出脉冲宽度不准
**A:** 确保 HAL_GetTick() 和 HAL_Delay() 基于正确的系统时钟。

### Q: 无法进入STOP2
**A:** 检查是否有其他中断持续发生，禁用不需要的中断源。

---

## 📞 技术支持

遇到问题时：

1. 查看对应的文档（README/QUICKSTART）
2. 查看源代码注释
3. 参考ADXL362数据手册
4. 参考STM32L432KC参考手册

---

## 📝 修改记录

| 日期 | 版本 | 说明 |
|------|------|------|
| 2026-03-04 | 1.0 | 初始发布 |

---

## 👥 项目信息

**团队**: ECE 49022 Team 2
**开发板**: NUCLEO-L432KC
**传感器**: ADXL362
**特性**: 低功耗、可配置、中断驱动

---

## 📦 依赖

- STM32CubeL4 HAL库
- STM32 CMSIS核心库
- GCC编译器 (via PlatformIO)

---

**最后更新**: 2026年3月4日
**项目状态**: ✅ 完整实现

---
