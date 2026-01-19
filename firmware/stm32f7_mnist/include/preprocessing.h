#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <stdint.h>

typedef enum {
    PREPROC_OK = 0,
    PREPROC_ERROR_INVALID_PARAMS,
    PREPROC_ERROR_UNSUPPORTED_SIZE,
} Preprocessing_Status;

typedef enum {
    PREPROCESSING_UINT8,
    PREPROCESSING_FLOAT32,
} Preprocessing_Format;

/**
 * @brief Resize and normalize image using bilinear interpolation
 * Designed for downsampling camera frames (e.g., 320x240) to 28x28
 * @param input_image Input image buffer (grayscale)
 * @param input_width Input image width
 * @param input_height Input image height
 * @param output_image Output image buffer
 * @param output_width Output image width (should be 28 for MNIST)
 * @param output_height Output image height (should be 28 for MNIST)
 * @param format Output format (uint8 or float32)
 * @return Preprocessing_Status
 */
Preprocessing_Status Preprocessing_ResizeAndNormalize(
    const uint8_t *input_image,
    uint16_t input_width,
    uint16_t input_height,
    uint8_t *output_image,
    uint16_t output_width,
    uint16_t output_height,
    Preprocessing_Format format);

/**
 * @brief Convert RGB to Grayscale
 * @param input_rgb Input RGB buffer (3 bytes per pixel)
 * @param num_pixels Number of pixels
 * @param output_gray Output grayscale buffer
 * @return Preprocessing_Status
 */
Preprocessing_Status Preprocessing_RGBToGrayscale(
    const uint8_t *input_rgb,
    uint32_t num_pixels,
    uint8_t *output_gray);

#endif /* PREPROCESSING_H */
