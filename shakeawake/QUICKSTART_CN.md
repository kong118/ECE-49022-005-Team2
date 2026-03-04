# ADXL362 Shake Awake 使用指南

## 快速配置

### 步骤 1: 编译
```bash
cd /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake
pio run
```

### 步骤 2: 上传
连接NUCLEO-L432KC开发板，运行：
```bash
pio run -t upload
```

开发板上的绿色LED指示灯应该先闪烁一次，表示系统已启动。

## 默认工作模式

系统初始化时：
- **活动阈值**: 1500 mg
- **输出脉冲**: 500 ms
- **睡眠超时**: 30秒
- **数据率**: 100 Hz
- **STOP2模式**: 已启用

## 常见应用配置

### 应用 1: 智能手机唤醒

用于通过轻微摇晃唤醒设备。

**修改** `src/main.c` 中的以下部分：
```c
/* 极度灵敏 */
Set_Activity_Threshold(800);       /* 800 mg */
Set_Output_Duration(200);          /* 200 ms 短脉冲 */
Set_Inactivity_Timeout(10);        /* 10秒快速睡眠 */
```

### 应用 2: 包裹监控

检测意外冲击或粗暴处理。

**修改** `src/main.c`：
```c
/* 低灵敏度，仅检测剧烈冲击 */
Set_Activity_Threshold(3000);      /* 3000 mg */
Set_Output_Duration(2000);         /* 2000 ms 长脉冲 */
Set_Inactivity_Timeout(60);        /* 60秒 */
```

### 应用 3: 工业振动监测

在高噪声工业环境中使用。

**修改** `src/main.c`：
```c
/* 高阈值，过滤背景噪声 */
Set_Activity_Threshold(4000);      /* 4000 mg */
Set_Output_Duration(500);          /* 500 ms */
Set_Inactivity_Timeout(120);       /* 2分钟 */
```

### 应用 4: 设备防盗

检测任何移动。

**修改** `src/main.c`：
```c
/* 极端灵敏，禁用STOP2睡眠 */
Set_Activity_Threshold(500);       /* 500 mg */
Set_Output_Duration(1000);         /* 1000 ms */
Disable_STOP2_Mode();              /* 保持MCU运行 */
```

## 阈值选择指南

选择合适的加速度阈值是关键。使用以下参考值：

| 加速度范围 | 应用场景 | 特点 |
|----------|---------|------|
| **500-1000 mg** | 轻微摇晃检测 | 极度灵敏，易误触 |
| **1000-1500 mg** | **推荐通用** | **平衡灵敏度** |
| **1500-2500 mg** | 中等冲击 | 稳定可靠 |
| **2500-4000 mg** | 强力冲击 | 忽略日常活动 |
| **4000+ mg** | 极端冲击 | 仅检测严重事件 |

### 如何确定最佳阈值

1. **从保守值开始**: 设置为 2000 mg
2. **逐步调整**: 每次降低 200 mg
3. **观察响应**: 记录何时触发
4. **找到临界点**: 在误触和漏检之间找平衡
5. **验证**: 在实际环境中测试 1 小时

## 功耗优化

### 方案 1: 极限低功耗（待命模式）

```c
/* 在 src/main.c 中修改 */
Set_Inactivity_Timeout(10);        /* 10秒进入STOP2 */
Enable_STOP2_Mode();               /* 启用*/
Set_Activity_Threshold(2000);      /* 确保不会误触 */
```

**预计功耗**: 11.4 µA (平均)

### 方案 2: 平衡功耗与性能

```c
Set_Inactivity_Timeout(30);        /* 30秒（默认） */
Enable_STOP2_Mode();
Set_Activity_Threshold(1500);
```

**预计功耗**: 12-20 mA (活跃) / 11.4 µA (睡眠)

### 方案 3: 最高性能（持续监控）

```c
Disable_STOP2_Mode();              /* 禁用睡眠 */
Set_Activity_Threshold(1000);      /* 灵敏 */
Set_Inactivity_Timeout(3600);      /* 无关 */
```

**预计功耗**: ~2 mA (持续)

## 调试步骤

### 步骤 1: 验证硬件连接

检查ADXL362芯片是否被正确识别：

修改 `src/main.c` 中的初始化部分为：
```c
uint8_t chip_id;
ADXL362_Read_Register(ADXL362_DEVID_AD, &chip_id);

if (chip_id == 0xAD) {
    Output_Set_High();   /* LED亮 - 连接正常 */
    HAL_Delay(1000);
    Output_Set_Low();
} else {
    while(1);            /* LED不亮 - 连接错误 */
}
```

### 步骤 2: 测试输出引脚

使用LED测试PA7输出：
```c
while(1) {
    Output_Set_High();
    HAL_Delay(500);
    Output_Set_Low();
    HAL_Delay(500);
}
```

LED应该每500ms闪烁一次。

### 步骤 3: 测试INT2中断

使用示波器或逻辑分析仪监测PB6（INT2）：
- 应该在加速度超过阈值时呈现脉冲
- 可以用手摇晃ADXL362来触发

### 步骤 4: 验证STOP2进入

添加调试输出：
```c
if (system_in_stop2) {
    // STOP2 mode - consume minimal power
    Output_Set_Low();
} else {
    // Run mode - normal operation
}
```

## 常见错误排查

### 错误 1: 编译失败

**症状**: `undefined reference to 'ADXL362_Init'`

**解决**:
```bash
pio run -t clean
pio run
```

### 错误 2: 上传失败

**症状**: `NUCLEO-L432KC not found`

**解决**:
1. 确认USB线连接牢固
2. 检查设备管理器是否识别开发板
3. 尝试重新连接USB或重启电脑

### 错误 3: Output引脚不响应

**症状**: PA7（A6）始终为低电平

**检查清单**:
- [ ] PA7是否正确连接到LED或示波器
- [ ] ADXL362芯片ID是否读取正确 (0xAD)
- [ ] 加速度幅值是否真的超过阈值
- [ ] INT2中断是否正确生成

试试手动连接PA7到+3.3V看LED是否亮。

### 错误 4: STOP2模式不工作

**症状**: 30秒后MCU没有睡着，功耗仍然很高

**检查**:
- [ ] 是否正确启用了STOP2？
- [ ] 是否有中断持续发生?
- [ ] Inactivity timeout是否设置得太长?

试试临时禁用INT2中断观察。

## 性能测试

### 测试 1: 活动检测性能

```c
/* 在 main 循环中添加计数器 */
static uint32_t trigger_count = 0;

if (activity_detected_flag) {
    activity_detected_flag = 0;
    trigger_count++;
    /* 查看 trigger_count 值 */
}

/* 在调试器中观察 trigger_count */
```

应该在摇晃时快速增加。

### 测试 2: 输出脉冲宽度

使用示波器测量PA7脉冲宽度。应该在±10ms范围内与设置值一致。

### 测试 3: STOP2功耗

- 测量NUCLEO-L432KC的VDD电流
- 30秒不动后应该看到显著的电流下降
- 用万用表的mA档测量

## 高级功能

### 多级阈值检测

```c
accel_data_t accel;
uint32_t magnitude;

ADXL362_Get_Acceleration(&accel);
magnitude = ADXL362_Get_Magnitude(&accel);

if (magnitude < 1000) {
    /* 轻微活动 - 不响应 */
} else if (magnitude < 2000) {
    /* 中等活动 - 短脉冲 */
    Set_Output_Duration(200);
    Output_Trigger();
} else {
    /* 强烈活动 - 长脉冲 */
    Set_Output_Duration(1000);
    Output_Trigger();
}
```

### 动态阈值调整

根据应用状态动态调整阈值：
```c
typedef enum {
    STATE_IDLE,
    STATE_ACTIVE,
    STATE_CRITICAL
} system_state_t;

void Update_Threshold(system_state_t state) {
    switch(state) {
        case STATE_IDLE:
            Set_Activity_Threshold(500);   /* 极度灵敏 */
            break;
        case STATE_ACTIVE:
            Set_Activity_Threshold(1500);  /* 正常 */
            break;
        case STATE_CRITICAL:
            Set_Activity_Threshold(3000);  /* 不易中断 */
            break;
    }
}
```

## 获取帮助

如果遇到问题：

1. 检查本指南的故障排除部分
2. 查看 README.md 或 README_CN.md
3. 参考ADXL362数据手册
4. 参考STM32L432KC参考手册

## 快速参考

```c
/* 设置阈值（mg） */
Set_Activity_Threshold(1500);

/* 设置输出脉冲宽度（ms） */
Set_Output_Duration(500);

/* 设置睡眠超时（秒） */
Set_Inactivity_Timeout(30);

/* 启用/禁用STOP2低功耗模式 */
Enable_STOP2_Mode();
Disable_STOP2_Mode();

/* 读取加速度 */
accel_data_t accel;
ADXL362_Get_Acceleration(&accel);
uint32_t mag = ADXL362_Get_Magnitude(&accel);

/* 手动控制输出 */
Output_Set_High();
Output_Set_Low();
Output_Trigger();  /* 输出配置的脉冲宽度 */
```

---

**创建时间**: 2026年3月4日
**维护者**: ECE 49022 Team 2
