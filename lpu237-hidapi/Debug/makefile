################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include pevents/subdir.mk
-include inih/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: liblpu237-hidapi.so.1.0.0

# Tool invocations
liblpu237-hidapi.so.1.0.0: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/totoro/work/library/common/hidapi/lib/Debug" -shared -Wl,-soname=liblpu237-hidapi.so.1 -pthread -o "liblpu237-hidapi.so.1.0.0" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(CC_DEPS)$(C++_DEPS)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) liblpu237-hidapi.so.1.0.0
	-@echo ' '

post-build:
	-@echo 'copy so'
	-cp -f liblpu237-hidapi.so.1.0.0 /home/totoro/work/library/common/lpu237-hidapi/lib/Debug;cp -f liblpu237-hidapi.so.1.0.0 /home/totoro/work/library/tp_hidapi/Debug
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
