# API Reference - STM32F7 MNIST Inference

## Overview

This document provides detailed API documentation for the STM32F7 MNIST edge AI system.

## Core Functions

### MNIST Inference Engine

#### `MNIST_Init()`

**Prototype**:
```c
MNIST_Status MNIST_Init(uint8_t *arena, size_t arena_size);
```

**Description**: Initialize MNIST inference engine with tensor arena.

**Parameters**:
- `arena` - Pointer to tensor arena buffer (must be at least 65536 bytes)
- `arena_size` - Size of tensor arena in bytes

**Returns**:
- `MNIST_OK` - Initialization successful
- `MNIST_ERROR_INVALID_ARENA` - Invalid arena pointer or size
- `MNIST_ERROR_INVALID_MODEL` - Model version mismatch
- `MNIST_ERROR_MEMORY` - Memory allocation failed
- `MNIST_ERROR_ALLOCATE_TENSORS` - Tensor allocation failed

**Example**:
```c
uint8_t tensor_arena[80 * 1024];
MNIST_Status status = MNIST_Init(tensor_arena, sizeof(tensor_arena));
if (status != MNIST_OK) {
    printf("Init failed: %d\n", status);
    return;
}
```

---

#### `MNIST_Infer()`

**Prototype**:
```c
MNIST_Status MNIST_Infer(const uint8_t *input_data, uint8_t *output_probs);
```

**Description**: Run inference on preprocessed 28×28 image.

**Parameters**:
- `input_data` - Pointer to 28×28 uint8 image (784 bytes total)
- `output_probs` - Pointer to output buffer (10 bytes minimum)

**Returns**:
- `MNIST_OK` - Inference successful
- `MNIST_ERROR_INVALID_PARAMS` - NULL pointer or invalid input
- `MNIST_ERROR_INVOKE_FAILED` - TensorFlow Lite invocation failed
- `MNIST_ERROR_TENSOR_NOT_FOUND` - Input/output tensor not found

**Output Format**:
The `output_probs` buffer contains 10 uint8 values (0-255) representing the quantized probability of each digit (0-9).

**Example**:
```c
uint8_t image[28 * 28];  // Preprocessed image
uint8_t predictions[10];  // Output probabilities

MNIST_Status status = MNIST_Infer(image, predictions);
if (status == MNIST_OK) {
    printf("Digit 0: %d\n", predictions[0]);
    printf("Digit 1: %d\n", predictions[1]);
    // ... etc
}
```

---

#### `MNIST_GetTopPrediction()`

**Prototype**:
```c
uint8_t MNIST_GetTopPrediction(const uint8_t *predictions, uint8_t *confidence);
```

**Description**: Extract top-1 prediction from output probabilities.

**Parameters**:
- `predictions` - Pointer to 10-element prediction array
- `confidence` - Pointer to confidence value (0-255), can be NULL

**Returns**:
- Predicted digit (0-9)

**Example**:
```c
uint8_t digit = MNIST_GetTopPrediction(predictions, NULL);
printf("Predicted digit: %d\n", digit);

uint8_t confidence;
digit = MNIST_GetTopPrediction(predictions, &confidence);
printf("Predicted: %d with confidence %d%%\n", digit, confidence);
```

---

#### `MNIST_GetModelSize()`

**Prototype**:
```c
uint32_t MNIST_GetModelSize(void);
```

**Description**: Get MNIST model size in bytes.

**Returns**: Model size (typically 45128 bytes for quantized model)

**Example**:
```c
printf("Model size: %lu bytes\n", MNIST_GetModelSize());
```

---

## Image Preprocessing

#### `Preprocessing_ResizeAndNormalize()`

**Prototype**:
```c
Preprocessing_Status Preprocessing_ResizeAndNormalize(
    const uint8_t *input_image,
    uint16_t input_width,
    uint16_t input_height,
    uint8_t *output_image,
    uint16_t output_width,
    uint16_t output_height,
    Preprocessing_Format format
);
```

**Description**: Resize and normalize image using bilinear interpolation.

**Parameters**:
- `input_image` - Input grayscale image buffer
- `input_width`, `input_height` - Input image dimensions
- `output_image` - Output image buffer
- `output_width`, `output_height` - Output dimensions (should be 28×28 for MNIST)
- `format` - Output format (PREPROCESSING_UINT8 or PREPROCESSING_FLOAT32)

**Returns**:
- `PREPROC_OK` - Success
- `PREPROC_ERROR_INVALID_PARAMS` - NULL pointer
- `PREPROC_ERROR_UNSUPPORTED_SIZE` - Invalid output size

**Example**:
```c
// Resize camera frame (320×240) to MNIST input (28×28)
uint8_t camera_frame[320 * 240];
uint8_t mnist_input[28 * 28];

Preprocessing_status status = Preprocessing_ResizeAndNormalize(
    camera_frame,
    320, 240,
    mnist_input,
    28, 28,
    PREPROCESSING_UINT8
);
```

---

## Camera Interface

#### `Camera_Init()`

**Prototype**:
```c
Camera_status Camera_Init(void);
```

**Description**: Initialize camera interface (DCMI + DMA).

**Returns**:
- `CAMERA_OK` - Initialization successful
- `CAMERA_ERROR_DMA_INIT` - DMA initialization failed
- `CAMERA_ERROR_DCMI_INIT` - DCMI initialization failed

---

#### `Camera_CaptureFrame()`

**Prototype**:
```c
Camera_status Camera_CaptureFrame(void);
```

**Description**: Capture frame from camera.

**Returns**:
- `CAMERA_OK` - Frame captured successfully
- `CAMERA_ERROR_CAPTURE` - Capture failed

---

#### `Camera_GetFrameBuffer()`

**Prototype**:
```c
uint8_t *Camera_GetFrameBuffer(void);
```

**Description**: Get pointer to frame buffer.

**Returns**: Pointer to QVGA frame buffer (320×240 RGB565)

---

## Debug Output

#### `UART_Init()`

**Prototype**:
```c
void UART_Init(void);
```

**Description**: Initialize UART1 for debugging at 115200 baud.

---

#### `UART_Printf()`

**Prototype**:
```c
int UART_Printf(const char *format, ...);
```

**Description**: Printf-style formatted output over UART.

**Parameters**:
- `format` - Printf-style format string
- `...` - Variable arguments

**Returns**: Number of characters sent

**Example**:
```c
UART_Printf("Predicted digit: %d with confidence %d%%\r\n", digit, confidence);
```

---

## Data Types & Enums

### Status Enums

```c
typedef enum {
    MNIST_OK = 0,
    MNIST_ERROR_INVALID_PARAMS,
    MNIST_ERROR_INVALID_ARENA,
    MNIST_ERROR_INVALID_MODEL,
    MNIST_ERROR_MEMORY,
    MNIST_ERROR_ALLOCATE_TENSORS,
    MNIST_ERROR_TENSOR_NOT_FOUND,
    MNIST_ERROR_INVALID_INPUT_SHAPE,
    MNIST_ERROR_INVALID_OUTPUT_SHAPE,
    MNIST_ERROR_UNSUPPORTED_TYPE,
    MNIST_ERROR_INVOKE_FAILED,
} MNIST_Status;
```

### Preprocessing Formats

```c
typedef enum {
    PREPROCESSING_UINT8,
    PREPROCESSING_FLOAT32,
} Preprocessing_Format;
```

---

## Complete Example

```c
#include "mnist_inference.h"
#include "preprocessing.h"
#include "camera_driver.h"
#include "uart_debug.h"

int main(void) {
    uint8_t tensor_arena[80 * 1024];
    uint8_t camera_frame[320 * 240];
    uint8_t mnist_input[28 * 28];
    uint8_t predictions[10];
    
    // Initialize subsystems
    UART_Init();
    Camera_Init();
    MNIST_Init(tensor_arena, sizeof(tensor_arena));
    
    UART_Printf("MNIST Inference System Ready\r\n");
    
    while (1) {
        // Capture frame
        if (Camera_CaptureFrame() != CAMERA_OK) {
            UART_Printf("Camera error\r\n");
            continue;
        }
        
        // Preprocess
        if (Preprocessing_ResizeAndNormalize(
            Camera_GetFrameBuffer(),
            320, 240,
            mnist_input,
            28, 28,
            PREPROCESSING_UINT8) != PREPROC_OK) {
            UART_Printf("Preprocessing error\r\n");
            continue;
        }
        
        // Run inference
        if (MNIST_Infer(mnist_input, predictions) != MNIST_OK) {
            UART_Printf("Inference error\r\n");
            continue;
        }
        
        // Get result
        uint8_t digit = MNIST_GetTopPrediction(predictions, NULL);
        
        // Output
        UART_Printf("Predicted: %d\r\n", digit);
        
        // Small delay
        HAL_Delay(50);
    }
    
    return 0;
}
```

---

**See also**: [ARCHITECTURE.md](ARCHITECTURE.md), [QUICKSTART.md](QUICKSTART.md)
