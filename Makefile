# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# MCU and compiler flags
MCU = cortex-m4
CFLAGS = -mcpu=$(MCU) -mthumb -O0 -g3 -Wall
CFLAGS += $(addprefix -I,$(INC_DIRS))
CFLAGS += -DSTM32F401xE

# Linker flags
LDFLAGS = -mcpu=$(MCU) -mthumb
LDFLAGS += -specs=nosys.specs
LDFLAGS += -T helpers/ST/src/STM32F401RETX_FLASH.ld
LDFLAGS += -Wl,-Map=build/output.map
LDFLAGS += -Wl,--gc-sections

# Build directory
BUILD_DIR = build

# Source directories (where .c and .s files are)
SRC_DIRS = . drivers helpers/ST/src
ASM_DIRS = helpers/ST/src

# Include directories (where .h files are)
INC_DIRS = drivers helpers/CMSIS/Include helpers/ST/include

# Find all source files automatically
C_SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
ASM_SOURCES = $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

# Target
TARGET = $(BUILD_DIR)/gpio_blink

# Default target: build everything
all: $(BUILD_DIR) $(TARGET).elf
	@echo "Build complete!"
	$(SIZE) $(TARGET).elf

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Tell Make where to find source files
vpath %.c $(SRC_DIRS)
vpath %.s $(ASM_DIRS)

# Link object files into .elf
$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Pattern rule: compile any C file to object file
$(BUILD_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Pattern rule: assemble any .s file to object file
$(BUILD_DIR)/%.o: %.s
	$(AS) $< -o $@ -mcpu=$(MCU) -mthumb -g

# Phony targets
.PHONY: all clean flash

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Flash to microcontroller
flash: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $(TARGET).elf verify reset exit"