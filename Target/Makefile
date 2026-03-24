# Simple Makefile for Target build (STM32F407VG, FreeRTOS, HAL)
PROJECT=EnvMonTarget

# Toolchain
CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc
CP=arm-none-eabi-objcopy
SZ=arm-none-eabi-size

ROOT:=$(CURDIR)
LD_SCRIPT?=Target/stm32f407vg.ld

CFLAGS= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -O2 -ffunction-sections -fdata-sections \
        -Wall -Wextra -Werror -DSTM32F407xx
LDFLAGS= -Wl,--gc-sections -Wl,-Map=$(PROJECT).map -T$(LD_SCRIPT)
LIBS= -lc -lm -lnosys

INCLUDES= -ITarget/Inc -ITarget/Src/SystemControl -ITarget/Src/WatchDog -ITarget/Src/Misc \
          -ITarget/Src/Temperature -ITarget/Src/Humidity -ITarget/Src/Light -ITarget/Src/Pressure -ITarget/Src/Storage

# Optional external HAL/CMSIS paths (used by CI). Provide:
#  HALDIR=path/to/stm32f4xx_hal_driver
#  CMSISDEV=path/to/cmsis_device_f4
#  CMSISINC=path/to/CMSIS/Core/Include
ifneq ($(HALDIR),)
INCLUDES+= -I$(HALDIR)/Inc -I$(CMSISINC) -I$(CMSISDEV)/Include
HAL_SRCS= \
  $(HALDIR)/Src/stm32f4xx_hal.c \
  $(HALDIR)/Src/stm32f4xx_hal_rcc.c \
  $(HALDIR)/Src/stm32f4xx_hal_rcc_ex.c \
  $(HALDIR)/Src/stm32f4xx_hal_gpio.c \
  $(HALDIR)/Src/stm32f4xx_hal_cortex.c \
  $(HALDIR)/Src/stm32f4xx_hal_i2c.c \
  $(HALDIR)/Src/stm32f4xx_hal_spi.c \
  $(HALDIR)/Src/stm32f4xx_hal_tim.c \
  $(HALDIR)/Src/stm32f4xx_hal_tim_ex.c \
  $(HALDIR)/Src/stm32f4xx_hal_flash.c \
  $(HALDIR)/Src/stm32f4xx_hal_flash_ex.c \
  $(HALDIR)/Src/stm32f4xx_hal_pwr.c \
  $(HALDIR)/Src/stm32f4xx_hal_pwr_ex.c
endif

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
      Target/Src/Storage/W25Q64.c \
      Target/Startup/startup_stm32f407xx.s \
      $(HAL_SRCS)

OBJS=$(SRCS:.c=.o)
OBJS:=$(OBJS:.s=.o)

all: $(PROJECT).elf

$(PROJECT).elf: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(LIBS)
	$(SZ) $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.s
	$(AS) -x assembler-with-cpp $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(PROJECT).elf $(PROJECT).map
