# Production Deployment Guide

## 🚀 Deployment Checklist

### Pre-Deployment Verification

#### Hardware Validation
- [ ] STM32F746G board powers on correctly
- [ ] All LEDs functioning
- [ ] USB connection stable
- [ ] Camera module detected (I2C scan)
- [ ] Serial debug output working
- [ ] Clock frequency verified (216 MHz)
- [ ] Power supply stable (3.3V ±5%)

#### Firmware Testing
- [ ] Build completes with zero warnings
- [ ] Flash successful via st-flash
- [ ] Serial output appears immediately after reset
- [ ] All 60 test frames process without errors
- [ ] Average inference time < 20ms
- [ ] No memory corruption detected
- [ ] Inference accuracy > 97%

#### Model Validation
- [ ] Model file size exactly 45128 bytes
- [ ] Model checksum verified
- [ ] INT8 quantization applied
- [ ] Test set accuracy >= 98%
- [ ] All 10 digit classes working
- [ ] Edge cases handled (blurry, rotated, etc.)

#### Documentation
- [ ] README updated with latest info
- [ ] Configuration documented
- [ ] Known issues listed
- [ ] Performance metrics recorded
- [ ] Support contact info provided

### Deployment Procedure

#### Step 1: Final Build
```bash
cd firmware/stm32f7_mnist
make clean
make -j4
make size
```

**Verify:**
- Binary size < 512 KB
- RAM usage < 150 KB
- Flash usage < 400 KB

#### Step 2: Flash Verification
```bash
# Erase device
st-flash erase

# Flash firmware
st-flash write build/stm32f7_mnist.bin 0x08000000

# Verify (optional)
st-flash read 0x08000000 0x80000 /tmp/verify.bin
```

#### Step 3: Functional Testing
```bash
# Connect to serial monitor
python scripts/serial_monitor.py /dev/ttyUSB0 115200

# Expected output:
# ==========================================
# === STM32F7 MNIST Edge AI System ===
# Build: Jan 19 2025 10:30:45
# System Clock: 216 MHz
# Tensor Arena: 80 KB
# ==========================================
#
# Initializing MNIST inference engine...
# MNIST initialized successfully
# Model size: 45128 bytes
#
# Starting real-time inference...
#
# [Frame     1] Predicted: 5 | Confidence: 250 | Time: 15 ms
```

#### Step 4: Performance Verification

Monitor for:
- **Consistency**: Latency variation < ±5ms
- **Accuracy**: No misclassifications on known patterns
- **Stability**: No crashes after 1000+ frames
- **Memory**: No heap fragmentation

#### Step 5: Power Profiling (Optional)

```bash
# Measure power consumption
# Using multimeter or power analyzer:
# - Idle: ~15 µA (STOP mode)
# - Inference: ~180 mA @ 3.3V
# - Average (1 fps): ~5 mA
```

### Production Configuration

#### Modify config.h for Production

```c
/* Disable debug output for reduced power */
#define DEBUG_UART_ENABLED      0

/* Optimize for power */
#define ENABLE_POWER_SAVING     1
#define SLEEP_MODE              2  /* STOP mode */
#define INFERENCE_DELAY_MS      1000  /* 1 second between inferences */

/* Tighten error checking */
#define DEBUG_TIMING            0
#define DEBUG_MEMORY            0
```

#### Rebuild for Production

```bash
make clean
make PRODUCTION=1 -j4
```

### Monitoring & Maintenance

#### In-Field Monitoring

Track metrics:
- Inference latency (moving average)
- Accuracy per digit class
- Error rate
- Power consumption
- Uptime/crashes

#### Error Logging

Implement circular buffer for errors:
```c
typedef struct {
    uint32_t timestamp;
    uint16_t error_code;
    uint8_t  context;
} error_log_t;
```

#### Remote Updates

Support OTA (Over-The-Air) updates:
1. Bootloader handles update verification
2. Model files can be updated from SD card
3. Firmware updates via serial or Ethernet (advanced)

### Troubleshooting Deployment Issues

#### Issue: "Model not found" error
**Solution:**
- Verify model binary embedded in flash
- Check model address matches linker script
- Validate model file size (45128 bytes)

#### Issue: Intermittent inference failures
**Solution:**
- Check stack size (minimum 8KB)
- Reduce inference frequency
- Enable watchdog timer to reset on hang

#### Issue: Power consumption higher than expected
**Solution:**
- Disable serial debug output
- Use sleep modes between inferences
- Profile individual components
- Check for busy-waiting loops

#### Issue: Accuracy degradation over time
**Solution:**
- Check for sensor drift
- Validate model checksum
- Verify preprocessing pipeline
- Retrain if distribution changed

### Security Considerations

#### Model Protection
- Encrypt model in flash (optional)
- Implement model signature verification
- Prevent unauthorized model replacement

#### Communication Security
- Use UART with authentication for updates
- Implement command verification
- Log all security events

#### Physical Security
- Use tamper detection (optional)
- Implement boot protection
- Secure JTAG interface

### Scaling to Multiple Devices

#### Configuration Management
```bash
# Version 1.0 configuration
config/v1.0/
├── config.h
├── model.tflite
└── parameters.txt

# Version 1.1 configuration
config/v1.1/
├── config.h
├── model_improved.tflite
└── parameters.txt
```

#### Device Provisioning
1. Flash base firmware to all devices
2. Assign unique device ID via serial
3. Load device-specific configuration
4. Verify against manifest

### Long-Term Support

#### Bug Tracking
- Create issues for any failures
- Document workarounds
- Plan fixes for next release

#### Performance Analysis
- Collect performance metrics
- Identify optimization opportunities
- Plan model improvements

#### Documentation Updates
- Update README with lessons learned
- Document any workarounds
- Maintain deployment checklist

### Rollback Procedure

If production deployment has issues:

1. **Immediate Rollback:**
   ```bash
   st-flash erase
   st-flash write previous_version.bin 0x08000000
   ```

2. **Root Cause Analysis:**
   - Review serial logs
   - Check error counters
   - Analyze timing data

3. **Fix & Redeploy:**
   - Address identified issue
   - Test locally first
   - Deploy to subset of devices
   - Monitor carefully before full rollout

### Sign-Off

**Pre-Deployment Sign-Off Checklist:**

- [ ] All tests passing
- [ ] Performance verified
- [ ] Documentation complete
- [ ] Known limitations documented
- [ ] Support process established
- [ ] Monitoring setup complete
- [ ] Rollback procedure ready

**Deployment Date:** ___________
**Deployed By:** ___________
**Verified By:** ___________

---

You're now ready to deploy to production! 🚀
