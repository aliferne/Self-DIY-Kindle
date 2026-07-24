#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sdio.h"
#include "bsp_spi.h"
#include "bsp_i2c.h"
#include "pin_src.h"
#include "bsp_sys.h"
#include "sdio.h"

gpio_t usr_led;
gpio_t pgup_btn;
gpio_t pgdown_btn;
gpio_t back_btn;
gpio_t home_btn;
gpio_t confirm_btn;

sdio_t sdcard;
gpio_t sdcard_det;

spi_t tft_spi;
gpio_t tft_dc;
gpio_t tft_rst;
gpio_t tft_blk;

/* touchpad related ----------- */
gpio_t tp_int;
gpio_t tp_rst;
iic_t tp_i2c;

/* TODO: I2C 的底层 API 尚未改写，需要改写 */
// static gpio_t tp_scl;
// static gpio_t tp_sda;

static void bsp_init_buttons(void);
static void bsp_init_leds(void);
static void bsp_init_tft(void);
static void bsp_init_touchpad(void);
static bool bsp_is_sdcard_inserted(void);

void bsp_init_hardware(void)
{
    dwt_init(); /* 初始化 DWT 以支持微秒级延时 */
    bsp_init_leds();
    bsp_init_buttons();
    /* 由于引入了 FatFs，这里直接在 disk_initialize 中调用此初始化函数 */
    // bsp_init_sdcard();
    bsp_init_tft();
    bsp_init_touchpad();
}

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

sdio_err_t bsp_init_sdcard(void)
{
    sdio_cfg_t sdio_cfg = {
        .mode     = SDIO_Mode_Polling,
        .wide_bus = 1,
    };

    /* 必须提供进出临界区的回调 */
    sdcard.enter_critical_cb = os_enter_critical;
    sdcard.exit_critical_cb  = os_exit_critical;
    sdcard.sdcard_det_cb     = bsp_is_sdcard_inserted;

    /* 需要先初始化检测口，sdio 的初始化依赖于卡是否插入 */
    gpio_init(
        &sdcard_det,
        (GPIO_Port_t)SDCARD_DET_PORT, (GPIO_Pin_t)SDCARD_DET_PIN,
        &(GPIO_Config_t){
            .mode  = GPIO_Mode_Input,
            .pull  = GPIO_Pull_Up,
            .speed = GPIO_Speed_Low,
        });

    sdio_err_t ret = sdio_init(&sdcard, (sdio_handle_t *)&hsd, &sdio_cfg);
    if (ret != SDIO_Err_Ok) {
        LOG_ERROR(
            "Failed to init sdio %s, into infinite loop\n",
            (ret == SDIO_Err_CardOut) ? "(card not inserted)" : "(other error)");
        for (;;);
    }

    return SDIO_Err_Ok;
}

static void bsp_init_tft(void)
{
    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Output_PP,
        .pull  = GPIO_Pull_None,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&tft_blk,
              (GPIO_Port_t)TFT_BLK_PORT, (GPIO_Pin_t)TFT_BLK_PIN,
              &init_conf);
    gpio_init(&tft_dc,
              (GPIO_Port_t)TFT_DC_PORT, (GPIO_Pin_t)TFT_DC_PIN,
              &init_conf);
    gpio_init(&tft_rst,
              (GPIO_Port_t)TFT_RST_PORT, (GPIO_Pin_t)TFT_RST_PIN,
              &init_conf);

    spi_reg_cfg_t spi_reg_cfg = {
        .drv     = SPI_Driver_HW,
        .src.hw  = (spi_handle_t)TFT_HSPI,
        .cs.port = TFT_CS_PORT,
        .cs.pin  = TFT_CS_PIN,
    };

    spi_cfg_t tft_spi_cfg = {
        .hw.timeout = 2000,
    };

    spi_init(&tft_spi,
             &spi_reg_cfg,
             &tft_spi_cfg);
}

static void bsp_init_touchpad(void)
{
    GPIO_Config_t init_conf = {
        .mode  = GPIO_Mode_Output_PP,
        .pull  = GPIO_Pull_None,
        .speed = GPIO_Speed_Low,
    };

    gpio_init(&tp_int,
              (GPIO_Port_t)TOUCH_INT_PORT, (GPIO_Pin_t)TOUCH_INT_PIN,
              &init_conf);
    gpio_init(&tp_rst,
              (GPIO_Port_t)TOUCH_RST_PORT, (GPIO_Pin_t)TOUCH_RST_PIN,
              &init_conf);

    iic_cfg_t i2c_cfg = {
        .sw = {
            .scl_delay_us = 50,
            .scl_pull     = GPIO_Pull_Up,
            .sda_pull     = GPIO_Pull_Up,
            .speed        = GPIO_Speed_Low,
        }};

    iic_init(&tp_i2c,
             (GPIO_Port_t)TOUCH_SDA_PORT, (GPIO_Pin_t)TOUCH_SDA_PIN,
             (GPIO_Port_t)TOUCH_SCL_PORT, (GPIO_Pin_t)TOUCH_SCL_PIN,
             &i2c_cfg);
}

static bool bsp_is_sdcard_inserted(void)
{
    GPIO_Level_t level = gpio_read(&sdcard_det);
    return (level == GPIO_Level_Low) ? true : false;
}
