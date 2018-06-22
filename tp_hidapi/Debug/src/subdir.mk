################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/dll_lpu237.cpp \
../src/tp_hidapi.cpp 

OBJS += \
./src/dll_lpu237.o \
./src/tp_hidapi.o 

CPP_DEPS += \
./src/dll_lpu237.d \
./src/tp_hidapi.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/totoro/work/library/common/lpu237-hidapi/inc" -O0 -g3 -Wall -c -fmessage-length=0 -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


