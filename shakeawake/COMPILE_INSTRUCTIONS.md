# 编译和故障排除说明

## 编译方式

### 方式 1: VS Code PlatformIO 插件 (推荐)

1. **打开项目**
   - 在 VS Code 中打开 `/Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake` 文件夹

2. **进行编译**
   - 点击状态栏左下角的 PlatformIO 图标
   - 或按 `Ctrl+Alt+B` (Windows/Linux) 或 `Cmd+Shift+B` (macOS)
   - 选择"Build"编译项目

3. **查看输出**
   - 编译输出会在 VS Code 的终端中显示
   - 如果编译成功，会显示"✓ SUCCESS"

### 方式 2: 命令行 (如果已安装 PlatformIO)

```bash
cd /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake
platformio run
```

## 如果仍然遇到编译错误

### 问题: 找不到 HAL 头文件

**解决**:
1. 确保 PlatformIO 已正确安装 STM32 框架
2. 在 VS Code 中重新加载项目: `Cmd+Shift+P` → "Developer: Reload Window"
3. 清理构建文件: `platformio run -t clean` 然后重新编译

### 问题: EXTI4_15_IRQn 未定义

**解决**:
- 已在最新版本中修复，使用 `__HAL_RCC_SYSCFG_CLK_ENABLE()` 宏启用SYSCFG时钟

### 问题: DEFAULT_ACTIVITY_THRESHOLD_MG 未定义

**解决**:
- 已更新 `adxl362_lowpower.c` 以包含 `config.h`
- 确保所有 #include 都在文件顶部

## 关键修复清单

✓ 已添加所有必要的 STM32 HAL 头文件
✓ 已修复 EXTI 配置以使用合适的时钟启用
✓ 已确保所有源文件包含正确的头文件  
✓ 已移除不适用的 LL_EXTI 调用
✓ 已使用正确的中断处理器名称

## 快速检查清单

在编译前：

- [ ] 在 VS Code 中打开正确的文件夹 (shakeawake)
- [ ] 确认 platformio.ini 存在于项目根目录
- [ ] 所有源文件在 `src/` 目录中
- [ ] 所有头文件在 `include/` 目录中
- [ ] 没有编辑未保存的文件

## 上传到开发板

编译成功后：

1. 使用 USB 线连接 NUCLEO-L432KC 开发板
2. 在 VS Code 中点击 PlatformIO 图标
3. 选择"Upload"或按相应快捷键
4. 等待上传完成（应该看到"✓ SUCCESS"）

## 监控串口输出 (如需要)

```bash
# 如果已安装 platformio
platformio device monitor --port /dev/cu.usbmodem* --baud 115200
```

或在 VS Code 中：
- 点击 PlatformIO 图标
- 选择"Serial Port Monitor"

## 如果编译仍失败

请检查：

1. **文件完整性**: 运行
   ```bash
   find /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake -name "*.c" -o -name "*.h" | wc -l
   ```
   应该有 9 个 C 文件和 4 个 H 文件

2. **包含语句**: 检查所有 .c 文件的开头是否都有必要的 #include

3. **PlatformIO 版本**: 确保使用最新版本的 PlatformIO 插件

## 常见错误信息及解决方案

| 错误 | 原因 | 解决方案 |
|------|------|--------|
| `undefined reference` | 函数未实现 | 检查 .c 文件是否在 src/ 目录 |
| `no such file` | 头文件找不到 | 检查 include/ 目录权限 |
| `conflicting types` | 函数声明冲突 | 检查是否有重复的 .c 文件 |

---

**最后更新**: 2026年3月4日
