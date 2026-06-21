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
 *   gpio_init(&btn, ...);
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

#include "bsp_handle.h"
#include <stdint.h>

/* ============================================================
 * 抽象类型定义
 * ============================================================ */

/** GPIO 端口句柄（芯片实现内部转型为具体外设指针） */
typedef void *gpio_port_t;

/** GPIO 引脚掩码（如 STM32 上的 GPIO_PIN_x） */
typedef uint16_t gpio_pin_t;

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
} gpio_mode_t;

typedef enum {
    GPIO_Pull_None = 0,
    GPIO_Pull_Up   = 1,
    GPIO_Pull_Down = 2,
} gpio_pull_t;

typedef enum {
    GPIO_Speed_Low       = 0,
    GPIO_Speed_Medium    = 1,
    GPIO_Speed_High      = 2,
    GPIO_Speed_Very_High = 3,
} gpio_speed_t;

/* ============================================================
 * 结构体定义
 * ============================================================ */

typedef struct {
    gpio_port_t port;
    gpio_pin_t pin;
    gpio_mode_t mode;
    gpio_pull_t pull;
    gpio_speed_t speed;
    uint8_t alternate; /**< AF 编号，0-15（仅 AF 模式有效） */ 
} gpio_cfg_t;

typedef struct {
    uint32_t preempt_priority;
    uint32_t sub_priority;
} gpio_irq_cfg_t;

typedef struct {
    uint8_t irq_flag : 1;
} gpio_stat_t;

typedef struct {
    gpio_cfg_t *cfg;
    gpio_stat_t *stat;
} gpio_t;

/* TODO: 思考一下应当如何重构 */
typedef struct { 
    void (*init)(gpio_t *gpio);
    void (*deinit)(gpio_t *gpio);
    void (*write)(gpio_t *gpio, uint8_t value);
    uint8_t (*read)(gpio_t *gpio);
    void (*set_irq)(gpio_t *gpio, uint8_t enable);
    void (*clear_irq_flag)(gpio_t *gpio);
} gpio_ops_t;


#define MAX_EXTI_IRQ 16
extern gpio_t *gpio_irq_models[MAX_EXTI_IRQ];

sys_stat_t gpio_init(gpio_t *m, gpio_port_t port, gpio_pin_t pin, const GPIO_Config_t *cfg);
sys_stat_t gpio_deinit(gpio_t *m);

sys_stat_t gpio_write(gpio_t *m, GPIO_Level_t level);
GPIO_Level_t gpio_read(gpio_t *m);
sys_stat_t gpio_toggle(gpio_t *m);

sys_stat_t gpio_attach_irq(gpio_t *m);
sys_stat_t gpio_detach_irq(gpio_t *m);

#define gpio_set_irq_flag(m) ((m)->irq_flag = 1)
#define gpio_clear_irq_flag(m) ((m)->irq_flag = 0)

/** 从引脚掩码中提取引脚编号 (0-15), 可以有多种实现方式，包括强依赖 GCC 的 `__builtin_ctz` */
static inline int gpio_get_pin_num(gpio_pin_t pin)
{
    return (pin == 0) ? -1 : __builtin_ctz(pin);
}
