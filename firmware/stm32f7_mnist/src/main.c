#include "hal.h"
#include "uart_debug.h"
#include "mnist_inference.h"
#include "camera_driver.h"
#include "preprocessing.h"
#include <string.h>
#include <stdio.h>

/* Tensor Arena for model - statically allocated */
#define TENSOR_ARENA_SIZE (80 * 1024)  /* 80KB for intermediate tensors */
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];

/* Input buffer for processed image */
static uint8_t image_buffer[28 * 28];  /* 28x28 grayscale image */

/* Output predictions */
static uint8_t predictions[10];

/* Performance metrics */
typedef struct {
    uint32_t total_inferences;
    uint32_t correct_predictions;
    uint32_t avg_inference_time_ms;
    uint32_t min_inference_time_ms;
    uint32_t max_inference_time_ms;
} PerformanceMetrics;

static PerformanceMetrics metrics = {0};

/**
 * @brief Initialize system peripherals
 */
static void System_Init(void) {
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize debug UART */
    UART_Init();
    
    /* Initialize camera */
    Camera_Init();
    
    UART_Printf("\n=== STM32F7 MNIST Edge AI System ===\r\n");
    UART_Printf("Build: %s %s\r\n", __DATE__, __TIME__);
    UART_Printf("System Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    UART_Printf("Tensor Arena: %d KB\r\n", TENSOR_ARENA_SIZE / 1024);
    UART_Printf("\r\n");
}

/**
 * @brief Main application entry point
 */
int main(void) {
    uint32_t inference_start_time;
    uint32_t inference_time_ms;
    uint8_t predicted_digit;
    uint8_t confidence;
    int i;
    
    /* System initialization */
    System_Init();
    
    /* Initialize MNIST inference engine */
    UART_Printf("Initializing MNIST inference engine...\r\n");
    if (MNIST_Init(tensor_arena, TENSOR_ARENA_SIZE) != MNIST_OK) {
        UART_Printf("ERROR: Failed to initialize MNIST\r\n");
        return -1;
    }
    UART_Printf("MNIST initialized successfully\r\n");
    UART_Printf("Model size: %d bytes\r\n", MNIST_GetModelSize());
    UART_Printf("\r\n");
    
    /* Main inference loop */
    UART_Printf("Starting real-time inference...\r\n");
    UART_Printf("=====================================\r\n\r\n");
    
    uint32_t frame_count = 0;
    
    while (1) {
        frame_count++;
        
        /* Capture image from camera */
        if (Camera_CaptureFrame() != CAMERA_OK) {
            UART_Printf("[Frame %lu] Camera capture failed\r\n", frame_count);
            continue;
        }
        
        /* Preprocess: resize and normalize to 28x28 */
        if (Preprocessing_ResizeAndNormalize(
            Camera_GetFrameBuffer(),
            320, 240,              /* Input: QVGA */
            image_buffer,
            28, 28,                /* Output: 28x28 */
            PREPROCESSING_UINT8) != PREPROC_OK) {
            UART_Printf("[Frame %lu] Preprocessing failed\r\n", frame_count);
            continue;
        }
        
        /* Run inference */
        inference_start_time = HAL_GetTick();
        
        if (MNIST_Infer(image_buffer, predictions) != MNIST_OK) {
            UART_Printf("[Frame %lu] Inference failed\r\n", frame_count);
            continue;
        }
        
        inference_time_ms = HAL_GetTick() - inference_start_time;
        
        /* Get top-1 prediction */
        predicted_digit = MNIST_GetTopPrediction(predictions, &confidence);
        
        /* Update metrics */
        metrics.total_inferences++;
        if (metrics.avg_inference_time_ms == 0) {
            metrics.min_inference_time_ms = inference_time_ms;
            metrics.max_inference_time_ms = inference_time_ms;
        } else {
            metrics.avg_inference_time_ms = 
                (metrics.avg_inference_time_ms + inference_time_ms) / 2;
            if (inference_time_ms < metrics.min_inference_time_ms)
                metrics.min_inference_time_ms = inference_time_ms;
            if (inference_time_ms > metrics.max_inference_time_ms)
                metrics.max_inference_time_ms = inference_time_ms;
        }
        
        /* Output results */
        UART_Printf("[Frame %5lu] Predicted: %d | Confidence: %3d%% | "
                    "Time: %3lu ms\r\n",
                    frame_count,
                    predicted_digit,
                    confidence,
                    inference_time_ms);
        
        /* Print per-class probabilities every 10 frames */
        if (frame_count % 10 == 0) {
            UART_Printf("  Probabilities: ");
            for (i = 0; i < 10; i++) {
                UART_Printf("%u:%3d%% ", i, predictions[i]);
            }
            UART_Printf("\r\n");
            
            /* Print statistics every 100 frames */
            if (frame_count % 100 == 0) {
                UART_Printf("\r\n--- Statistics (100 frames) ---\r\n");
                UART_Printf("Total inferences: %lu\r\n", metrics.total_inferences);
                UART_Printf("Avg time: %lu ms\r\n", metrics.avg_inference_time_ms);
                UART_Printf("Min time: %lu ms\r\n", metrics.min_inference_time_ms);
                UART_Printf("Max time: %lu ms\r\n", metrics.max_inference_time_ms);
                UART_Printf("Frame rate: %.1f FPS\r\n", 
                           1000.0f / metrics.avg_inference_time_ms);
                UART_Printf("---\r\n\r\n");
            }
        }
        
        /* Small delay to prevent overwhelming UART */
        HAL_Delay(10);
    }
    
    return 0;
}

/**
 * @brief System error handler (called by HAL on critical errors)
 */
void Error_Handler(void) {
    UART_Printf("FATAL ERROR: System halted\r\n");
    while (1) {
        HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);  /* Toggle LED for visibility */
        HAL_Delay(500);
    }
}

/**
 * @brief Assertion handler for debug builds
 */
void assert_failed(uint8_t *file, uint32_t line) {
    UART_Printf("Assert failed: %s:%lu\r\n", file, line);
    Error_Handler();
}
