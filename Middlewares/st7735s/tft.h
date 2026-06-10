#pragma once

/*
 * ============================================================
 * St7735tft.h — ST7735S TFT 驱动
 *
 * 使用方式：
 *   1. 调用方自行初始化 SPI_Model_t 和 GPIO_Model_t（DC/RST/BLK）
 *   2. 填充 TFT_t 上下文
 *   3. 调用 TFT_Init(&tft)
 *
 *   示例：
 *     SPI_Model_t lcd_spi = { ... };    // 已初始化
 *     GPIO_Model_t lcd_dc  = { ... };
 *     GPIO_Model_t lcd_rst = { ... };
 *
 *     TFT_t tft = {
 *         .spi     = &lcd_spi,
 *         .dc_pin  = &lcd_dc,
 *         .rst_pin = &lcd_rst,
 *         .blk_pin = NULL,               // 不使用背光控制
 *     };
 *     TFT_Init(&tft);
 * ============================================================
 */

#include <stdint.h>
#include "bsp_spi.h"
#include "bsp_gpio.h"

/* ============================================================
 * TFT 控制上下文
 * ============================================================ */

typedef struct {
    uint8_t is_initialized; /**< 是否已经初始化 */
    SPI_Model_t *spi;       /**< SPI 总线（软件或硬件均可） */
    GPIO_Model_t *dc_pin;   /**< Data/Command 引脚 */
    GPIO_Model_t *rst_pin;  /**< Reset 引脚 */
    GPIO_Model_t *blk_pin;  /**< 背光引脚（无需控制可传 NULL） */
} TFT_Model_t;

/* ============================================================
 * 常用颜色
 * ============================================================ */

#define TFT_RED           0xf800
#define TFT_GREEN         0x07e0
#define TFT_BLUE          0x001f
#define TFT_BLUE2         0x1c9f
#define TFT_PINK          0xd8a7
#define TFT_ORANGE        0xfa20
#define TFT_WHITE         0xffff
#define TFT_BLACK         0x0000
#define TFT_YELLOW        0xFFE0
#define TFT_CYAN          0x07ff
#define TFT_PURPLE        0xf81f
#define TFT_PURPLE2       0xdb92
#define TFT_PURPLE3       0x8811
#define TFT_GRAY0         0xEF7D
#define TFT_GRAY1         0x8410
#define TFT_GRAY2         0x4208

/* ============================================================
 * API
 * ============================================================ */

/* ---- SPI 通信原语 ---- */
void TFT_Init(TFT_Model_t *tft);                               /**< 初始化 TFT */
void TFT_DeInit(TFT_Model_t *tft);                             /**< 去初始化 */
void TFT_Reset(TFT_Model_t *tft);                              /**< 硬件复位 */
void TFT_TurnOff(TFT_Model_t *tft, uint8_t io);                /**< 背光开关 0=关 1=开 */
void TFT_SendIndex(TFT_Model_t *tft, uint8_t reg);             /**< 发送指令 */
void TFT_SendData(TFT_Model_t *tft, uint8_t data);             /**< 发送 8 位数据 */
void TFT_Send16Bit(TFT_Model_t *tft, uint16_t data);           /**< 发送 16 位数据 */
void TFT_SendReg(TFT_Model_t *tft, uint8_t addr, uint8_t val); /**< 发送指令+数据 */

/* ---- TFT 控制 ---- */
void TFT_SpinScreen(TFT_Model_t *tft, uint8_t dir); /**< 旋转方向 0-3 */
void TFT_SetCursor(TFT_Model_t *tft, uint16_t x, uint16_t y);
void TFT_Clear(TFT_Model_t *tft, uint16_t color); /**< 清屏 */
void TFT_SetRegion(TFT_Model_t *tft,
                   uint16_t x1, uint16_t y1,
                   uint16_t x2, uint16_t y2); /**< 选中区域 */
void TFT_FullScreen(TFT_Model_t *tft,
                    uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2,
                    uint16_t color); /**< 区间填充 */

/* ---- 基本绘制 ---- */
void TFT_DrawPoint(TFT_Model_t *tft, uint16_t x, uint16_t y, uint16_t color);
void TFT_DrawCircle(TFT_Model_t *tft, uint16_t x, uint16_t y,
                    uint16_t r, uint16_t color);
void TFT_DrawLine(TFT_Model_t *tft,
                  uint16_t x0, uint16_t y0,
                  uint16_t x1, uint16_t y1,
                  uint16_t color);

/* ---- 组合绘制 ---- */
void TFT_box(TFT_Model_t *tft,
             uint16_t x, uint16_t y,
             uint16_t w, uint16_t h,
             uint16_t color);
void TFT_box2(TFT_Model_t *tft,
              uint16_t x, uint16_t y,
              uint16_t w, uint16_t h,
              uint8_t mode);
void ButtonDown(TFT_Model_t *tft,
                uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2);
void ButtonUp(TFT_Model_t *tft,
              uint16_t x1, uint16_t y1,
              uint16_t x2, uint16_t y2);

/* ---- 图片、文字、中文字符 ---- */
void TFT_ShowImage(TFT_Model_t *tft,
                   uint16_t x, uint16_t y,
                   uint16_t length, uint16_t width,
                   const unsigned char *p);
void TFT_ShowChar(TFT_Model_t *tft,
                  uint8_t x, uint8_t y,
                  uint16_t fc, uint16_t bc, char c);
void TFT_ShowString(TFT_Model_t *tft,
                    uint8_t x, uint8_t y,
                    uint16_t fc, uint16_t bc, char *c);
void TFT_ShowNumber(TFT_Model_t *tft,
                    uint8_t x, uint8_t y,
                    uint16_t fc, uint16_t bc,
                    long long num);

/* ---- 中文 ---- */
int map(char *c);
void TFT_ShowChinese(TFT_Model_t *tft,
                     uint8_t x, uint8_t y,
                     uint16_t fc, uint16_t bc, char *c);
