

######################################
# Target
######################################

STRUCT    = arm_m
PLATFORM  = stm32f4
PREFIX    = arm-none-eabi-

DEBUG = 1
OPT = -Og

######################################
# Building variables
######################################
BASE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR = $(BASE_DIR)/build
OUTPUT_DIR = $(BUILD_DIR)/out
TARGET    = $(BASE_DIR)/out/$(STRUCT)-$(PLATFORM)-image

######################################
# Source
######################################
C_SOURCES   =  
ASM_SOURCES =  
ASMM_SOURCES= 
AS_DEFS     = 
C_DEFS      =  
AS_INCLUDES = 
C_INCLUDES  =  

START_UP = $(BASE_DIR)/arch/$(STRUCT)/boot/startup.s 
ASM_SOURCES += $(START_UP)
LDSCRIPT = $(BASE_DIR)/arch/$(STRUCT)/boot/link.ld

DTS_FILE = $(BASE_DIR)/arch/$(STRUCT)/boot/dts/$(PLATFORM).dts
DTB_SOURCES = $(OUTPUT_DIR)/$(PLATFORM)-bord-out.dtb
DTB_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(DTB_SOURCES:.dtb=.dtb.o)))
OBJECTS += $(DTB_OBJECTS)

$(BUILD_DIR)/%.dtb.o: $(OUTPUT_DIR)/%.dtb | $(BUILD_DIR)
	$(PREFIX)objcopy -I binary -O elf32-littlearm --rename-section .data=.dtb $< $@

#######################################
# CFLAGS
#######################################

C_INCLUDES += \
    -I$(BASE_DIR)/include \
    -I$(BASE_DIR)/include/uapi \
    -I$(BASE_DIR)/arch/$(STRUCT)/include \
    -I$(BASE_DIR)/arch/$(STRUCT)/include/uapi \

CFLAGS = -nostdlib -nostartfiles -Wno-unused-function

CFLAGS += -DSTRUCT=\"$(STRUCT)\" -DPLATFORM=\"$(PLATFORM)\" -DPREFIX=\"$(PREFIX)\"






include $(BASE_DIR)/arch/$(STRUCT)/makefile 
include $(BASE_DIR)/drivers/makefile
include $(BASE_DIR)/fs/makefile
include $(BASE_DIR)/block/makefile
include $(BASE_DIR)/init/makefile
include $(BASE_DIR)/kernel/makefile
include $(BASE_DIR)/ipc/makefile
include $(BASE_DIR)/lib/makefile
include $(BASE_DIR)/user/makefile



ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# Compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LIBS    = 
LIBDIR  = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(OUTPUT_DIR)/$(notdir $(TARGET)).map,--cref -Wl,--gc-sections

# Default action: build all
all: $(OUTPUT_DIR)/$(notdir $(TARGET)).elf $(OUTPUT_DIR)/$(notdir $(TARGET)).hex $(OUTPUT_DIR)/$(notdir $(TARGET)).bin

#######################################
# Build the application
#######################################

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@$(AS) -c $(ASFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@$(AS) -c $(ASFLAGS) $< -o $@

$(OUTPUT_DIR)/$(notdir $(TARGET)).elf: $(OBJECTS) Makefile | $(OUTPUT_DIR)
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(OUTPUT_DIR)/%.hex: $(OUTPUT_DIR)/%.elf | $(OUTPUT_DIR)
	@$(HEX) $< $@
	
$(OUTPUT_DIR)/%.bin: $(OUTPUT_DIR)/%.elf | $(OUTPUT_DIR)
	@$(BIN) $< $@	

$(BUILD_DIR):
	@mkdir -p $@		

$(OUTPUT_DIR):
	@mkdir -p $@

#######################################
# Clean up
#######################################
clean:
	@if [ -d "$(BUILD_DIR)" ]; then rm -rf $(BUILD_DIR); fi
	@echo "Clean done!"

#######################################
# KCONFIG
#######################################
.PHONY: menuconfig

menuconfig:
	kconfig-mconf $(BASE_DIR)/arch/$(STRUCT)/config/kconfig.$(PLATFORM)
	@echo "Generating config/config.h from .config..."
	@grep '^CONFIG_' .config | sed 's/^CONFIG_/#define CONFIG_/; s/=\(.*\)/ \1/' > $(BASE_DIR)/include/generated/autoconf.h
	@mkdir -p $(BUILD_DIR)
	@echo "Create link script ..." 
	@cpp -nostdinc -I$(BASE_DIR)/arch/$(STRUCT)/boot -I$(BASE_DIR)/include -undef -x assembler-with-cpp $(BASE_DIR)/arch/$(STRUCT)/boot/ld/$(PLATFORM).ld.in -o $(LDSCRIPT)
	@echo "Create startup script ..." 
	@cpp -nostdinc -I$(BASE_DIR)/arch/$(STRUCT)/kernel -I$(BASE_DIR)/include -undef -x assembler-with-cpp $(BASE_DIR)/arch/$(STRUCT)/kernel/Interruptvectorscale.s -o $(START_UP)

dtbs:
	@mkdir -p $(OUTPUT_DIR)
	@touch $(DTB_SOURCES)
	@cpp -nostdinc -I$(BASE_DIR)/arch/$(STRUCT)/boot/dts -I$(BASE_DIR)/include -undef -x assembler-with-cpp $(DTS_FILE) -o $(DTS_FILE).pp
	@echo "Create dtb file..."
	@dtc -I dts -O dtb -o $(DTB_SOURCES) $(DTS_FILE).pp 

installtools:
	@echo "Installing required tools..."
	sudo apt-get update
	sudo apt-get install -y \
		device-tree-compiler \
		python3\
		kconfig-frontends \
		build-essential
	@echo "Tools installed!"

rootfs:
	@sudo python3 $(BASE_DIR)/user/rootfs/rootfs_create.py

userclean:
	@$(MAKE) -C $(BASE_DIR)/user/userspace clean

help:
	@echo "Available targets:"
	@echo "  all           - Build everything"
	@echo "  dtbs          - Compile device tree"
	@echo "  menuconfig    - Launch configuration UI"
	@echo "  rootfs        - Generate user rootfs"
	@echo "  clean         - Remove build artifacts"
	@echo "  installtools  - Install required tools"

#######################################
# Dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***