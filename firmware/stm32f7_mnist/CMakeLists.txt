cmake_minimum_required(VERSION 3.15)
project(stm32f7_mnist_edgeai C CXX ASM)

# Set compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")

# Target specification
set(CPU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_FLAGS}")

# Include directories
include_directories(
    include
    lib/tensorflow_lite
    lib/cmsis_nn
    lib/stm32cubef7/Drivers/STM32F7xx_HAL_Driver/Inc
    lib/stm32cubef7/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    lib/stm32cubef7/Drivers/CMSIS/Include
)

# Source files
set(SOURCES
    src/main.c
    src/mnist_inference.c
    src/preprocessing.c
    src/camera_driver.c
    src/uart_debug.c
    src/hal_init.c
)

# Create executable
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}.elf
    -L${CMAKE_SOURCE_DIR}/lib/tensorflow_lite
    -L${CMAKE_SOURCE_DIR}/lib/cmsis_nn
    -ltensorflow-lite
    -lcmsis-nn
    -lm
)

# Generate binary
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND arm-none-eabi-size ${PROJECT_NAME}.elf
)
