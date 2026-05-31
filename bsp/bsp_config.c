#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sdio.h"
#include "pin_src.h"
#include "bsp_sys.h"
#include "sdio.h"

GPIO_Model_t usr_led;
GPIO_Model_t pgup_btn;
GPIO_Model_t pgdown_btn;
GPIO_Model_t back_btn;
GPIO_Model_t home_btn;
GPIO_Model_t confirm_btn;

SDIO_Model_t storage;

static void bsp_init_buttons(void);
static void bsp_init_leds(void);
static void bsp_init_storage(void);

void bsp_init_hardware(void)
{
    dwt_init(); /* 初始化 DWT 以支持微秒级延时 */
    bsp_init_leds();
    bsp_init_buttons();
    bsp_init_storage();
}

/* ============================================================
 * 按键初始化（输入 + 上拉 + 下降沿中断）
 *
 * 中断仅置 irq_flag，应用层轮询后调用 gpio_clear_irq_flag() 清除。
 * ============================================================ */

static void bsp_init_buttons(void)
{
    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Input,
        .pull  = GPIO_Pull_Up,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&pgup_btn,
              (GPIO_Port_t)PAGEUP_BTN_PORT, (GPIO_Pin_t)PAGEUP_BTN_PIN,
              &init_conf);
    gpio_init(&pgdown_btn,
              (GPIO_Port_t)PAGEDOWN_BTN_PORT, (GPIO_Pin_t)PAGEDOWN_BTN_PIN,
              &init_conf);
    gpio_init(&back_btn,
              (GPIO_Port_t)BACK_BTN_PORT, (GPIO_Pin_t)BACK_BTN_PIN,
              &init_conf);
    gpio_init(&home_btn,
              (GPIO_Port_t)HOME_BTN_PORT, (GPIO_Pin_t)HOME_BTN_PIN,
              &init_conf);
    gpio_init(&confirm_btn,
              (GPIO_Port_t)CONFIRM_BTN_PORT, (GPIO_Pin_t)CONFIRM_BTN_PIN,
              &init_conf);

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
    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Output_PP,
        .pull  = GPIO_Pull_None,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&usr_led,
              (GPIO_Port_t)USER_LED_PORT, (GPIO_Pin_t)USER_LED_PIN,
              &init_conf);
}

static void bsp_init_storage(void)
{
    SDIO_Config_t sdio_cfg = {
        .mode     = SDIO_Mode_DMA,
        .wide_bus = 1,
    };

    sdio_init(&storage, (SDIO_Handle_t *)&hsd, &sdio_cfg);
}
