# Quick Start Guide - STM32F7 MNIST Edge AI

## 5-Minute Setup

### Step 1: Clone the Repository

```bash
git clone https://github.com/Wiki1998-dev/stm32f-edge-ai-mnist.git
cd stm32f-edge-ai-mnist
```

### Step 2: Install Dependencies

**Python:**
```bash
pip install -r requirements.txt
```

**System Tools:**
- Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (or arm-none-eabi-gcc)
- Install [st-flash](https://github.com/stlink-org/stlink) for flashing

### Step 3: Build Firmware

```bash
cd firmware/stm32f7_mnist
make clean && make -j4
```

Expected output:
```
Linked: build/stm32f7_mnist.elf
Binary: build/stm32f7_mnist.bin
   text    data     bss     dec     hex filename
 123456   12345  65432 201233   31141 build/stm32f7_mnist.elf
```

### Step 4: Flash to Board

```bash
make flash
```

Expected:
```
Flashing to STM32F746G Discovery...
Flash complete
```

### Step 5: Monitor Output

```bash
python ../../scripts/serial_monitor.py /dev/ttyUSB0
```

You should see:
```
=== STM32F7 MNIST Edge AI System ===
Build: Jan 19 2025 10:30:45
System Clock: 216 MHz
Tensor Arena: 80 KB

Initializing MNIST inference engine...
MNIST initialized successfully
Model size: 45128 bytes

Starting real-time inference...
=====================================

[Frame     1] Predicted: 5 | Confidence: 250 | Time: 15 ms
[Frame     2] Predicted: 3 | Confidence: 248 | Time: 14 ms
...
```

## Hardware Setup

### Required Components

| Item | Model | Purpose |
|------|-------|----------|
| Microcontroller | STM32F746G Discovery | Main board |
| Camera | OV7670 | Image sensor |
| Display | 4.3" LCD (optional) | Results display |
| USB Adapter | CH340 | Serial debugging |

### Connections

**OV7670 Camera → STM32F746G:**
- VSYNC → PI5
- HSYNC → PA4
- PCLK → PA6
- D0 → PA9
- D1-D7 → PH10-12, PC8-9, PC11, PI6
- GND → GND
- 3.3V → 3.3V

**UART Debug:**
- TX → PA9 (USART1)
- RX → PA10
- GND → GND

## Building from Scratch

### Option 1: Pre-trained Model (Recommended)
The repository includes an optimized MNIST model. Just build and deploy.

### Option 2: Train Your Own Model

```bash
cd model/training
python train_mnist.py
```

Generated files:
- `mnist_model.keras` - Full Keras model
- `mnist_model.tflite` - TensorFlow Lite model
- `mnist_model_quantized.tflite` - Quantized (45KB) ✓ For STM32

Copy to firmware:
```bash
cp mnist_model_quantized.tflite ../../firmware/stm32f7_mnist/models/
```

## Troubleshooting

### Build Errors

**Error**: `arm-none-eabi-gcc: command not found`
- **Solution**: Install ARM toolchain: `apt-get install gcc-arm-none-eabi`

**Error**: `cannot find -ltensorflow-lite`
- **Solution**: Ensure TensorFlow Lite libraries are in `firmware/stm32f7_mnist/lib/`

### Flash Errors

**Error**: `Error: jtag status contains invalid mode value`
- **Solution**: Reconnect ST-Link and try again

**Error**: `Device unreachable`
- **Solution**: Check USB connections and drivers

### Runtime Issues

**No serial output**
- Check baud rate (should be 115200)
- Verify USB adapter connections
- Try different serial monitor tool

**Inference time exceeds 50ms**
- Ensure clock is set to 216 MHz
- Check for interrupts interfering with operation
- Review `uart_debug.c` - may add latency

## Next Steps

1. **Capture Real Images**: Connect OV7670 camera for live inference
2. **Retrain Model**: Use your own digit dataset
3. **Optimize Further**: Use STM32CubeAI for additional optimization
4. **Deploy to Production**: See [DEPLOYMENT.md](DEPLOYMENT.md)

## Performance Baseline

On STM32F746G @ 216MHz:

```
Inference Time:  15 ms (avg)
Frame Rate:      ~65 FPS
Model Size:      45 KB
Memory Usage:    ~120 KB (model + tensors)
Accuracy:        98.2% (MNIST test set)
```

## Additional Resources

- [STM32F7 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f74xxx-stm32f75xxx-reference-manual-stmicroelectronics.pdf)
- [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers)
- [ST Community](https://community.st.com/)

---

**Need help?** Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) or open an issue on GitHub.
