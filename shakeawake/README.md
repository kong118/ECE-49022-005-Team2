# ADXL362 Shake Awake Low-Power Implementation - NUCLEO-L432KC

This project implements a **low-power Shake Awake** system using the ADXL362 accelerometer on the NUCLEO-L432KC microcontroller.

## Core Features

### Hardware Configuration
✅ **SPI Interface**
- SCLK: D13 (PB3)
- MISO: D12 (PB4)
- MOSI: D11 (PB5)
- CS: D3 (PB0) - Active Low
- INT2: D5 (PB6) - Interrupt Input

✅ **Output Control**
- PA7 (A6) - Active High, configurable pulse width

### Low-Power Features
✅ **ADXL362 Configuration**
- **Wake-up Mode**: Ultra-low power wake-up detection
- **100Hz Data Rate**: Power-optimized sampling
- **Activity Detection**: Automatic interrupt generation

✅ **MCU Power Management**
- **STOP2 Mode**: Deep sleep with µA-level consumption
- **INT2 Wake-up**: Triggered by acceleration detection
- **Auto Sleep**: Configurable inactivity timeout

## Quick Start

### Build
```bash
cd /Users/rayk/Documents/GitHub/ECE-49022-005-Team2/shakeawake
pio run
```

### Upload
```bash
pio run -t upload
```

### Monitor
```bash
pio device monitor --baud 115200
```

## Configuration

### Default Values

Defined in `include/config.h`:

```c
#define DEFAULT_ACTIVITY_THRESHOLD_MG        1500    /* 1500 mg */
#define DEFAULT_OUTPUT_PULSE_DURATION_MS     500     /* 500 ms */
#define DEFAULT_INACTIVITY_TIMEOUT_SEC       30      /* 30 seconds */
#define DEFAULT_ENABLE_STOP2_MODE            1       /* Enable STOP2 */
#define DEFAULT_DATA_RATE_HZ                 100     /* 100 Hz */
```

### Runtime Configuration

Add in `src/main.c` after initialization:

```c
/* Modify activity threshold */
Set_Activity_Threshold(1200);      /* 1200 mg */

/* Modify output pulse width */
Set_Output_Duration(800);          /* 800 ms */

/* Modify inactivity timeout */
Set_Inactivity_Timeout(60);        /* 60 seconds */

/* Disable STOP2 (keep MCU running) */
Disable_STOP2_Mode();

/* Enable STOP2 */
Enable_STOP2_Mode();
```

## Operation Flow

### Normal Operation (MCU Awake)
1. MCU runs in normal mode (4MHz)
2. ADXL362 continuously monitors acceleration
3. When threshold exceeded, INT2 interrupt triggered
4. INT2 wakes MCU if in STOP2
5. MCU reads acceleration and triggers PA7 pulse
6. Output returns low after configured duration

### Low-Power Operation (STOP2 Sleep)
1. MCU enters STOP2 after 30 seconds of inactivity
2. MCU consumes µA-level power
3. ADXL362 continues in Wake-up mode (mA level)
4. Activity detection triggers INT2
5. MCU wakes and processes acceleration data

## Power Consumption

| Mode | Current | Description |
|------|---------|-------------|
| **Run Mode (4MHz)** | ~2 mA | Normal operation |
| **STOP2 (RTC on)** | ~0.4 µA | Deep sleep |
| **ADXL362 Wakeup** | ~11 µA | 100Hz sampling |
| **Total (Sleeping)** | ~11.4 µA | Sensor dominated |

## Hardware Connections

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

## API Reference

### Configuration Functions
```c
void Set_Activity_Threshold(uint16_t threshold_mg);     /* Set activity threshold */
void Set_Output_Duration(uint16_t duration_ms);         /* Set output pulse width */
void Set_Inactivity_Timeout(uint16_t timeout_sec);      /* Set sleep timeout */
void Enable_STOP2_Mode(void);                           /* Enable STOP2 */
void Disable_STOP2_Mode(void);                          /* Disable STOP2 */
```

### Sensor Functions
```c
void ADXL362_Init_Wakeup_Mode(void);                    /* Initialize Wake-up mode */
void ADXL362_Get_Acceleration(accel_data_t *accel);     /* Read XYZ data */
uint32_t ADXL362_Get_Magnitude(accel_data_t *accel);    /* Calculate acceleration magnitude */
void ADXL362_Set_Activity_Threshold(uint16_t threshold);/* Set ADXL threshold */
```

### Power Management
```c
void Enter_STOP2_Mode(void);                            /* Enter STOP2 */
void Exit_STOP2_Mode(void);                             /* Exit STOP2 */
uint8_t Is_System_Awake(void);                          /* Check if MCU awake */
```

## Interrupt Handling

INT2 interrupt from ADXL362:
1. Sets `activity_detected_flag`
2. Wakes MCU if in STOP2
3. Updates last activity timestamp

Using global flags avoids SPI reads during sleep.

## Debugging Tips

### Verify Wake-up Mode
```c
uint8_t chip_id;
ADXL362_Read_Register(ADXL362_DEVID_AD, &chip_id);
if (chip_id == 0xAD) {
    /* Connection successful */
}
```

### Monitor STOP2 Entry
```c
/* Manually test STOP2 (disable interrupt wake) */
Disable_Sleep_Mode();
Enter_STOP2_Mode();     /* Wait for RESET button */
```

### Check INT2 Interrupt
```c
/* INT2 should go high when threshold exceeded */
/* Use oscilloscope or logic analyzer on PB6 */
```

## Optimization Tips

### Further Reduce Power
1. Disable UART/debug ports
2. Disable unused peripheral clocks
3. Use lower SPI clock rate
4. Increase STOP2 entry delay

### Improve Detection Accuracy
1. Enable data buffering (multi-sample averaging)
2. Increase ADXL362 sample rate
3. Fine-tune threshold for environment

## FAQ

**Q: STOP2 still consuming high power?**
A: Check for unused peripheral clocks. Use `HAL_PWREx_EnableLowPowerRegulator()` to optimize regulator.

**Q: INT2 interrupt not triggering?**
A: Verify:
- ADXL362 properly initialized
- INT2_MAP register correctly set
- EXTI interrupt enabled
- Threshold is reasonable

**Q: Output pulse width inaccurate?**
A: Check HAL_GetTick() and HAL_Delay() time base configuration.

## File Structure

```
shakeawake/
├── include/
│   ├── adxl362_lowpower.h      ← Sensor driver
│   ├── spi_gpio_config.h       ← SPI/GPIO config
│   ├── power_management.h      ← Power management
│   └── config.h                ← System configuration
├── src/
│   ├── main.c                  ← Main program
│   ├── adxl362_lowpower.c      ← Sensor implementation
│   ├── spi_gpio_config.c       ← SPI/GPIO implementation
│   ├── power_management.c      ← Power management implementation
│   └── config.c                ← Configuration implementation
└── platformio.ini              ← Project configuration
```

## Performance Specifications

| Metric | Value |
|--------|-------|
| **Data Rate** | 100 Hz |
| **Acceleration Range** | ±2g |
| **Resolution** | 1 mg/LSB |
| **INT2 Latency** | < 10 ms |
| **STOP2 Wake Latency** | < 20 ms |
| **Output Control Accuracy** | ±10 ms |

## References

- ADXL362 Datasheet
- STM32L432KC Reference Manual
- NUCLEO-L432KC User Manual
- STM32CubeL4 HAL Documentation

---

**Project Team**: ECE 49022 Team 2
**Last Updated**: March 4, 2026
