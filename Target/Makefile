# Simple Makefile for Target build (STM32F407VG, FreeRTOS, HAL)
PROJECT=EnvMonTarget

# Toolchain
CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc
CP=arm-none-eabi-objcopy
SZ=arm-none-eabi-size

CFLAGS= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -O2 -ffunction-sections -fdata-sections \
        -Wall -Wextra -Werror -DSTM32F407xx
LDFLAGS= -Wl,--gc-sections -Wl,-Map=$(PROJECT).map

INCLUDES= -ITarget/Inc -ITarget/Src/SystemControl -ITarget/Src/WatchDog -ITarget/Src/Misc \
          -ITarget/Src/Temperature -ITarget/Src/Humidity -ITarget/Src/Light -ITarget/Src/Pressure -ITarget/Src/Storage

SRCS= Target/Src/main.c \
      Target/Src/stm32f4xx_hal_msp.c \
      Target/Src/freertos.c \
      Target/Src/system_stm32f4xx.c \
      Target/Src/SystemControl/sysControl.c \
      Target/Src/WatchDog/WDG.c \
      Target/Src/Misc/misc.c \
      Target/Src/Temperature/SHT30-DIS.c \
      Target/Src/Humidity/SHT30-DIS.c \
      Target/Src/Light/TSL25911.c \
      Target/Src/Pressure/BME280.c \
      Target/Src/Storage/W25Q64.c

OBJS=$(SRCS:.c=.o)

all: $(PROJECT).elf

$(PROJECT).elf: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -Tstm32f407vg.ld -o $@ $^ $(LDFLAGS)
	$(SZ) $@

clean:
	rm -f $(OBJS) $(PROJECT).elf $(PROJECT).map
