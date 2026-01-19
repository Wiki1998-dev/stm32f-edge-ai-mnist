#include "preprocessing.h"
#include <string.h>
#include <stdint.h>

/**
 * @brief Simple bilinear interpolation for image resizing
 * Downsample 320x240 QVGA to 28x28 for MNIST model input
 */
Preprocessing_Status Preprocessing_ResizeAndNormalize(
    const uint8_t *input_image,
    uint16_t input_width,
    uint16_t input_height,
    uint8_t *output_image,
    uint16_t output_width,
    uint16_t output_height,
    Preprocessing_Format format) {
    
    if (input_image == nullptr || output_image == nullptr) {
        return PREPROC_ERROR_INVALID_PARAMS;
    }
    
    if (output_width != 28 || output_height != 28) {
        return PREPROC_ERROR_UNSUPPORTED_SIZE;
    }
    
    uint32_t x_ratio = (input_width << 16) / output_width;
    uint32_t y_ratio = (input_height << 16) / output_height;
    
    uint32_t i, j;
    uint32_t x, y;
    uint32_t x_p, y_p;
    uint32_t a, b, c, d;
    uint32_t gray;
    
    for (i = 0; i < output_height; i++) {
        for (j = 0; j < output_width; j++) {
            x = (j * x_ratio) >> 16;
            y = (i * y_ratio) >> 16;
            x_p = x;
            y_p = y;
            
            /* Bilinear interpolation */
            a = input_image[(y_p * input_width) + x_p];
            b = (x_p + 1 < input_width) ? 
                input_image[(y_p * input_width) + x_p + 1] : a;
            c = (y_p + 1 < input_height) ? 
                input_image[((y_p + 1) * input_width) + x_p] : a;
            d = (x_p + 1 < input_width && y_p + 1 < input_height) ? 
                input_image[((y_p + 1) * input_width) + x_p + 1] : a;
            
            gray = (a + b + c + d) >> 2;  /* Average of 4 neighbors */
            
            /* Normalize to 0-255 range for uint8 quantized model */
            if (format == PREPROCESSING_UINT8) {
                output_image[(i * output_width) + j] = (uint8_t)(gray & 0xFF);
            }
        }
    }
    
    return PREPROC_OK;
}

/**
 * @brief Convert RGB to Grayscale (if needed)
 */
Preprocessing_Status Preprocessing_RGBToGrayscale(
    const uint8_t *input_rgb,
    uint32_t num_pixels,
    uint8_t *output_gray) {
    
    if (input_rgb == nullptr || output_gray == nullptr) {
        return PREPROC_ERROR_INVALID_PARAMS;
    }
    
    uint32_t i;
    uint8_t r, g, b;
    
    for (i = 0; i < num_pixels; i++) {
        r = input_rgb[i * 3 + 0];
        g = input_rgb[i * 3 + 1];
        b = input_rgb[i * 3 + 2];
        
        /* Standard luminance formula */
        output_gray[i] = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
    }
    
    return PREPROC_OK;
}
