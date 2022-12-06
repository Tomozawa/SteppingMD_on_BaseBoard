################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.c 

C_DEPS += \
./Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.d 

OBJS += \
./Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/CRSLib/CMSIS/src/%.o Core/Inc/CRSLib/CMSIS/src/%.su: ../Core/Inc/CRSLib/CMSIS/src/%.c Core/Inc/CRSLib/CMSIS/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu18 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-CRSLib-2f-CMSIS-2f-src

clean-Core-2f-Inc-2f-CRSLib-2f-CMSIS-2f-src:
	-$(RM) ./Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.d ./Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.o ./Core/Inc/CRSLib/CMSIS/src/cmsis_for_cpp.su

.PHONY: clean-Core-2f-Inc-2f-CRSLib-2f-CMSIS-2f-src

