# STM32F7 MNIST Edge AI Makefile

PROJECT_NAME = stm32f7_mnist
BUILD_DIR = build
FW_DIR = .

# Tools
ARM_GCC_PREFIX = arm-none-eabi
CC = $(ARM_GCC_PREFIX)-gcc
OBJCOPY = $(ARM_GCC_PREFIX)-objcopy
OBJDUMP = $(ARM_GCC_PREFIX)-objdump
SIZE = $(ARM_GCC_PREFIX)-size

# Target
TARGET = $(BUILD_DIR)/$(PROJECT_NAME)

# Compiler flags
CFLAGS = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
CFLAGS += -Wall -Wextra -O2 -g3
CFLAGS += -DSTM32F746xx -DUSE_HAL_DRIVER
CFLAGS += -I$(FW_DIR)/include
CFLAGS += -I$(FW_DIR)/lib/tensorflow_lite
CFLAGS += -I$(FW_DIR)/lib/cmsis_nn
CFLAGS += -I$(FW_DIR)/lib/stm32cubef7/Drivers/STM32F7xx_HAL_Driver/Inc
CFLAGS += -I$(FW_DIR)/lib/stm32cubef7/Drivers/CMSIS/Device/ST/STM32F7xx/Include
CFLAGS += -I$(FW_DIR)/lib/stm32cubef7/Drivers/CMSIS/Include

# Source files
SOURCES = $(FW_DIR)/src/main.c
SOURCES += $(FW_DIR)/src/mnist_inference.c
SOURCES += $(FW_DIR)/src/preprocessing.c
SOURCES += $(FW_DIR)/src/camera_driver.c
SOURCES += $(FW_DIR)/src/uart_debug.c
SOURCES += $(FW_DIR)/src/hal_init.c

# Object files
OBJECTS = $(patsubst $(FW_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Linker script
LDSCRIPT = $(FW_DIR)/linker/STM32F746NGHx_FLASH.ld
LDFLAGS = -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS += -L$(FW_DIR)/lib/tensorflow_lite -ltensorflow-lite
LDFLAGS += -L$(FW_DIR)/lib/cmsis_nn -lcmsis-nn
LDFLAGS += -lm

# Targets
.PHONY: all clean flash test help

all: $(TARGET).elf $(TARGET).bin

$(BUILD_DIR)/%.o: $(FW_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled: $<"

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SIZE) $@
	@echo "Linked: $@"

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned"

flash: $(TARGET).bin
	@echo "Flashing to STM32F746G Discovery..."
	st-flash --reset write $(TARGET).bin 0x08000000
	@echo "Flash complete"

test: $(TARGET).elf
	@echo "Running tests..."
	# Add test commands here

help:
	@echo "Available targets:"
	@echo "  all   - Build firmware (default)"
	@echo "  clean - Remove build artifacts"
	@echo "  flash - Build and flash to board"
	@echo "  test  - Run tests"
	@echo "  help  - Show this help message"
