# Production Deployment Guide

## Pre-Deployment Checklist

### Hardware Verification

- [ ] **Board Functionality**
  - [ ] LEDs blink correctly
  - [ ] Serial communication @ 115200 baud
  - [ ] Clock running at 216 MHz
  - [ ] Memory tests pass

- [ ] **Camera Integration**
  - [ ] OV7670 recognized and initialized
  - [ ] Frames captured without DMA errors
  - [ ] Image quality acceptable
  - [ ] No I2C/SPI conflicts

- [ ] **Performance Targets**
  - [ ] Inference time <20ms
  - [ ] Accuracy >98% on validation set
  - [ ] No memory overflow
  - [ ] Stable operation >1 hour

### Software Validation

```bash
# 1. Run firmware validation
python scripts/validate_board.py --port /dev/ttyUSB0

# Expected output:
# ✓ Firmware version: 1.0.0
# ✓ Model loaded: 45 KB
# ✓ Camera initialized
# ✓ 100 inferences: 98.2% accuracy
# ✓ Memory: 152/320 KB used (47%)
# ✓ Power: 5mA average

# 2. Run stress test (24 hours)
python scripts/stress_test.py --duration 86400 --port /dev/ttyUSB0

# 3. Benchmark performance
python scripts/benchmark.py --board STM32F746 --iterations 1000
```

## Firmware Generation

### Build Release Binary

```bash
cd firmware/stm32f7_mnist
make clean
make -j4 RELEASE=1

# Output: build/stm32f7_mnist_v1.0.0.bin
```

### Sign for Security (Optional)

```bash
# Generate signing key
openssl genrsa -out private_key.pem 2048

# Sign firmware
openssl dgst -sha256 -sign private_key.pem \
    build/stm32f7_mnist_v1.0.0.bin > firmware.sig

# Verify signature
openssl dgst -sha256 -verify public_key.pem \
    -signature firmware.sig build/stm32f7_mnist_v1.0.0.bin
```

## Flashing to Production

### Method 1: ST-Link (Development)

```bash
# Flash via ST-Link
st-flash write build/stm32f7_mnist_v1.0.0.bin 0x08000000

# Verify
st-flash read 0x08000000 100
```

### Method 2: Bootloader (OTA)

```c
// In bootloader (address: 0x08000000)
void bootloader_update(const char *filename) {
    // 1. Load new firmware from external storage
    // 2. Verify signature/checksum
    // 3. Erase application area
    // 4. Write new firmware
    // 5. Set boot flag & reset
}
```

### Method 3: Mass Production

```bash
# Batch script for production
for i in {1..100}; do
    echo "Flashing board $i..."
    st-flash write build/stm32f7_mnist_v1.0.0.bin 0x08000000
    
    # Verify firmware
    st-flash read 0x08000000 100 > /tmp/verify.bin
    if cmp -s /tmp/verify.bin build/stm32f7_mnist_v1.0.0.bin; then
        echo "✓ Board $i OK"
    else
        echo "✗ Board $i FAILED"
    fi
    
    # Wait before next
    sleep 1
done
```

## Monitoring & Logging

### Real-Time Monitoring

```bash
# Monitor inference quality
python scripts/monitor.py --port /dev/ttyUSB0 --log inference.csv

# Log format:
# timestamp,frame,predicted_digit,confidence,inference_time_ms
# 2025-01-19T10:30:45.123,1,5,250,15
# 2025-01-19T10:30:45.138,2,3,248,14
# ...
```

### Metrics Collection

```python
# Example: Collect statistics over 1 hour
import subprocess
import time
from collections import defaultdict

stats = defaultdict(list)

for _ in range(3600):  # 1 hour
    # Read inference time from serial
    result = subprocess.run(
        ['timeout', '1', 'cat', '/dev/ttyUSB0'],
        capture_output=True,
        text=True
    )
    
    # Parse and collect
    for line in result.stdout.split('\n'):
        if 'Time:' in line:
            time_ms = int(line.split()[-2])
            stats['inference_time'].append(time_ms)
    
    time.sleep(1)

# Analyze
import statistics
print(f"Mean: {statistics.mean(stats['inference_time']):.1f} ms")
print(f"Std Dev: {statistics.stdev(stats['inference_time']):.1f} ms")
print(f"Min: {min(stats['inference_time'])} ms")
print(f"Max: {max(stats['inference_time'])} ms")
```

## Maintenance & Updates

### Firmware Updates

**Version Scheme**: `MAJOR.MINOR.PATCH` (e.g., 1.2.3)

```bash
# Tagging releases
git tag -a v1.0.0 -m "Production release"
git push origin v1.0.0

# Create release artifact
gh release create v1.0.0 \
    firmware/stm32f7_mnist/build/stm32f7_mnist_v1.0.0.bin \
    --title "v1.0.0 - MNIST Edge AI" \
    --notes "Production-ready release"
```

### Model Updates

```bash
# Retrain with new data
cd model/training
python train_mnist.py --data-path ./new_dataset

# Convert for deployment
cd ../conversion
python quantize_model.py ../training/mnist_model_quantized.tflite

# Generate C header
python ../../scripts/generate_c_header.py \
    mnist_model_quantized.tflite \
    -o ../../firmware/stm32f7_mnist/models/mnist_model.h

# Rebuild firmware with new model
cd ../../firmware/stm32f7_mnist
make clean && make RELEASE=1
```

## Troubleshooting Production Issues

### Issue: Inference accuracy drops over time

```bash
# Collect confusion matrix every hour
python scripts/accuracy_monitor.py --port /dev/ttyUSB0 --interval 3600

# Possible causes:
# 1. Camera drift (aging sensor) → Recalibrate camera
# 2. Lighting changes → Adjust preprocessing
# 3. Model degradation → Retrain with current data
# 4. Hardware failure → Replace components
```

### Issue: Inference time exceeds threshold

```bash
# Profile individual layers
python scripts/profile_layers.py --model mnist_model_quantized.tflite

# Typical causes:
# 1. Clock throttling → Check power supply
# 2. Temperature throttling → Improve cooling
# 3. Interrupt overload → Reduce background tasks
# 4. Memory stalls → Use CCM for hot data
```

### Issue: Random crashes

```bash
# Enable watchdog to catch crashes
cd firmware/stm32f7_mnist
make ENABLE_WATCHDOG=1

# Monitor for resets
python scripts/crash_monitor.py --port /dev/ttyUSB0

# Causes:
# 1. Stack overflow → Increase stack size
# 2. Null pointer → Add debug assertions
# 3. Memory corruption → Run memory tests
# 4. Power supply → Check PSU voltage
```

## CI/CD Pipeline

### GitHub Actions (Automated Testing)

```yaml
# .github/workflows/deploy.yml
name: Deploy to Production

on:
  push:
    tags: ['v*']

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build firmware
        run: |
          cd firmware/stm32f7_mnist
          make clean && make RELEASE=1
      - name: Run tests
        run: python tests/integration_tests.py
      - name: Create release
        uses: actions/create-release@v1
        with:
          tag_name: ${{ github.ref }}
          files: firmware/stm32f7_mnist/build/*.bin
```

## Compliance & Certification

### Safety Standards

- **EMC**: Check for electromagnetic interference
- **Thermal**: Monitor operating temperature (<60°C)
- **Power**: Verify compliance with specifications

### Documentation

For production deployment, include:
1. **Hardware Schematic** - Design documentation
2. **Firmware Build Report** - Compiler warnings/errors
3. **Test Results** - Validation data
4. **Performance Data** - Timing and accuracy benchmarks
5. **Safety Analysis** - FMEA if applicable

## Support

- **Internal**: [GitHub Issues](https://github.com/Wiki1998-dev/stm32f-edge-ai-mnist/issues)
- **Community**: [STM32 Forums](https://community.st.com/)
- **Vendor**: [ST Support](https://www.st.com/en/support.html)

---

**Last Updated**: January 2025 | **Status**: Production Ready ✅
