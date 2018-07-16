################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/KBMapTable.cpp \
../src/dev_lpu237.cpp \
../src/inner_event.cpp \
../src/inner_rom.cpp \
../src/inner_worker.cpp \
../src/log_util.cpp \
../src/lpu237-hidapi.cpp 

C_SRCS += \
../src/KISA_SHA256.c 

OBJS += \
./src/KBMapTable.o \
./src/KISA_SHA256.o \
./src/dev_lpu237.o \
./src/inner_event.o \
./src/inner_rom.o \
./src/inner_worker.o \
./src/log_util.o \
./src/lpu237-hidapi.o 

CPP_DEPS += \
./src/KBMapTable.d \
./src/dev_lpu237.d \
./src/inner_event.d \
./src/inner_rom.d \
./src/inner_worker.d \
./src/log_util.d \
./src/lpu237-hidapi.d 

C_DEPS += \
./src/KISA_SHA256.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -DWFMO -I"/home/totoro/work/library/lpu237-hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/pevents" -I"/home/totoro/work/library/lpu237-hidapi/inih" -I"/home/totoro/work/library/common/hidapi/inc" -I"/home/totoro/work/library/common/lpu237-hidapi/inc" -I"/home/totoro/work/library/common/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -DWFMO -I"/home/totoro/work/library/lpu237-hidapi/inc" -I"/home/totoro/work/library/lpu237-hidapi/pevents" -I"/home/totoro/work/library/lpu237-hidapi/inih" -I"/home/totoro/work/library/common/hidapi/inc" -I"/home/totoro/work/library/common/lpu237-hidapi/inc" -I"/home/totoro/work/library/common/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


