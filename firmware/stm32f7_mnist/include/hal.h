#ifndef HAL_H
#define HAL_H

#include "stm32f7xx_hal.h"

/**
 * @brief Initialize hardware (clock, GPIO, peripherals)
 */
void System_Init(void);

/**
 * @brief Configure system clock to 216 MHz
 */
void SystemClock_Config(void);

/**
 * @brief Error handler for critical failures
 */
void Error_Handler(void);

/**
 * @brief Assertion handler for debug builds
 */
void assert_failed(uint8_t *file, uint32_t line);

#endif /* HAL_H */
