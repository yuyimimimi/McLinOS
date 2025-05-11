######################################
# target
######################################


STRUCT    =   arm_m
PLATFORM  =   stm32f4
PREFIX    =   arm-none-eabi-


DEBUG = 1
OPT = -Og

######################################
# building variables
######################################

BUILD_DIR = build
TARGET    = out/$(STRUCT)-$(PLATFORM)-image


######################################
# source
######################################
# C sources

C_SOURCES   =  
ASM_SOURCES =  
ASMM_SOURCES= 
AS_DEFS 	= 
C_DEFS 	    =  
AS_INCLUDES = 
C_INCLUDES  =  


START_UP = arch/$(STRUCT)/boot/startup.s 
ASM_SOURCES += $(START_UP)
LDSCRIPT = arch/$(STRUCT)/boot/link.ld
DTS_FILE = arch/$(STRUCT)/boot/dts/$(PLATFORM).dts
DTB_SOURCES = $(BUILD_DIR)/out/$(PLATFORM)-bord-out.dtb


DTB_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(DTB_SOURCES:.dtb=.dtb.o)))
OBJECTS += $(DTB_OBJECTS)

$(BUILD_DIR)/%.dtb.o: ./$(BUILD_DIR)/out/%.dtb | $(BUILD_DIR)
	$(PREFIX)objcopy -I binary -O elf32-littlearm --rename-section .data=.dtb $< $@

#######################################
# CFLAGS
#######################################


C_INCLUDES += \
    -I./include \
	-I./include/uapi \
	-I./arch/$(STRUCT)/include \
	-I./arch/$(STRUCT)/include/uapi \

CFLAGS =  -nostdlib -nostartfiles -Wno-unused-function

CFLAGS += -DPLATFORM=$(PLATFORM) \
		  -DSTRUCT=$(STRUCT)

#CFLAGS += -include ./include/generated/autoconf.h \

include ./arch/$(STRUCT)/makefile 
include ./drivers/makefile
include ./fs/makefile
include ./block/makefile
include ./init/makefile
include ./kernel/makefile
include ./lib/makefile



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



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS += $(MCU) $(C_DEFS)  $(C_INCLUDES)  $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LIBS    = 
LIBDIR  = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin






#######################################
# build the application
#######################################
# list of objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(BIN) $< $@	

$(BUILD_DIR):
	@mkdir $@		

#######################################
# clean up
#######################################
clean:
	@if [ -n "$(wildcard $(BUILD_DIR)/*.d)" ]; then rm  $(BUILD_DIR)/*.d; fi
	@if [ -n "$(wildcard $(BUILD_DIR)/*.o)" ]; then rm  $(BUILD_DIR)/*.o; fi
	@if [ -n "$(wildcard $(BUILD_DIR)/*.lst)" ]; then rm  $(BUILD_DIR)/*.lst; fi
	@echo "clean done!"

#######################################
# KCONFIG you need use : apt install kconfig-frontends
#####################################
.PHONY: menuconfig

menuconfig:
	kconfig-mconf arch/$(STRUCT)/config/kconfig.$(PLATFORM)
	@echo "Generating config/config.h from .config..."
	@grep '^CONFIG_' .config | sed 's/^CONFIG_/#define CONFIG_/; s/=\(.*\)/ \1/' > include/generated/autoconf.h
	@mkdir -p $(BUILD_DIR)
	@echo "create link script ..." 
	@cpp -nostdinc -I./arch/$(STRUCT)/boot  -I./include -undef -x assembler-with-cpp arch/$(STRUCT)/boot/ld/$(PLATFORM).ld.in  -o  $(LDSCRIPT)
	@echo "create startup script ..." 
	@cpp -nostdinc -I./arch/$(STRUCT)/kernel -I./include -undef -x assembler-with-cpp arch/$(STRUCT)/kernel/Interruptvectorscale.s  -o  $(START_UP)


dtbs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/out
	@touch $(DTB_SOURCES)
	@cpp -nostdinc -I./arch/$(STRUCT)/boot/dts -I./include -undef -x assembler-with-cpp  $(DTS_FILE)  -o  $(DTS_FILE).pp
	@echo "create dtb file..."
	@dtc -I dts -O dtb -o $(DTB_SOURCES) $(DTS_FILE).pp 


installtools:
	@echo "Installing required tools..."
	sudo apt-get update
	sudo apt-get install -y \
		device-tree-compiler \
		kconfig-frontends \
		build-essential
	@echo "Tools installed!"
	 

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***


