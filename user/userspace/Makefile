CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU       = -mcpu=cortex-m4
FPU       = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard


C_INCLUDE = 
BUILD_DIR = build
C_SOURCES = main.c
TARGET    = for_cortex_m4


CFLAGS = $(CPU) -mthumb $(FPU) $(FLOAT-ABI) -fpic -ffreestanding -nostdlib -Os $(C_INCLUDE)
LDFLAGS = -T linker.ld -nostdlib -Wl,--gc-sections -Wl,-e,reset_handler
LIBS = -lc -lgcc

OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_FILES = $(patsubst %.c,$(BUILD_DIR)/%.s,$(C_SOURCES))

all: $(BUILD_DIR)/$(TARGET).bin $(ASM_FILES)
	@$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.s: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
