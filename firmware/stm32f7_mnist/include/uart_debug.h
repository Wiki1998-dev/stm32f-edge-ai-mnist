#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include <stdint.h>

/**
 * @brief Initialize UART1 for debugging (115200 baud)
 */
void UART_Init(void);

/**
 * @brief Printf-style formatted output over UART
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return Number of characters sent
 */
int UART_Printf(const char *format, ...);

/**
 * @brief Send raw data over UART
 * @param data Pointer to data buffer
 * @param length Number of bytes to send
 */
void UART_SendData(const uint8_t *data, uint16_t length);

#endif /* UART_DEBUG_H */
