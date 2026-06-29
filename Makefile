# ============================================================
# Makefile for Self-DIY-Kindle
#
# 基于 eide 配置 (.eide/eide.yml) 自动生成
# 工具链: arm-none-eabi-gcc (GCC)
# 目标芯片: STM32F407VGTx (Cortex-M4F)
# ============================================================

TARGET = Self-DIY-Kindle

######################################
# 工具链
######################################
PREFIX   = arm-none-eabi-
CC       = $(PREFIX)gcc
AS       = $(PREFIX)gcc -x assembler-with-cpp
CP       = $(PREFIX)objcopy
SZ       = $(PREFIX)size
HEX      = $(CP) -O ihex
BIN      = $(CP) -O binary -S

######################################
# 路径
######################################
BUILD_DIR = build/mk

######################################
# 显示屏选择
#   可选: st7796 (默认，带触摸), epaper, st7735s
######################################
USE_DISPLAY ?= st7796

######################################
# MCU 配置
######################################
CPU      = -mcpu=cortex-m4
FPU      = -mfpu=fpv4-sp-d16
FLOAT_ABI = -mfloat-abi=hard
MCU      = $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

######################################
# 汇编源文件
######################################
ASM_SOURCES = \
  startup_stm32f407xx.s

######################################
# C 源文件 — Core
######################################
C_SOURCES = \
  Core/Src/main.c \
  Core/Src/gpio.c \
  Core/Src/freertos.c \
  Core/Src/stm32f4xx_it.c \
  Core/Src/stm32f4xx_hal_msp.c \
  Core/Src/sysmem.c \
  Core/Src/syscalls.c \
  Core/Src/sdio.c \
  Core/Src/dma.c \
  Core/Src/spi.c \
  Core/Src/stm32f4xx_hal_timebase_tim.c \
  Core/Src/usart.c \
  Core/Src/system_stm32f4xx.c \
Core/Src/main.c \
Core/Src/gpio.c \
Core/Src/freertos.c \
Core/Src/dma.c \
Core/Src/sdio.c \
Core/Src/spi.c \
Core/Src/usart.c \
Core/Src/stm32f4xx_it.c \
Core/Src/stm32f4xx_hal_msp.c \
Core/Src/stm32f4xx_hal_timebase_tim.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
Core/Src/system_stm32f4xx.c \
Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
Middlewares/Third_Party/FreeRTOS/Source/list.c \
Middlewares/Third_Party/FreeRTOS/Source/queue.c \
Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
Middlewares/Third_Party/FreeRTOS/Source/timers.c \
Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
Core/Src/sysmem.c \
Core/Src/syscalls.c

######################################
# C 源文件 — HAL 驱动
######################################
C_SOURCES += \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_wwdg.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c


######################################
# C 源文件 — ebtn
######################################
C_SOURCES += \
  Middlewares/ebtn/ebtn.c

######################################
# C 源文件 — FatFs
######################################
C_SOURCES += \
  Middlewares/FatFs/diskio.c \
  Middlewares/FatFs/ff.c \
  Middlewares/FatFs/ffsystem.c \
  Middlewares/FatFs/ffunicode.c

######################################
# C 源文件 — FreeRTOS
######################################
C_SOURCES += \
  Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
  Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
  Middlewares/Third_Party/FreeRTOS/Source/list.c \
  Middlewares/Third_Party/FreeRTOS/Source/queue.c \
  Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
  Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
  Middlewares/Third_Party/FreeRTOS/Source/timers.c \
  Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

######################################
# C 源文件 — miniz
######################################
C_SOURCES += \
  Middlewares/miniz/miniz.c

######################################
# C 源文件 — middleware config
######################################
C_SOURCES += \
  Middlewares/mid_config.c

######################################
# C 源文件 — display drivers (由 USE_DISPLAY 控制)
######################################

ifeq ($(USE_DISPLAY), st7796)
  DISPLAY_DEF    = -DUSE_DISPLAY_ST7796
  DISPLAY_SRCS   = \
    Middlewares/display_drv/st7796-with-touch/LCD/LCD.c \
    Middlewares/display_drv/st7796-with-touch/TOUCH/ctp.c \
    Middlewares/display_drv/st7796-with-touch/touch_drv.c \
    Middlewares/display_drv/st7796-with-touch/display_drv.c
else ifeq ($(USE_DISPLAY), epaper)
  DISPLAY_DEF    = -DUSE_DISPLAY_EPAPER
  DISPLAY_SRCS   = \
    Middlewares/display_drv/epaper/epaper.c \
    Middlewares/display_drv/epaper/config/DEV_Config.c \
    Middlewares/display_drv/epaper/drv/epd_4in2_v2.c \
    Middlewares/display_drv/epaper/fonts/font12.c \
    Middlewares/display_drv/epaper/fonts/font12CN.c \
    Middlewares/display_drv/epaper/fonts/font16.c \
    Middlewares/display_drv/epaper/fonts/font20.c \
    Middlewares/display_drv/epaper/fonts/font24.c \
    Middlewares/display_drv/epaper/fonts/font24CN.c \
    Middlewares/display_drv/epaper/fonts/font8.c \
    Middlewares/display_drv/epaper/gui/gui_paint.c \
    Middlewares/display_drv/epaper/test/ImageData.c
else ifeq ($(USE_DISPLAY), st7735s)
  DISPLAY_DEF    = -DUSE_DISPLAY_ST7735S
  DISPLAY_SRCS   = \
    Middlewares/display_drv/st7735s/st7735s.c
else
  $(error 无效的 USE_DISPLAY 值: "$(USE_DISPLAY)"。可选: st7796, epaper, st7735s)
endif

C_SOURCES += $(DISPLAY_SRCS)
C_DEFS += $(DISPLAY_DEF)

######################################
# C 源文件 — algorithm
######################################
C_SOURCES += \
  Middlewares/algorithm/fifo.c

######################################
# C/ASM 源文件 — RTT
######################################
C_SOURCES += \
  Middlewares/RTT/RTT/SEGGER_RTT.c \
  Middlewares/RTT/RTT/SEGGER_RTT_printf.c \
  Middlewares/RTT/rtt_srv.c \
  # Middlewares/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c

ASM_SOURCES += \
  Middlewares/RTT/RTT/SEGGER_RTT_ASM_ARMv7M.s

######################################
# C 源文件 — LVGL (使用通配符自动收集)
######################################
LVGL_DIRS = \
  Middlewares/lvgl/src/core \
  Middlewares/lvgl/src/draw \
  Middlewares/lvgl/src/draw/sw \
  Middlewares/lvgl/src/draw/arm2d \
  Middlewares/lvgl/src/draw/stm32_dma2d \
  Middlewares/lvgl/src/extra \
  Middlewares/lvgl/src/extra/themes/mono \
  Middlewares/lvgl/src/extra/themes/default \
  Middlewares/lvgl/src/extra/themes/basic \
  Middlewares/lvgl/src/extra/widgets/animimg \
  Middlewares/lvgl/src/extra/widgets/calendar \
  Middlewares/lvgl/src/extra/widgets/chart \
  Middlewares/lvgl/src/extra/widgets/colorwheel \
  Middlewares/lvgl/src/extra/widgets/imgbtn \
  Middlewares/lvgl/src/extra/widgets/keyboard \
  Middlewares/lvgl/src/extra/widgets/led \
  Middlewares/lvgl/src/extra/widgets/list \
  Middlewares/lvgl/src/extra/widgets/menu \
  Middlewares/lvgl/src/extra/widgets/meter \
  Middlewares/lvgl/src/extra/widgets/msgbox \
  Middlewares/lvgl/src/extra/widgets/span \
  Middlewares/lvgl/src/extra/widgets/spinbox \
  Middlewares/lvgl/src/extra/widgets/spinner \
  Middlewares/lvgl/src/extra/widgets/tabview \
  Middlewares/lvgl/src/extra/widgets/tileview \
  Middlewares/lvgl/src/extra/widgets/win \
  Middlewares/lvgl/src/extra/layouts/grid \
  Middlewares/lvgl/src/extra/layouts/flex \
  Middlewares/lvgl/src/font \
  Middlewares/lvgl/src/hal \
  Middlewares/lvgl/src/misc \
  Middlewares/lvgl/src/widgets

C_SOURCES += $(wildcard $(LVGL_DIRS:%=%/*.c))

# LVGL porting
C_SOURCES += \
  Middlewares/lvgl/porting/lv_port_disp.c \
  Middlewares/lvgl/porting/lv_port_fs.c \
  Middlewares/lvgl/porting/lv_port_indev.c

# LVGL demos
C_SOURCES += \
  Middlewares/lvgl/demos/widgets/lv_demo_widgets.c \
  Middlewares/lvgl/demos/widgets/assets/img_clothes.c \
  Middlewares/lvgl/demos/widgets/assets/img_demo_widgets_avatar.c \
  Middlewares/lvgl/demos/widgets/assets/img_lvgl_logo.c

# LVGL fonts
C_SOURCES += \
  Middlewares/lvgl/fonts/simhei_size14.c \
  Middlewares/lvgl/fonts/simhei_size16.c \
  Middlewares/lvgl/fonts/simhei_size18.c

######################################
# C 源文件 — BSP
######################################
C_SOURCES += \
  bsp/bsp_config.c \
  bsp/bsp_rtc.c \
  bsp/bsp_i2c.c \
  bsp/bsp_spi.c

######################################
# C 源文件 — Services
######################################
C_SOURCES += \
  services/ebook_srv.c \
  services/music_srv.c \
  services/net_srv.c \
  services/time_srv.c \
  services/input_srv.c \
  services/storage_srv.c \
  services/srv_config.c

######################################
# C 源文件 — Application Tasks
######################################
C_SOURCES += \
  applications/input_task.c \
  applications/music_task.c \
  applications/net_task.c \
  applications/process_task.c \
  applications/ui/ui_task.c \
  applications/ui/ui_home_page.c \
  applications/ui/ui_net_page.c \
  applications/ui/ui_player_page.c \
  applications/ui/ui_reader_page.c \
  applications/ui/ui_settings_page.c \

######################################
# C 源文件 — Chip layer
######################################
C_SOURCES += \
  chip/stm32f4/gpio_chip.c \
  chip/stm32f4/i2c_chip.c \
  chip/stm32f4/i2s_chip.c \
  chip/stm32f4/spi_chip.c \
  chip/stm32f4/sys_chip.c \
  chip/stm32f4/sdio_chip.c \
  chip/stm32f4/uart_chip.c \
  chip/stm32f4/irq/irq_gpio_exti.c \
  chip/stm32f4/irq/irq_sys_it.c \
  chip/stm32f4/irq/irq_sdio.c \
  chip/stm32f4/irq/irq_uart.c

######################################
# 预定义宏
######################################
C_DEFS = \
-D  USE_HAL_DRIVER \
-D  STM32F407xx \
-DUSE_HAL_DRIVER \
-DSTM32F407xx

######################################
# 头文件搜索路径
######################################
C_INCLUDES = \
  -ICore/Inc \
  -IDrivers/STM32F4xx_HAL_Driver/Inc \
  -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
  -IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
  -IDrivers/CMSIS/Include \
  -IMiddlewares/Third_Party/FreeRTOS/Source/include \
  -IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
  -IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
  -IMiddlewares/FatFs \
  -IMiddlewares/miniz \
  -Ibsp \
  -Iservices \
  -Iapplications \
  -Iapplications/ui \
  -I. \
  -Ichip/stm32f4 \
  -IMiddlewares \
  -IMiddlewares/lvgl \
  -IMiddlewares/ebtn \
  -IMiddlewares/lvgl/demos \
  -IMiddlewares/lvgl/porting \
  -IMiddlewares/lvgl/src/core \
  -IMiddlewares/lvgl/src/font \
  -IMiddlewares/lvgl/src \
  -IMiddlewares/lvgl/src/hal \
  -IMiddlewares/lvgl/src/widgets \
  -IMiddlewares/lvgl/src/misc \
  -IMiddlewares/lvgl/src/draw \
  -IMiddlewares/lvgl/src/draw/stm32_dma2d \
  -IMiddlewares/lvgl/src/draw/sw \
  -IMiddlewares/display_drv \
  -IMiddlewares/display_drv/st7796-with-touch \
  -IMiddlewares/display_drv/st7796-with-touch/LCD \
  -IMiddlewares/display_drv/st7796-with-touch/TOUCH \
  -IMiddlewares/display_drv/epaper \
  -IMiddlewares/display_drv/epaper/config \
  -IMiddlewares/display_drv/epaper/drv \
  -IMiddlewares/display_drv/epaper/fonts \
  -IMiddlewares/display_drv/epaper/gui \
  -IMiddlewares/display_drv/epaper/test \
  -IMiddlewares/display_drv/st7735s \
  -IMiddlewares/algorithm \
  -IMiddlewares/RTT \
  -IMiddlewares/RTT/RTT \
  -IMiddlewares/RTT/Config \
  -ICore/Inc \
  -IDrivers/STM32F4xx_HAL_Driver/Inc \
  -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
  -IMiddlewares/Third_Party/FreeRTOS/Source/include \
  -IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
  -IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
  -IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
  -IDrivers/CMSIS/Include

######################################
# 编译选项
######################################
OPT = -Og
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) \
  -Wall \
  -Wextra \
  -Wpedantic \
  -std=c11 \
  -ffunction-sections \
  -fdata-sections \
  -g \
  -gdwarf-2 \
  -u _printf_float \
  -u _scanf_float \
  -MMD \
  -MP

ASFLAGS = $(MCU) $(OPT) $(C_INCLUDES) \
  -Wall \
  -ffunction-sections \
  -fdata-sections \
  -g \
  -gdwarf-2

######################################
# 链接选项
######################################
LDSCRIPT = STM32F407XX_FLASH.ld
LDFLAGS = $(MCU) \
  -specs=nano.specs \
  -T$(LDSCRIPT) \
  -lm \
  -lc \
  -lnosys \
  -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
  -Wl,--gc-sections

######################################
# 目标文件（使用完整路径避免文件名冲突）
######################################
C_SOURCES := $(sort $(C_SOURCES))
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

######################################
# 构建规则
######################################
.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@echo "====== Build complete: $^ ======"

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(basename $@).lst $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

# 自动依赖（路径与目标文件对应）
-include $(C_SOURCES:%.c=$(BUILD_DIR)/%.d)
