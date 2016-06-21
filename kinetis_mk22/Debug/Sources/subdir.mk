################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/console.c \
../Sources/leds.c \
../Sources/main.c \
../Sources/mcu_tracer.c \
../Sources/uart1.c 

OBJS += \
./Sources/console.o \
./Sources/leds.o \
./Sources/main.o \
./Sources/mcu_tracer.o \
./Sources/uart1.o 

C_DEPS += \
./Sources/console.d \
./Sources/leds.d \
./Sources/main.d \
./Sources/mcu_tracer.d \
./Sources/uart1.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"../Sources" -I"../Includes" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


