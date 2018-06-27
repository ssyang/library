################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/inner_event.cpp \
../src/inner_worker.cpp \
../src/log_util.cpp \
../src/lpu237-hidapi.cpp 

OBJS += \
./src/inner_event.o \
./src/inner_worker.o \
./src/log_util.o \
./src/lpu237-hidapi.o 

CPP_DEPS += \
./src/inner_event.d \
./src/inner_worker.d \
./src/log_util.d \
./src/lpu237-hidapi.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -DWFMO -I"/home/totoro/work/library/lpu237-hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/pevents" -I"/home/totoro/work/library/lpu237-hidapi/inih" -I"/home/totoro/work/library/common/hidapi/inc" -I"/home/totoro/work/library/common/lpu237-hidapi/inc" -I"/home/totoro/work/library/common/inc" -O2 -Wall -c -fmessage-length=0 -fPIC -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


