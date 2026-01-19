/**
 * @file camera_driver.c
 * @brief OV7670 Camera Driver Implementation
 * @author Wiki1998
 * @date January 19, 2025
 *
 * DCMI (Digital Camera Memory Interface) driver for OV7670 camera module
 * with DMA support for efficient frame capture.
 */

#include "camera_driver.h"
#include "stm32f7xx_hal.h"
#include "config.h"

/* DMA and DCMI handles */
static DCMI_HandleTypeDef hdcmi;
static DMA_HandleTypeDef hdma_dcmi;

/**
 * @brief Initialize camera interface (DCMI + DMA)
 */
camera_status_t camera_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_DCMI_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    
    /* Configure GPIO pins for DCMI */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure DCMI */
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
    
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
        return CAMERA_ERROR;
    }
    
    return CAMERA_OK;
}

/**
 * @brief Capture single frame from camera
 */
camera_status_t camera_capture_frame(uint8_t *frame_buffer) {
    if (!frame_buffer) {
        return CAMERA_INVALID_PARAM;
    }
    
    /* Configure DMA for frame capture */
    hdma_dcmi.Instance = DMA2_Stream1;
    hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_dcmi.Init.Mode = DMA_NORMAL;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    
    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
        return CAMERA_ERROR;
    }
    
    /* Start capture */
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_CAPTURE_MODE_CONTINUOUS, 
                           (uint32_t)frame_buffer, FRAME_SIZE / 4) != HAL_OK) {
        return CAMERA_ERROR;
    }
    
    /* Wait for capture complete */
    HAL_Delay(50);
    
    return CAMERA_OK;
}

/**
 * @brief Read camera status
 */
camera_status_t camera_get_status(void) {
    return (hdcmi.State == HAL_DCMI_STATE_READY) ? CAMERA_OK : CAMERA_ERROR;
}

/**
 * @brief Deinitialize camera interface
 */
camera_status_t camera_deinit(void) {
    HAL_DCMI_Stop(&hdcmi);
    HAL_DCMI_DeInit(&hdcmi);
    return CAMERA_OK;
}
