#include "bsp_config.h"
#include "pin_src.h"

GPIO_Model_t usr_led;
GPIO_Model_t pgup_btn;
GPIO_Model_t pgdown_btn;
GPIO_Model_t back_btn;
GPIO_Model_t home_btn;
GPIO_Model_t confirm_btn;

I2C_Model_t oled;

SPI_Model_t spi;

static void bsp_init_buttons(void);
static void bsp_init_leds(void);
static void bsp_init_i2c(void);
static void bsp_init_spi(void);

void bsp_init_hardware(void)
{
    bsp_init_leds();
    bsp_init_buttons();
    bsp_init_i2c();
    bsp_init_spi();
}

/* ============================================================
 * 按键初始化（输入 + 上拉 + 下降沿中断）
 *
 * 中断仅置 irq_flag，应用层轮询后调用 gpio_clear_irq_flag() 清除。
 * ============================================================ */

static void bsp_init_buttons(void)
{
    /* --- 注册 --- */
    gpio_register(&pgup_btn,
                  (GPIO_Port_t)PAGEUP_BTN_PORT,
                  (GPIO_Pin_t)PAGEUP_BTN_PIN);
    gpio_register(&pgdown_btn,
                  (GPIO_Port_t)PAGEDOWN_BTN_PORT,
                  (GPIO_Pin_t)PAGEDOWN_BTN_PIN);
    gpio_register(&back_btn,
                  (GPIO_Port_t)BACK_BTN_PORT,
                  (GPIO_Pin_t)BACK_BTN_PIN);
    gpio_register(&home_btn,
                  (GPIO_Port_t)HOME_BTN_PORT,
                  (GPIO_Pin_t)HOME_BTN_PIN);
    gpio_register(&confirm_btn,
                  (GPIO_Port_t)CONFIRM_BTN_PORT,
                  (GPIO_Pin_t)CONFIRM_BTN_PIN);

    /* input + pull-up */
    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Input,
        .pull  = GPIO_Pull_Up,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&pgup_btn, &init_conf);
    gpio_init(&pgdown_btn, &init_conf);
    gpio_init(&back_btn, &init_conf);
    gpio_init(&home_btn, &init_conf);
    gpio_init(&confirm_btn, &init_conf);

    /* --- 中断配置（仅配硬件，ISR 自动置 irq_flag） --- */
    GPIO_IRQ_Config_t irq_conf = {
        .trigger_edge     = GPIO_Mode_IT_Falling,
        .preempt_priority = 5,
        .sub_priority     = 0,
    };

    gpio_attach_irq(&pgup_btn, &irq_conf);
    gpio_attach_irq(&pgdown_btn, &irq_conf);
    gpio_attach_irq(&back_btn, &irq_conf);
    gpio_attach_irq(&home_btn, &irq_conf);
    gpio_attach_irq(&confirm_btn, &irq_conf);
}

static void bsp_init_leds(void)
{
    gpio_register(&usr_led,
                  (GPIO_Port_t)USER_LED_PORT,
                  (GPIO_Pin_t)USER_LED_PIN);

    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Output_PP,
        .pull  = GPIO_Pull_None,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&usr_led, &init_conf);
}

static void bsp_init_i2c(void)
{
    i2c_register(&oled, GPIO_Speed_Low,
                 (GPIO_Port_t)I2C_SDA_PORT,
                 (GPIO_Pin_t)I2C_SDA_PIN,
                 (GPIO_Port_t)I2C_SCL_PORT,
                 (GPIO_Pin_t)I2C_SCL_PIN);

    I2C_Config_t i2c_conf = {
        .sw = {
            .scl_delay_us = 50,
            .scl_pull     = GPIO_Pull_Up,
            .sda_pull     = GPIO_Pull_Up,
        },
    };

    i2c_init(&oled, &i2c_conf);
}

static void bsp_init_spi(void)
{
    // spi_register();
}
