################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/portable/fsl_tickless_systick.c \
../freertos/portable/heap_3.c \
../freertos/portable/port.c 

OBJS += \
./freertos/portable/fsl_tickless_systick.o \
./freertos/portable/heap_3.o \
./freertos/portable/port.o 

C_DEPS += \
./freertos/portable/fsl_tickless_systick.d \
./freertos/portable/heap_3.d \
./freertos/portable/port.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/portable/%.o: ../freertos/portable/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DFRDM_K64F -DFREEDOM -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 -DFSL_RTOS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_FREE_RTOS -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"C:\Projects\SE2\Wireless_Speaker\board" -I"C:\Projects\SE2\Wireless_Speaker\source" -I"C:\Projects\SE2\Wireless_Speaker" -I"C:\Projects\SE2\Wireless_Speaker\drivers" -I"C:\Projects\SE2\Wireless_Speaker\CMSIS" -I"C:\Projects\SE2\Wireless_Speaker\utilities" -I"C:\Projects\SE2\Wireless_Speaker\startup" -I"C:\Projects\SE2\Wireless_Speaker\lwip\contrib\apps\tcpecho" -I"C:\Projects\SE2\Wireless_Speaker\lwip\port\arch" -I"C:\Projects\SE2\Wireless_Speaker\lwip\port" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip\priv" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip\prot" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif\ppp\polarssl" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif\ppp" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\posix\sys" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\posix" -I"C:\Projects\SE2\Wireless_Speaker\freertos\include" -I"C:\Projects\SE2\Wireless_Speaker\freertos\portable" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include" -I"C:\Projects\SE2\Wireless_Speaker\lwip\contrib\apps" -O0 -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


