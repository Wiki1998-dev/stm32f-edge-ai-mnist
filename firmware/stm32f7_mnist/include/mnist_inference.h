#ifndef MNIST_INFERENCE_H
#define MNIST_INFERENCE_H

#include <stdint.h>
#include <stddef.h>

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

/**
 * @brief Initialize MNIST inference engine with tensor arena
 * @param arena Pointer to tensor arena buffer
 * @param arena_size Size of tensor arena (minimum 65536 bytes recommended)
 * @return MNIST_Status
 */
MNIST_Status MNIST_Init(uint8_t *arena, size_t arena_size);

/**
 * @brief Run inference on preprocessed 28x28 image
 * @param input_data Pointer to 28x28 uint8 image data
 * @param output_probs Pointer to 10-element output probability array
 * @return MNIST_Status
 */
MNIST_Status MNIST_Infer(const uint8_t *input_data, uint8_t *output_probs);

/**
 * @brief Get top-1 prediction from output probabilities
 * @param predictions Pointer to 10-element probability array
 * @param confidence Pointer to store confidence (0-255), can be NULL
 * @return Predicted digit (0-9)
 */
uint8_t MNIST_GetTopPrediction(const uint8_t *predictions, uint8_t *confidence);

/**
 * @brief Get MNIST model size
 * @return Model size in bytes
 */
uint32_t MNIST_GetModelSize(void);

/**
 * @brief Get tensor arena usage
 * @return Arena size in bytes
 */
uint32_t MNIST_GetArenaUsage(void);

#endif /* MNIST_INFERENCE_H */
