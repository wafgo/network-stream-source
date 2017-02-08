################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../initcalls.c \
../main.c \
../streaming_source.c \
../v4l_device.c 

OBJS += \
./initcalls.o \
./main.o \
./streaming_source.o \
./v4l_device.o 

C_DEPS += \
./initcalls.d \
./main.d \
./streaming_source.d \
./v4l_device.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/sefo/devel/workspace_cpp/network-stream-source/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


