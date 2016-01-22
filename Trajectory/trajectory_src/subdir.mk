################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../trajectory_src/Integrator.cpp \
../trajectory_src/firstsim.cpp \
../trajectory_src/trajectory.cpp 

OBJS += \
./trajectory_src/Integrator.o \
./trajectory_src/firstsim.o \
./trajectory_src/trajectory.o 

CPP_DEPS += \
./trajectory_src/Integrator.d \
./trajectory_src/firstsim.d \
./trajectory_src/trajectory.d 


# Each subdirectory must supply rules for building sources it contributes
trajectory_src/%.o: ../trajectory_src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


