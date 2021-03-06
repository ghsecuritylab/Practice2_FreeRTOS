################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/source/croutine.c \
../freertos/source/event_groups.c \
../freertos/source/list.c \
../freertos/source/queue.c \
../freertos/source/tasks.c \
../freertos/source/timers.c 

OBJS += \
./freertos/source/croutine.o \
./freertos/source/event_groups.o \
./freertos/source/list.o \
./freertos/source/queue.o \
./freertos/source/tasks.o \
./freertos/source/timers.o 

C_DEPS += \
./freertos/source/croutine.d \
./freertos/source/event_groups.d \
./freertos/source/list.d \
./freertos/source/queue.d \
./freertos/source/tasks.d \
./freertos/source/timers.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/source/%.o: ../freertos/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DFRDM_K64F -DFREEDOM -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 -DFSL_RTOS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_FREE_RTOS -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"C:\Projects\SE2\Wireless_Speaker\board" -I"C:\Projects\SE2\Wireless_Speaker\source" -I"C:\Projects\SE2\Wireless_Speaker" -I"C:\Projects\SE2\Wireless_Speaker\drivers" -I"C:\Projects\SE2\Wireless_Speaker\CMSIS" -I"C:\Projects\SE2\Wireless_Speaker\utilities" -I"C:\Projects\SE2\Wireless_Speaker\startup" -I"C:\Projects\SE2\Wireless_Speaker\lwip\contrib\apps\tcpecho" -I"C:\Projects\SE2\Wireless_Speaker\lwip\port\arch" -I"C:\Projects\SE2\Wireless_Speaker\lwip\port" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip\priv" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip\prot" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\lwip" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif\ppp\polarssl" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif\ppp" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\netif" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\posix\sys" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include\posix" -I"C:\Projects\SE2\Wireless_Speaker\freertos\include" -I"C:\Projects\SE2\Wireless_Speaker\freertos\portable" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src" -I"C:\Projects\SE2\Wireless_Speaker\lwip\src\include" -I"C:\Projects\SE2\Wireless_Speaker\lwip\contrib\apps" -O0 -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


