################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
STEROWNIK_GSM.lss \

FLASH_IMAGE += \
STEROWNIK_GSM.hex \

EEPROM_IMAGE += \
STEROWNIK_GSM.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: STEROWNIK_GSM.elf secondary-outputs

# Tool invocations
STEROWNIK_GSM.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,STEROWNIK_GSM.map -mmcu=atmega32 -o "STEROWNIK_GSM.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

STEROWNIK_GSM.lss: STEROWNIK_GSM.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S STEROWNIK_GSM.elf  >"STEROWNIK_GSM.lss"
	@echo 'Finished building: $@'
	@echo ' '

STEROWNIK_GSM.hex: STEROWNIK_GSM.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex STEROWNIK_GSM.elf  "STEROWNIK_GSM.hex"
	@echo 'Finished building: $@'
	@echo ' '

STEROWNIK_GSM.eep: STEROWNIK_GSM.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex STEROWNIK_GSM.elf  "STEROWNIK_GSM.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: STEROWNIK_GSM.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32 STEROWNIK_GSM.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) STEROWNIK_GSM.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
