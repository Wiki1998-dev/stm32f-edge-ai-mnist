#ifndef CONFIG_H
#define CONFIG_H

/* Board Configuration */
#define BOARD_STM32F746G_DISCOVERY 1

/* Memory Configuration */
#define TENSOR_ARENA_SIZE (80 * 1024)   /* 80 KB for inference tensors */
#define FRAME_BUFFER_SIZE (320 * 240)   /* QVGA resolution */

/* Camera Configuration */
#define CAMERA_WIDTH  320
#define CAMERA_HEIGHT 240
#define MODEL_INPUT_SIZE 28

/* MNIST Model Configuration */
#define MNIST_NUM_CLASSES 10
#define MNIST_INFERENCE_TIME_MS 15      /* Typical inference time */

/* Debug Configuration */
#define DEBUG_UART_BAUD 115200
#define DEBUG_ENABLE 1

/* Performance Configuration */
#define FPS_TARGET 30
#define INFERENCE_TIMEOUT_MS 1000

#endif /* CONFIG_H */
