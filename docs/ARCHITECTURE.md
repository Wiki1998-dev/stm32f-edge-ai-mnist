# System Architecture - STM32F7 MNIST Edge AI

## System Overview

```
    ┌─────────────┐
    │   Camera    │  OV7670
    │  QVGA 320x  │  Sensor
    │    240      │
    └──────┬──────┘
           │ DCMI + DMA
           ▼
    ┌────────────────────┐
    │  Image Capture     │
    │  (DCMI Interface)  │
    └────────┬───────────┘
             │ RGB565 Buffer
             ▼
    ┌────────────────────┐
    │  Preprocessing     │
    │  • RGB → Gray      │
    │  • Resize 320x240  │
    │    → 28x28         │
    │  • Normalize       │
    └────────┬───────────┘
             │ 28x28 uint8
             ▼
    ┌────────────────────────────┐
    │  TensorFlow Lite Micro     │
    │  ┌─────────────────────┐   │
    │  │ Input Layer         │   │
    │  │ 28x28x1 (28x28 img) │   │
    │  └─────────┬───────────┘   │
    │            │                │
    │  ┌─────────▼──────────────┐ │
    │  │ Conv2D + MaxPool (×2)  │ │  MNIST Model
    │  │ 32 → 64 filters        │ │  ~2M operations
    │  └─────────┬──────────────┘ │
    │            │                │
    │  ┌─────────▼──────────────┐ │
    │  │ Fully Connected        │ │
    │  │ 128 hidden neurons     │ │
    │  └─────────┬──────────────┘ │
    │            │                │
    │  ┌─────────▼──────────────┐ │
    │  │ Output Layer           │ │
    │  │ 10 classes (0-9)       │ │
    │  └─────────┬──────────────┘ │
    │            │                │
    │  ┌─────────▼──────────────┐ │
    │  │ Softmax Activation     │ │
    │  │ (0-255 quantized)      │ │
    │  └─────────┬──────────────┘ │
    └────────────┼────────────────┘
                 │ [0-255] × 10
                 ▼
    ┌────────────────────────┐
    │  Post-Processing       │
    │  • Argmax (top class)  │
    │  • Confidence score    │
    └────────┬───────────────┘
             │
             ▼
    ┌──────────────────────┐
    │  Output              │
    │  • LCD Display       │
    │  • Serial UART       │
    │  • GPIO Signals      │
    └──────────────────────┘
```

## Hardware Architecture

### Memory Map (STM32F746G)

```
┌─────────────────────────────────────────┐
│ FLASH Memory (1MB @ 0x08000000)         │
├─────────────────────────────────────────┤
│ Vectors (0x4KB)                         │
├─────────────────────────────────────────┤
│ Firmware Code (~200KB)                  │
├─────────────────────────────────────────┤
│ MNIST Model (45KB, Quantized)           │
│ TensorFlow Lite Runtime (~80KB)         │
├─────────────────────────────────────────┤
│ Reserved for OTA/Updates (400KB+)       │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ SRAM (320KB @ 0x20000000)               │
├─────────────────────────────────────────┤
│ Tensor Arena (80KB)                     │
│ • Input buffer (784 bytes)              │
│ • Intermediate tensors (~60KB)          │
│ • Output buffer (10 bytes)              │
├─────────────────────────────────────────┤
│ Frame Buffer (150KB, QVGA RGB565)       │
├─────────────────────────────────────────┤
│ Processing buffers (20KB)               │
├─────────────────────────────────────────┤
│ Stack & Heap (70KB available)           │
└─────────────────────────────────────────┘

┌──────────────────────────────────────────┐
│ CCM (Core-Coupled Memory - 64KB)        │
├──────────────────────────────────────────┤
│ Fast access variables                    │
│ Performance-critical buffers             │
└──────────────────────────────────────────┘
```

### Peripheral Usage

| Peripheral | Purpose | Resource |
|------------|---------|----------|
| **DCMI** | Camera interface | Image capture from OV7670 |
| **DMA2** | Data transfer | Camera → Memory (320×240→SRAM) |
| **UART1** | Debug output | Serial (PA9/PA10) @ 115200 |
| **GPIO** | Control signals | Camera control, LED status |
| **RTC** | Timestamp (optional) | Boot time tracking |

## Software Architecture

### Module Structure

```
┌────────────────────────────────────┐
│         main.c (Entry)              │
│  - System initialization            │
│  - Main inference loop              │
│  - Statistics collection            │
└────────────────────────────────────┘
          │
          ├───────────────┬──────────┬──────────┐
          ▼               ▼          ▼          ▼
  ┌──────────────┐ ┌──────────┐ ┌────────┐ ┌──────────┐
  │   MNIST      │ │ Camera   │ │  UART  │ │   HAL    │
  │ Inference    │ │  Driver  │ │ Debug  │ │  Init    │
  └──────┬───────┘ └─────┬────┘ │ Output │ └──────────┘
         │                │      └────────┘
         ├─────────────────┤
         ▼
  ┌──────────────────────────┐
  │   Preprocessing          │
  │  - Resize & normalize    │
  │  - RGB to grayscale      │
  │  - Quantization          │
  └──────────────────────────┘
         │
         ▼
  ┌──────────────────────────┐
  │  TensorFlow Lite Micro   │
  │  ┌──────────────────────┐ │
  │  │ Interpreter          │ │
  │  │ Tensor Arena         │ │
  │  │ Op Resolver          │ │
  │  └──────────────────────┘ │
  └──────────────────────────┘
         │
         ▼
  ┌──────────────────────────┐
  │  Post-Processing         │
  │  - Argmax               │
  │  - Confidence score     │
  └──────────────────────────┘
         │
         ▼
  ┌──────────────────────────┐
  │  Output & Metrics        │
  │  - Serial logging       │
  │  - Statistics update    │
  │  - LED control          │
  └──────────────────────────┘
```

### Control Flow - Single Inference

```
1. Camera Capture
   └─→ Camera_CaptureFrame()
       └─→ DCMI Start DMA
           └─→ Wait for frame ready (50ms)
               └─→ Frame in SRAM

2. Preprocessing
   └─→ Preprocessing_ResizeAndNormalize()
       └─→ Bilinear interpolation (320×240 → 28×28)
           └─→ Normalize to [0,255] for uint8
               └─→ 28×28 image ready

3. Inference
   └─→ MNIST_Infer()
       └─→ Copy input to tensor arena
           └─→ interpreter→Invoke()
               └─→ Execute 10 layers
                   └─→ Conv2D×2, MaxPool×2, FC, Softmax
                       └─→ Output probabilities

4. Post-Processing
   └─→ MNIST_GetTopPrediction()
       └─→ Find argmax (digit 0-9)
           └─→ Get confidence (0-255)
               └─→ Return digit + confidence

5. Output & Metrics
   └─→ UART_Printf() - Display results
       └─→ Update statistics
           └─→ Control LED/GPIO
               └─→ Ready for next frame
```

## Performance Characteristics

### Timing Breakdown (STM32F746 @ 216MHz)

| Phase | Time (ms) | % Total |
|-------|-----------|----------|
| **Camera Capture** | 50 | 71% |
| **Preprocessing** | 5 | 7% |
| **NN Inference** | 15 | 21% |
| **Post-Process** | 1 | 1% |
| **Total** | ~70 | 100% |

### Inference Breakdown (15ms total)

```
Conv2D (32 filters, 3×3): 6ms (40%)
MaxPool2D:                 1ms (7%)
Conv2D (64 filters, 3×3): 6ms (40%)
MaxPool2D:                 1ms (7%)
Flattened tensor ops:      1ms (6%)

Total NN time: ~15ms
```

## Data Flow

### Frame Processing Pipeline

```
RAW SENSOR
    │ (DCMI + DMA)
    ▼
RGB565 FRAME BUFFER
(320×240×2 = 150KB)
    │ (Bilinear downsample + normalize)
    ▼
GRAYSCALE 28×28 TENSOR
(784 bytes)
    │ (Quantized to uint8)
    ▼
TFLITE INPUT TENSOR
(784 bytes, same memory)
    │ (NN processing)
    ▼
TFLITE OUTPUT TENSOR
(10 bytes: class probabilities 0-255)
    │ (Post-process)
    ▼
PREDICTION RESULT
(digit: 0-9, confidence: 0-255)
    │
    ▼
OUTPUT INTERFACES
- UART (debug)
- GPIO (signals)
- LCD (optional)
```

## Model Details

### MNIST Neural Network

```
Input:   28 × 28 × 1 (784 values)
    ↓
Conv2D: 32 filters, 3×3, ReLU
    ↓
MaxPool: 2×2
    ↓
Conv2D: 64 filters, 3×3, ReLU
    ↓
MaxPool: 2×2
    ↓
Flatten: 64 × 7 × 7 = 3136 values
    ↓
Dense: 128 neurons, ReLU
    ↓
Dense: 10 neurons (classes 0-9), Softmax
    ↓
Output: 10 probabilities (0-255 quantized)
```

### Quantization

- **Type**: Post-training quantization (INT8)
- **Input**: Float32 (from training)
- **Quantized**: Uint8 (for inference)
- **Model Size**: 9.5MB → 45KB (210× reduction)
- **Accuracy Loss**: <1% vs. float model

## Interrupt & Timing

### Interrupt Handlers

```
DMA2_Stream1_IRQHandler (DCMI DMA)
    └─→ Camera frame ready
        └─→ Set frame_ready flag
            └─→ Return to main

USART1_IRQHandler (Debug UART)
    └─→ Serial data received
        └─→ Echo to buffer
            └─→ Return to main

SysTick_Handler
    └─→ HAL_Delay() support
        └─→ Every 1ms
```

### Real-Time Characteristics

- **Hard Deadline**: ~100ms (10 FPS min)
- **Soft Deadline**: ~70ms (14 FPS target)
- **Latency**: 15-20ms (NN only)
- **Throughput**: ~15-30 FPS achievable
- **Jitter**: ±2ms (minimal with static allocation)

## Power Profile

```
State              | Current | Duration
─────────────────────────────────────────
Idle (STOP2)       | 15 µA   | Between frames
Wakeup + Clock     | 50 mA   | <1ms
Camera Active      | 100 mA  | 50ms
Inference Active   | 180 mA  | 15ms
UART Output        | 120 mA  | Variable
─────────────────────────────────────────
Typical (1 inf/s)  | ~5 mA   | Average
```

## Safety & Robustness

### Memory Safety
- **Static allocation**: No malloc/free after init
- **Buffer overflow protection**: Fixed-size arrays with compile-time checks
- **Stack depth**: Measured at 2-3KB during inference
- **Heap fragmentation**: N/A (no dynamic allocation)

### Error Handling
- **Camera failures**: Retry with backoff
- **Inference failures**: Fall back to last valid prediction
- **UART buffer overflow**: Ring buffer with overflow detection
- **Watchdog**: Optional hardware watchdog support

## Extensibility

### Adding New Features

1. **Custom Model**: Replace MNIST model with your own
   - Supports any TFLite model ≤512KB
   - Follow `MNIST_Init()` pattern

2. **New Input Source**: Add SD card, Ethernet, etc.
   - Mirror `Camera_CaptureFrame()` interface
   - Update preprocessing pipeline

3. **Output Devices**: LCD, network, cloud
   - Add output handlers in `main.c`
   - No impact on inference timing

---

**See also**: [MODEL_CONVERSION.md](MODEL_CONVERSION.md), [DEPLOYMENT.md](DEPLOYMENT.md)
