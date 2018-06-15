################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/inner_worker.cpp 

C_SRCS += \
../src/lpu237-hidapi.c 

OBJS += \
./src/inner_worker.o \
./src/lpu237-hidapi.o 

CPP_DEPS += \
./src/inner_worker.d 

C_DEPS += \
./src/lpu237-hidapi.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/totoro/work/library/hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/totoro/work/library/hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


