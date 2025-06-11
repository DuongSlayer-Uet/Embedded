################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Flash.c \
../Src/Interrupt.c \
../Src/RCC.c \
../Src/Ringbuffer.c \
../Src/Timer.c \
../Src/UART.c \
../Src/gpio.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/Flash.o \
./Src/Interrupt.o \
./Src/RCC.o \
./Src/Ringbuffer.o \
./Src/Timer.o \
./Src/UART.o \
./Src/gpio.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/Flash.d \
./Src/Interrupt.d \
./Src/RCC.d \
./Src/Ringbuffer.d \
./Src/Timer.d \
./Src/UART.d \
./Src/gpio.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Flash.cyclo ./Src/Flash.d ./Src/Flash.o ./Src/Flash.su ./Src/Interrupt.cyclo ./Src/Interrupt.d ./Src/Interrupt.o ./Src/Interrupt.su ./Src/RCC.cyclo ./Src/RCC.d ./Src/RCC.o ./Src/RCC.su ./Src/Ringbuffer.cyclo ./Src/Ringbuffer.d ./Src/Ringbuffer.o ./Src/Ringbuffer.su ./Src/Timer.cyclo ./Src/Timer.d ./Src/Timer.o ./Src/Timer.su ./Src/UART.cyclo ./Src/UART.d ./Src/UART.o ./Src/UART.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

