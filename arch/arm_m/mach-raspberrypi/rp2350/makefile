CPU = -mcpu=cortex-m33
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

C_INCLUDES += \
	-I$(BASE_DIR)/arch/$(STRUCT)/mach-$(VENDOR)/$(PLATFORM)/include 


C_SOURCES    += \
	$(wildcard $(BASE_DIR)/arch/$(STRUCT)/mach-$(VENDOR)/$(PLATFORM)/*.c) 


