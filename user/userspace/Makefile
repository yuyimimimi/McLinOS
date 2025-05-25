PREFIX    = arm-none-eabi-
# PREFIX    =  linux-arm-gnueabi- #需要使用加载器2(beta)

CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

CPU       = -mcpu=cortex-m4
FPU       = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard


SRC_DIRS = 
INC_DIRS = 

SRC_DIRS += \
		main.c\

include lib/Makefile


BUILD_DIR = build
TARGET    = for_cortex_m4

C_SOURCES := $(shell find $(SRC_DIRS) -name '*.c')
C_INCLUDE := $(addprefix -I,$(INC_DIRS))

CFLAGS = $(CPU) -mthumb $(FPU) $(FLOAT-ABI) \
		 -fpic -ffreestanding -Os \
		 --specs=nano.specs  \
		 $(C_INCLUDE) \

#-nostdlib -nostartfiles -Wno-unused-function \

LDFLAGS = -T linker.ld  \
		 -Wl,--gc-sections \
		 -Wl,-e,reset_handler
		 
LIBS = -lc -lgcc

OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_FILES := $(patsubst %.c,$(BUILD_DIR)/%.s,$(C_SOURCES))
DEPS := $(patsubst %.c,$(BUILD_DIR)/%.d,$(C_SOURCES))

all: $(BUILD_DIR)/$(TARGET).bin $(ASM_FILES)
	@$(SIZE) $(BUILD_DIR)/$(TARGET).elf

# 创建构建目录结构
$(BUILD_DIR):
	mkdir -p $(dir $(OBJECTS))
	mkdir -p $(dir $(ASM_FILES))

# 生成依赖文件
$(BUILD_DIR)/%.d: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<

# 包含依赖文件
-include $(DEPS)

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

.PHONY: all clean