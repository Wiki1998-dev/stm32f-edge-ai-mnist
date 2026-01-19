#ifndef CAMERA_DRIVER_H
#define CAMERA_DRIVER_H

#include <stdint.h>

typedef enum {
    CAMERA_OK = 0,
    CAMERA_ERROR_DMA_INIT,
    CAMERA_ERROR_DCMI_INIT,
    CAMERA_ERROR_CAPTURE,
} Camera_Status;

/**
 * @brief Initialize camera interface (DCMI + DMA)
 * @return Camera_Status
 */
Camera_Status Camera_Init(void);

/**
 * @brief Capture frame from camera
 * @return Camera_Status
 */
Camera_Status Camera_CaptureFrame(void);

/**
 * @brief Get pointer to frame buffer (QVGA: 320x240 RGB565)
 * @return Pointer to frame buffer
 */
uint8_t *Camera_GetFrameBuffer(void);

#endif /* CAMERA_DRIVER_H */
