################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../pevents/pevents.cpp 

OBJS += \
./pevents/pevents.o 

CPP_DEPS += \
./pevents/pevents.d 


# Each subdirectory must supply rules for building sources it contributes
pevents/%.o: ../pevents/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -DWFMO -I"/home/totoro/work/library/lpu237-hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/pevents" -I"/home/totoro/work/library/lpu237-hidapi/inih" -I"/home/totoro/work/library/common/hidapi/inc" -I"/home/totoro/work/library/common/lpu237-hidapi/inc" -I"/home/totoro/work/library/common/inc" -O2 -Wall -c -fmessage-length=0 -fPIC -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


