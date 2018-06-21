################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include pevents/subdir.mk
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
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: liblpu237-hidapi.so.1.0.0

# Tool invocations
liblpu237-hidapi.so.1.0.0: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/totoro/work/library/hidapi/Release" -Wl,-rpath=. -shared -Wl,-soname=liblpu237-hidapi.so.1 -pthread -o "liblpu237-hidapi.so.1.0.0" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(CC_DEPS)$(C++_DEPS)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) liblpu237-hidapi.so.1.0.0
	-@echo ' '

pre-build:
	-@echo 'remove symblic link'
	-rm liblpu237-hidapi.so;rm liblpu237-hidapi.so.1
	-@echo ' '

post-build:
	-@echo 'generate symbloic link'
	-ln -s liblpu237-hidapi.so.1.0.0 liblpu237-hidapi.so;ln -s liblpu237-hidapi.so.1.0.0 liblpu237-hidapi.so.1
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
