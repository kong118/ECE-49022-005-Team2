# Shake Awake 快速参考指南

## 快速开始

### 1. 编译项目
```bash
cmake -B build
cmake --build build
```

### 2. 烧写到MCU
使用STM32CubeIDE或STM32Programmer烧写生成的hex文件

### 3. 测试功能
摇晃MCU开发板，LED应该闪烁表示检测到shake事件

---

## 调整灵敏度

### 修改默认阈值
编辑 `Src/main.c` 第16行：

```c
#define ACTIVITY_THRESHOLD 150  /* 改为你需要的值 */
```

### 推荐值参考

| 应用场景 | 阈值(mg) | 说明 |
|---------|---------|------|
| 检测轻微触碰 | 20-50 | 非常灵敏，可能误触发 |
| 检测正常摇晃 | 50-150 | **推荐值** |
| 检测剧烈动作 | 150-300 | 仅检测明显动作 |
| 检测重击 | 300-500 | 仅检测强冲击 |

---

## API 快速参考

### 初始化
```c
ShakeDetect_Init(ACTIVITY_THRESHOLD);  // 初始化shake检测
```

### 运行检测
```c
ShakeDetect_Process();  // 在主循环中调用
```

### 检查事件
```c
if(ShakeDetect_IsAwake()) {
    // 检测到shake事件
    ShakeDetect_ClearAwake();  // 清除标志
}
```

### 动态调整阈值
```c
Set_ActivityThreshold(200);           // 设置新阈值
uint16_t current = Get_ActivityThreshold();  // 获取当前阈值
```

### 获取统计信息
```c
uint32_t count = Get_WakeEventCount();  // 获取总检测次数
```

---

## LED 指示灯

- **位置**: D10 (PA11) on NUCLEO-L432KC
- **行为**: 每次检测到shake时闪烁一次（active‑high）
- **用途**: 视觉/外部触发信号

---

## 故障排查

| 问题 | 原因 | 解决方案 |
|------|------|--------|
| LED不闪烁 | 硬件未连接 | 检查SPI和INT2连接 |
| 频繁误触发 | 阈值太低 | 增大ACTIVITY_THRESHOLD |
| 无法检测shake | 阈值太高 | 减小ACTIVITY_THRESHOLD |
| SPI错误 | 时钟配置问题 | 检查系统时钟初始化 |

---

## 硬件管脚总结

```
ADXL362          NUCLEO-L432KC
═════════════════════════════
VDD    ←→   3.3V
GND    ←→   GND
CS/nCS ←→   D3 (PB0, active‑low during SPI transfers)
SPI1:
  SCLK ←→   A4 (PA5)
  MOSI ←→   A6 (PA7)
  MISO ←→   A5 (PA6)
INT2   ←→   D5 (PB6)
Output signal (wake LED/trigger) ←→ D10 (PA11, active‑high)
```

---

## 性能指标

- **最大SPI速率**: 4 MHz
- **系统频率**: 80 MHz
- **响应时间**: < 100ms
- **功耗**: 低于5mA (测量模式)

---

## 扩展建议

1. **添加UART调试输出** - 输出加速度值和检测状态
2. **实现低功耗模式** - 改进电池寿命
3. **添加手势识别** - 识别特定的摇晃模式
4. **融合多传感器** - 结合陀螺仪数据

---

## 联系和支持

遇到问题？检查 CONFIG.md 获取详细文档。
