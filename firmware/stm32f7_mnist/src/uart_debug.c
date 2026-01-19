#include "uart_debug.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define UART_TX_BUFFER_SIZE 256

static UART_HandleTypeDef huart1;
static char tx_buffer[UART_TX_BUFFER_SIZE];

/**
 * @brief Initialize UART1 (115200 baud, 8N1)
 */
void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure GPIO pins (PA9=TX, PA10=RX) */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* UART configuration */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        while (1);
    }
}

/**
 * @brief Printf-style debug output
 */
int UART_Printf(const char *format, ...) {
    va_list args;
    int len;
    
    va_start(args, format);
    len = vsnprintf(tx_buffer, UART_TX_BUFFER_SIZE, format, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t *)tx_buffer, len, HAL_MAX_DELAY);
    }
    
    return len;
}

/**
 * @brief Send raw data over UART
 */
void UART_SendData(const uint8_t *data, uint16_t length) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, length, HAL_MAX_DELAY);
}
