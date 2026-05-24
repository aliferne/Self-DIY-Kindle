#include "bsp_config.h"

/* ============= GPIO Configurations =============== */
GPIO_Model_t usr_led;
GPIO_Model_t pgup_btn;
GPIO_Model_t pgdown_btn;
GPIO_Model_t back_btn;
GPIO_Model_t home_btn;
GPIO_Model_t confirm_btn;
/* ============= GPIO Configurations =============== */

/* ============= Inner Functions =============== */
static void bsp_init_buttons();
static void bsp_init_leds();
/* ============= Inner Functions =============== */

/*
 * initialize hardware, including GPIOs and other peripherals.
 */
void bsp_init_hardware()
{
    bsp_init_leds();
    bsp_init_buttons();
}

static void bsp_init_buttons()
{
    gpio_register(&pgup_btn, PAGEUP_BTN_PORT, PAGEUP_BTN_PIN);
    gpio_register(&pgdown_btn, PAGEDOWN_BTN_PORT, PAGEDOWN_BTN_PIN);
    gpio_register(&back_btn, BACK_BTN_PORT, BACK_BTN_PIN);
    gpio_register(&home_btn, HOME_BTN_PORT, HOME_BTN_PIN);
    gpio_register(&confirm_btn, CONFIRM_BTN_PORT, CONFIRM_BTN_PIN);

    GPIOx_Config_t init_conf = {0};
    init_conf.mode           = GPIO_MODE_INPUT;
    init_conf.pull           = GPIO_NOPULL;

    pgup_btn.init(&pgup_btn, &init_conf);
    pgdown_btn.init(&pgdown_btn, &init_conf);
    back_btn.init(&back_btn, &init_conf);
    home_btn.init(&home_btn, &init_conf);
    confirm_btn.init(&confirm_btn, &init_conf);

    /* TODO: button with irq */
}

static void bsp_init_leds()
{
    gpio_register(&usr_led, USER_LED_PORT, USER_LED_PIN);

    GPIOx_Config_t init_conf = {0};
    init_conf.mode           = GPIO_MODE_OUTPUT_PP;
    init_conf.pull           = GPIO_NOPULL;

    usr_led.init(&usr_led, &init_conf);
}
