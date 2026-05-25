#pragma once

/*
 * ============================================================
 * bsp_gpio.h  —  GPIO 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 芯片相关实现位于 chip/<vendor>/gpio_chip.c，
 *
 * 使用示例：
 *
 *   #include "bsp_gpio.h"
 *
 *   GPIO_Model_t btn;
 *   GPIO_Config_t cfg = {
 *       .mode  = GPIO_Mode_Input,
 *       .pull  = GPIO_Pull_Up,
 *   };
 *   gpio_register(&btn, ...);
 *   gpio_init(&btn, &cfg);
 *
 *   GPIO_IRQ_Config_t irq = {
 *       .trigger_edge     = GPIO_Mode_IT_Falling,
 *       .preempt_priority = 5,
 *   };
 *   gpio_attach_irq(&btn, &irq);
 *
 *   // 在任务中轮询
 *   if (btn.irq_flag) {
 *       gpio_clear_irq_flag(&btn);
 *       // 处理按键...
 *   }
 */

#include <stdint.h>

/* ============================================================
 * 抽象类型定义
 * ============================================================ */

/** GPIO 端口句柄（芯片实现内部转型为具体外设指针） */
typedef void *GPIO_Port_t;

/** GPIO 引脚掩码（如 STM32 上的 GPIO_PIN_x） */
typedef uint16_t GPIO_Pin_t;

/* ============================================================
 * 枚举定义
 * ============================================================ */

typedef enum {
    GPIO_Level_Low  = 0,
    GPIO_Level_High = 1,
} GPIO_Level_t;

typedef enum {
    GPIO_Mode_Input             = 0,
    GPIO_Mode_Output_PP         = 1,
    GPIO_Mode_Output_OD         = 2,
    GPIO_Mode_AF_PP             = 3,
    GPIO_Mode_AF_OD             = 4,
    GPIO_Mode_Analog            = 5,
    GPIO_Mode_IT_Rising         = 6,
    GPIO_Mode_IT_Falling        = 7,
    GPIO_Mode_IT_Rising_Falling = 8,
} GPIO_Mode_t;

typedef enum {
    GPIO_Pull_None = 0,
    GPIO_Pull_Up   = 1,
    GPIO_Pull_Down = 2,
} GPIO_Pull_t;

typedef enum {
    GPIO_Speed_Low       = 0,
    GPIO_Speed_Medium    = 1,
    GPIO_Speed_High      = 2,
    GPIO_Speed_Very_High = 3,
} GPIO_Speed_t;

typedef enum {
    GPIO_Err_OK                        = 0,
    GPIO_Err_Incorrect_Mode            = 1,
    GPIO_Err_Call_Write_When_Using_IRQ = 2,
    GPIO_Err_IRQ_Table_Was_Registered  = 3,
    GPIO_Err_Pin_Num_Out_Of_Index      = 4,
} GPIO_Err_t;

/* ============================================================
 * 结构体定义
 * ============================================================ */

typedef struct {
    GPIO_Mode_t mode;
    GPIO_Pull_t pull;
    GPIO_Speed_t speed;
    uint8_t alternate; /**< AF 编号，0-15（仅 AF 模式有效） */
} GPIO_Config_t;

typedef struct {
    GPIO_Mode_t trigger_edge; /**< 使用 GPIO_Mode_IT_* */
    uint32_t preempt_priority;
    uint32_t sub_priority;
} GPIO_IRQ_Config_t;

/*
 * hardware source of gpio
 */
typedef struct {
    GPIO_Port_t port;
    GPIO_Pin_t pin;
} GPIO_HWSource_t;

typedef struct {
    GPIO_HWSource_t src;
    GPIO_Config_t config;
    GPIO_IRQ_Config_t irq_config;
    uint8_t use_irq : 1;
    uint8_t irq_flag : 1; /**< ISR 置位，应用层查询后清除 */
} GPIO_Model_t;

/* ============================================================
 * 中断模型表
 *
 * 芯片 ISR 通过 pin 号查表找到对应的 GPIO_Model_t，
 * 然后置其 irq_flag。上层任务轮询 irq_flag 处理事件。
 *
 * 注意：STM32 的 EXTI 线按 pin 号共享 (PA0 和 PB0 共用 EXTI0),
 * 因此同一 pin 号只能有一个 model 注册中断。
 * ============================================================ */

#define GPIO_MAX_PIN 16

extern GPIO_Model_t *gpio_irq_models[GPIO_MAX_PIN];

GPIO_Err_t gpio_register(GPIO_Model_t *m, GPIO_Port_t port, GPIO_Pin_t pin);
GPIO_Err_t gpio_init(GPIO_Model_t *m, const GPIO_Config_t *cfg);
GPIO_Err_t gpio_deinit(GPIO_Model_t *m);

GPIO_Err_t gpio_write(GPIO_Model_t *m, GPIO_Level_t level);
GPIO_Level_t gpio_read(GPIO_Model_t *m);
GPIO_Err_t gpio_toggle(GPIO_Model_t *m);

GPIO_Err_t gpio_attach_irq(GPIO_Model_t *m, const GPIO_IRQ_Config_t *irq_cfg);
GPIO_Err_t gpio_detach_irq(GPIO_Model_t *m);

/** ISR 中调用，置 irq_flag（由芯片中断处理函数调用） */
void gpio_set_irq_flag(GPIO_Model_t *m);

/** 应用层调用，清除 irq_flag */
void gpio_clear_irq_flag(GPIO_Model_t *m);

/** 从引脚掩码中提取引脚编号 (0-15), 可以有多种实现方式，包括强依赖 GCC 的 `__builtin_ctz` */
int gpio_get_pin_num(GPIO_Pin_t pin);
