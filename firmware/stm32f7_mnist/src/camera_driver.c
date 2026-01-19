#include "camera_driver.h"
#include "stm32f7xx_hal.h"
#include <string.h>

/* Camera frame buffer (QVGA: 320x240 RGB565) */
static uint16_t camera_frame_buffer[320 * 240];

/* DCMI configuration */
static DCMI_HandleTypeDef hdcmi;
static DMA_HandleTypeDef hdma_dcmi;

/**
 * @brief Initialize camera interface (DCMI + DMA)
 */
Camera_Status Camera_Init(void) {
    /* Enable clocks */
    __HAL_RCC_DCMI_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    /* Configure GPIO pins for camera interface */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIOA, GPIOB, GPIOC, GPIOG, GPIOH, GPIOI clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    
    /* DCMI GPIO Configuration */
    /* D0 (PA9), D1 (PH10), D2 (PH11), D3 (PH12) */
    /* D4 (PC8), D5 (PC9), D6 (PC11), D7 (PI6) */
    /* HSYNC (PA4), VSYNC (PI5), PCLK (PA6) */
    
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_4 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    
    /* DCMI DMA Configuration */
    hdma_dcmi.Instance = DMA2_Stream1;
    hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_NORMAL;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_dcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_dcmi.Init.MemBurst = DMA_MBURST_INC4;
    hdma_dcmi.Init.PeriphBurst = DMA_PBURST_INC4;
    
    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
        return CAMERA_ERROR_DMA_INIT;
    }
    
    /* DCMI Configuration */
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAMES;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
        return CAMERA_ERROR_DCMI_INIT;
    }
    
    /* Link DMA to DCMI */
    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi);
    
    return CAMERA_OK;
}

/**
 * @brief Capture frame from camera
 */
Camera_Status Camera_CaptureFrame(void) {
    /* Start DMA transfer */
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS,
                          (uint32_t)camera_frame_buffer,
                          (320 * 240 * 2) / 4) != HAL_OK) {
        return CAMERA_ERROR_CAPTURE;
    }
    
    /* Wait for frame ready */
    HAL_Delay(50);
    HAL_DCMI_Stop(&hdcmi);
    
    return CAMERA_OK;
}

/**
 * @brief Get pointer to frame buffer
 */
uint8_t *Camera_GetFrameBuffer(void) {
    return (uint8_t *)camera_frame_buffer;
}
