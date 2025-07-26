################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BME690/bme69x.c 

OBJS += \
./BME690/bme69x.o 

C_DEPS += \
./BME690/bme69x.d 


# Each subdirectory must supply rules for building sources it contributes
BME690/bme69x.o: ../BME690/bme69x.c BME690/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32U575xx -c -I../Core/Inc -I"/Users/matteogosi/STM32CubeIDE/workspace_1.18.1/Waveshare_LCD_U575/BME690" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32U5xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BME690

clean-BME690:
	-$(RM) ./BME690/bme69x.cyclo ./BME690/bme69x.d ./BME690/bme69x.o ./BME690/bme69x.su

.PHONY: clean-BME690

