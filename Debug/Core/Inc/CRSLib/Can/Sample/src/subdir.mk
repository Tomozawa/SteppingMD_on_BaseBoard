################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Inc/CRSLib/Can/Sample/src/sample.cpp 

OBJS += \
./Core/Inc/CRSLib/Can/Sample/src/sample.o 

CPP_DEPS += \
./Core/Inc/CRSLib/Can/Sample/src/sample.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/CRSLib/Can/Sample/src/%.o Core/Inc/CRSLib/Can/Sample/src/%.su: ../Core/Inc/CRSLib/Can/Sample/src/%.cpp Core/Inc/CRSLib/Can/Sample/src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++20 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-CRSLib-2f-Can-2f-Sample-2f-src

clean-Core-2f-Inc-2f-CRSLib-2f-Can-2f-Sample-2f-src:
	-$(RM) ./Core/Inc/CRSLib/Can/Sample/src/sample.d ./Core/Inc/CRSLib/Can/Sample/src/sample.o ./Core/Inc/CRSLib/Can/Sample/src/sample.su

.PHONY: clean-Core-2f-Inc-2f-CRSLib-2f-Can-2f-Sample-2f-src

