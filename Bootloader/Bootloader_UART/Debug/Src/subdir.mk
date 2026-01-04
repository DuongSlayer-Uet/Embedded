################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/CRC.c \
../Src/DMA.c \
../Src/DMAxRingBuffer.c \
../Src/Flash.c \
../Src/IWDG.c \
../Src/Interrupt.c \
../Src/Metadata.c \
../Src/RCC.c \
../Src/Ringbuffer.c \
../Src/SPI.c \
../Src/SPIxDMA.c \
../Src/Timer.c \
../Src/UART.c \
../Src/UARTxDMA.c \
../Src/gpio.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/CRC.o \
./Src/DMA.o \
./Src/DMAxRingBuffer.o \
./Src/Flash.o \
./Src/IWDG.o \
./Src/Interrupt.o \
./Src/Metadata.o \
./Src/RCC.o \
./Src/Ringbuffer.o \
./Src/SPI.o \
./Src/SPIxDMA.o \
./Src/Timer.o \
./Src/UART.o \
./Src/UARTxDMA.o \
./Src/gpio.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/CRC.d \
./Src/DMA.d \
./Src/DMAxRingBuffer.d \
./Src/Flash.d \
./Src/IWDG.d \
./Src/Interrupt.d \
./Src/Metadata.d \
./Src/RCC.d \
./Src/Ringbuffer.d \
./Src/SPI.d \
./Src/SPIxDMA.d \
./Src/Timer.d \
./Src/UART.d \
./Src/UARTxDMA.d \
./Src/gpio.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/CRC.cyclo ./Src/CRC.d ./Src/CRC.o ./Src/CRC.su ./Src/DMA.cyclo ./Src/DMA.d ./Src/DMA.o ./Src/DMA.su ./Src/DMAxRingBuffer.cyclo ./Src/DMAxRingBuffer.d ./Src/DMAxRingBuffer.o ./Src/DMAxRingBuffer.su ./Src/Flash.cyclo ./Src/Flash.d ./Src/Flash.o ./Src/Flash.su ./Src/IWDG.cyclo ./Src/IWDG.d ./Src/IWDG.o ./Src/IWDG.su ./Src/Interrupt.cyclo ./Src/Interrupt.d ./Src/Interrupt.o ./Src/Interrupt.su ./Src/Metadata.cyclo ./Src/Metadata.d ./Src/Metadata.o ./Src/Metadata.su ./Src/RCC.cyclo ./Src/RCC.d ./Src/RCC.o ./Src/RCC.su ./Src/Ringbuffer.cyclo ./Src/Ringbuffer.d ./Src/Ringbuffer.o ./Src/Ringbuffer.su ./Src/SPI.cyclo ./Src/SPI.d ./Src/SPI.o ./Src/SPI.su ./Src/SPIxDMA.cyclo ./Src/SPIxDMA.d ./Src/SPIxDMA.o ./Src/SPIxDMA.su ./Src/Timer.cyclo ./Src/Timer.d ./Src/Timer.o ./Src/Timer.su ./Src/UART.cyclo ./Src/UART.d ./Src/UART.o ./Src/UART.su ./Src/UARTxDMA.cyclo ./Src/UARTxDMA.d ./Src/UARTxDMA.o ./Src/UARTxDMA.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

