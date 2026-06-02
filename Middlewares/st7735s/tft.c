/*
 * ============================================================
 * St7735tft.c — ST7735S TFT 驱动
 *
 * 使用项目 bsp_spi / bsp_gpio 抽象层取代直接寄存器操作。
 *
 * 对外接口所有函数增加 TFT_t *tft 参数，
 * 模块内部通过 g_tft 全局指针供高层绘图函数查找上下文。
 * ============================================================
 */

#include "string.h"
#include "math.h"
#include "tft.h"
#include "bsp_sys.h"
#include "tft_font.h"

#define TFT_DELAY_MS(ms) chip_delay_ms(ms)

/* ============================================================
 * SPI 通信原语
 *
 * 所有写操作前自动选中 CS，写完后释放 CS。
 * DC 和 RST/BLK 由调用方通过 gpio_write 控制。
 * ============================================================ */

static void tft_spi_write(TFT_Model_t *tft, const uint8_t *data, uint16_t len)
{
    if (tft == NULL || tft->spi == NULL)
        return;

    spi_cs_select(tft->spi);
    spi_write(tft->spi, data, len);
    spi_cs_deselect(tft->spi);
}

/* ============================================================
 * API 实现
 * ============================================================ */

/* ---------- 初始化 ---------- */

void TFT_Init(TFT_Model_t *tft)
{
    TFT_Reset(tft);

    TFT_SendIndex(tft, 0x11); // 唤醒
    TFT_DELAY_MS(120);

    // ---- 基本配置 ----
    TFT_SendIndex(tft, 0x36);
    TFT_SendData(tft, 0x00);
    TFT_SendIndex(tft, 0x3A);
    TFT_SendData(tft, 0x05); // 16bit

    // 帧率
    TFT_SendIndex(tft, 0xB1);
    TFT_SendData(tft, 0x05);
    TFT_SendData(tft, 0x3C);
    TFT_SendData(tft, 0x3C);

    TFT_SendIndex(tft, 0xB2);
    TFT_SendData(tft, 0x05);
    TFT_SendData(tft, 0x3C);
    TFT_SendData(tft, 0x3C);

    TFT_SendIndex(tft, 0xB3);
    TFT_SendData(tft, 0x05);
    TFT_SendData(tft, 0x3C);
    TFT_SendData(tft, 0x3C);
    TFT_SendData(tft, 0x05);
    TFT_SendData(tft, 0x3C);
    TFT_SendData(tft, 0x3C);

    TFT_SendIndex(tft, 0xB4);
    TFT_SendData(tft, 0x03);

    // 电源控制
    TFT_SendIndex(tft, 0xC0);
    TFT_SendData(tft, 0x2E);
    TFT_SendData(tft, 0x06);
    TFT_SendData(tft, 0x04);

    TFT_SendIndex(tft, 0xC1);
    TFT_SendData(tft, 0xC0);
    TFT_SendData(tft, 0xC2);

    TFT_SendIndex(tft, 0xC2);
    TFT_SendData(tft, 0x0D);
    TFT_SendData(tft, 0x0D);

    TFT_SendIndex(tft, 0xC3);
    TFT_SendData(tft, 0x8D);
    TFT_SendData(tft, 0xEE);

    TFT_SendIndex(tft, 0xC4);
    TFT_SendData(tft, 0x8D);
    TFT_SendData(tft, 0xEE);

    TFT_SendIndex(tft, 0xC5);
    TFT_SendData(tft, 0x00);

    // 数据访问方式
    TFT_SendIndex(tft, 0x36);
    TFT_SendData(tft, 0xC0);

    // 伽马
    TFT_SendIndex(tft, 0xE0);
    TFT_SendData(tft, 0x1B);
    TFT_SendData(tft, 0x21);
    TFT_SendData(tft, 0x10);
    TFT_SendData(tft, 0x15);
    TFT_SendData(tft, 0x2B);
    TFT_SendData(tft, 0x25);
    TFT_SendData(tft, 0x1F);
    TFT_SendData(tft, 0x23);
    TFT_SendData(tft, 0x22);
    TFT_SendData(tft, 0x22);
    TFT_SendData(tft, 0x2B);
    TFT_SendData(tft, 0x37);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, 0x15);
    TFT_SendData(tft, 0x02);
    TFT_SendData(tft, 0x3F);

    TFT_SendIndex(tft, 0xE1);
    TFT_SendData(tft, 0x1A);
    TFT_SendData(tft, 0x20);
    TFT_SendData(tft, 0x0F);
    TFT_SendData(tft, 0x15);
    TFT_SendData(tft, 0x2A);
    TFT_SendData(tft, 0x25);
    TFT_SendData(tft, 0x1E);
    TFT_SendData(tft, 0x23);
    TFT_SendData(tft, 0x23);
    TFT_SendData(tft, 0x22);
    TFT_SendData(tft, 0x2B);
    TFT_SendData(tft, 0x37);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, 0x15);
    TFT_SendData(tft, 0x02);
    TFT_SendData(tft, 0x3F);

    TFT_SendIndex(tft, 0x2C);
    TFT_SendIndex(tft, 0x29); // 开屏
    TFT_Clear(tft, TFT_BLACK);
}

/* ---------- 复位 ---------- */

void TFT_Reset(TFT_Model_t *tft)
{
    if (tft == NULL || tft->rst_pin == NULL)
        return;

    gpio_write(tft->rst_pin, GPIO_Level_Low);
    TFT_DELAY_MS(100);
    gpio_write(tft->rst_pin, GPIO_Level_High);
    TFT_DELAY_MS(50);
}

/* ---------- 背光 ---------- */

void TFT_TurnOff(TFT_Model_t *tft, uint8_t io)
{
    if (tft == NULL || tft->blk_pin == NULL)
        return;

    gpio_write(tft->blk_pin, io ? GPIO_Level_High : GPIO_Level_Low);
}

/* ---------- 指令/数据发送 ---------- */

void TFT_SendIndex(TFT_Model_t *tft, uint8_t reg)
{
    if (tft->dc_pin)
        gpio_write(tft->dc_pin, GPIO_Level_Low); // DC=低 → 指令

    tft_spi_write(tft, &reg, 1);
}

void TFT_SendData(TFT_Model_t *tft, uint8_t data)
{
    if (tft->dc_pin)
        gpio_write(tft->dc_pin, GPIO_Level_High); // DC=高 → 数据

    tft_spi_write(tft, &data, 1);
}

void TFT_Send16Bit(TFT_Model_t *tft, uint16_t data)
{
    uint8_t buf[1] = {(uint8_t)(data >> 8)};

    if (tft->dc_pin)
        gpio_write(tft->dc_pin, GPIO_Level_High);

    tft_spi_write(tft, buf, 1);
    buf[0] = (uint8_t)(data & 0xFF);
    tft_spi_write(tft, buf, 1);
}

void TFT_SendReg(TFT_Model_t *tft, uint8_t addr, uint8_t val)
{
    TFT_SendIndex(tft, addr);
    TFT_SendData(tft, val);
}

/* ---------- 方向 ---------- */

void TFt_SpinScreen(TFT_Model_t *tft, uint8_t dir)
{
    static const uint8_t vals[] = {0xC0, 0xA0, 0x00, 0x60};

    if (dir > 3) dir = 0;

    TFT_SendIndex(tft, 0x36);
    TFT_SendData(tft, vals[dir]);
}

/* ---------- 区域设置 ---------- */

void TFT_SetRegion(TFT_Model_t *tft,
                   uint16_t x1, uint16_t y1,
                   uint16_t x2, uint16_t y2)
{
    TFT_SendIndex(tft, 0x2A);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, x1);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, x2);

    TFT_SendIndex(tft, 0x2B);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, y1);
    TFT_SendData(tft, 0x00);
    TFT_SendData(tft, y2);

    TFT_SendIndex(tft, 0x2C);
}

/* ---------- 清屏 ---------- */

void TFT_Clear(TFT_Model_t *tft, uint16_t color)
{
    TFT_SetRegion(tft, 0, 0, 127, 159);
    TFT_SendIndex(tft, 0x2C);

    for (uint16_t i = 0; i < 128; i++)
        for (uint16_t j = 0; j < 160; j++)
            TFT_Send16Bit(tft, color);
}

/* ---------- 区间填充 ---------- */

void TFT_FullScreen(TFT_Model_t *tft,
                    uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2,
                    uint16_t color)
{
    int count = (x2 - x1 + 1) * (y2 - y1 + 1);

    TFT_SetRegion(tft, x1, y1, x2, y2);

    while (count--)
        TFT_Send16Bit(tft, color);

    TFT_SetRegion(tft, 0, 0, 127, 159);
}

/* ---------- 光标 ---------- */

void TFT_SetCursor(TFT_Model_t *tft, uint16_t x, uint16_t y)
{
    TFT_SetRegion(tft, x, y, x, y);
}

/* ---------- 画点 ---------- */

void TFT_DrawPoint(TFT_Model_t *tft, uint16_t x, uint16_t y, uint16_t color)
{
    TFT_SetCursor(tft, x, y);
    TFT_Send16Bit(tft, color);
}

/* ---------- 画圆 ---------- */

void TFT_DrawCircle(TFT_Model_t *tft,
                    uint16_t cx, uint16_t cy,
                    uint16_t r, uint16_t color)
{
    int16_t a = 0, b = (int16_t)r;
    int16_t c = 3 - 2 * (int16_t)r;

    while (a < b) {
        TFT_DrawPoint(tft, cx + a, cy + b, color);
        TFT_DrawPoint(tft, cx - a, cy + b, color);
        TFT_DrawPoint(tft, cx + a, cy - b, color);
        TFT_DrawPoint(tft, cx - a, cy - b, color);
        TFT_DrawPoint(tft, cx + b, cy + a, color);
        TFT_DrawPoint(tft, cx - b, cy + a, color);
        TFT_DrawPoint(tft, cx + b, cy - a, color);
        TFT_DrawPoint(tft, cx - b, cy - a, color);

        if (c < 0)
            c = c + 4 * a + 6;
        else {
            c = c + 4 * (a - b) + 10;
            b -= 1;
        }
        a += 1;
    }
    if (a == b) {
        TFT_DrawPoint(tft, cx + a, cy + b, color);
        TFT_DrawPoint(tft, cx + a, cy - b, color);
        TFT_DrawPoint(tft, cx - a, cy - b, color);
        TFT_DrawPoint(tft, cx + b, cy + a, color);
        TFT_DrawPoint(tft, cx - b, cy + a, color);
        TFT_DrawPoint(tft, cx + b, cy - a, color);
        TFT_DrawPoint(tft, cx - b, cy - a, color);
    }
}

/* ---------- 画线 (Bresenham) ---------- */

void TFT_DrawLine(TFT_Model_t *tft,
                  uint16_t x0, uint16_t y0,
                  uint16_t x1, uint16_t y1,
                  uint16_t color)
{
    int16_t dx    = (int16_t)x1 - (int16_t)x0;
    int16_t dy    = (int16_t)y1 - (int16_t)y0;
    int16_t x_inc = (dx >= 0) ? 1 : -1;
    int16_t y_inc = (dy >= 0) ? 1 : -1;
    int16_t dx2, dy2, error;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    dx2 = dx << 1;
    dy2 = dy << 1;

    if (dx > dy) {
        error = dy2 - dx;
        for (int16_t i = 0; i <= dx; i++) {
            TFT_DrawPoint(tft, (uint16_t)x0, (uint16_t)y0, color);
            if (error >= 0) {
                error -= dx2;
                y0 += y_inc;
            }
            error += dy2;
            x0 += x_inc;
        }
    } else {
        error = dx2 - dy;
        for (int16_t i = 0; i <= dy; i++) {
            TFT_DrawPoint(tft, (uint16_t)x0, (uint16_t)y0, color);
            if (error >= 0) {
                error -= dy2;
                x0 += x_inc;
            }
            error += dx2;
            y0 += y_inc;
        }
    }
}

/* ---------- 矩形边框 ---------- */

void TFT_box(TFT_Model_t *tft,
             uint16_t x, uint16_t y,
             uint16_t w, uint16_t h,
             uint16_t color)
{
    TFT_DrawLine(tft, x, y, x + w, y, color);
    TFT_DrawLine(tft, x + w, y, x + w, y + h, color);
    TFT_DrawLine(tft, x, y + h, x + w, y + h, color);
    TFT_DrawLine(tft, x, y, x, y + h, color);
}

/* ---------- 矩形边框（预设方案） ---------- */

void TFT_box2(TFT_Model_t *tft,
              uint16_t x, uint16_t y,
              uint16_t w, uint16_t h,
              uint8_t mode)
{
    switch (mode) {
        case 1:
            TFT_DrawLine(tft, x, y, x + w, y, TFT_GRAY0);
            TFT_DrawLine(tft, x + w, y, x + w, y + h, 0x2965);
            TFT_DrawLine(tft, x, y + h, x + w, y + h, 0x2965);
            TFT_DrawLine(tft, x, y, x, y + h, TFT_GRAY0);
            break;
        case 2:
            TFT_DrawLine(tft, x, y, x + w, y, TFT_RED);
            TFT_DrawLine(tft, x + w, y, x + w, y + h, TFT_YELLOW);
            TFT_DrawLine(tft, x, y + h, x + w, y + h, TFT_YELLOW);
            TFT_DrawLine(tft, x, y, x, y + h, TFT_RED);
            break;
        case 3:
            TFT_DrawLine(tft, x, y, x + w, y, TFT_WHITE);
            TFT_DrawLine(tft, x + w, y, x + w, y + h, TFT_GRAY0);
            TFT_DrawLine(tft, x, y + h, x + w, y + h, TFT_GRAY0);
            TFT_DrawLine(tft, x, y, x, y + h, TFT_WHITE);
            break;
        case 0:
        default:
            TFT_DrawLine(tft, x, y, x + w, y, TFT_GREEN);
            TFT_DrawLine(tft, x + w, y, x + w, y + h, TFT_PINK);
            TFT_DrawLine(tft, x, y + h, x + w, y + h, TFT_PINK);
            TFT_DrawLine(tft, x, y, x, y + h, TFT_GREEN);
            break;
    }
}

/* ---------- 按钮按下/抬起 ---------- */

void ButtonDown(TFT_Model_t *tft,
                uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2)
{
    TFT_DrawLine(tft, x1, y1, x2, y1, TFT_GRAY2);
    TFT_DrawLine(tft, x1, y1, x2, y1, TFT_GRAY1);
    TFT_DrawLine(tft, x1, y1, x1, y2, TFT_GRAY2);
    TFT_DrawLine(tft, x1, y1, x1, y2, TFT_GRAY1);
    TFT_DrawLine(tft, x1, y2, x2, y2, TFT_WHITE);
    TFT_DrawLine(tft, x2, y1, x2, y2, TFT_WHITE);
}

void ButtonUp(TFT_Model_t *tft,
              uint16_t x1, uint16_t y1,
              uint16_t x2, uint16_t y2)
{
    TFT_DrawLine(tft, x1, y1, x2, y1, TFT_WHITE);
    TFT_DrawLine(tft, x1, y1, x1, y2, TFT_WHITE);
    TFT_DrawLine(tft, x1, y2, x2, y2, TFT_GRAY1);
    TFT_DrawLine(tft, x1, y2, x2, y2, TFT_GRAY2);
    TFT_DrawLine(tft, x2, y1, x2, y2, TFT_GRAY1);
    TFT_DrawLine(tft, x2, y1, x2, y2, TFT_GRAY2);
}

/* ---------- 图片 ---------- */

void TFT_ShowImage(TFT_Model_t *tft,
                   uint16_t x, uint16_t y,
                   uint16_t length, uint16_t width,
                   const unsigned char *p)
{
    TFT_SetRegion(tft, x, y, x + length - 1, y + width - 1);

    if (tft->dc_pin) gpio_write(tft->dc_pin, GPIO_Level_High);
    spi_cs_select(tft->spi);
    for (int i = 0; i < length * width; i++) {
        uint8_t lo     = p[2 * i];
        uint8_t hi     = p[2 * i + 1];
        uint8_t buf[2] = {hi, lo};
        spi_write(tft->spi, buf, 2);
    }
    spi_cs_deselect(tft->spi);

    TFT_SetRegion(tft, 0, 0, 127, 159);
}

/* ---------- 字符 ---------- */

void TFT_ShowChar(TFT_Model_t *tft,
                  uint8_t x, uint8_t y,
                  uint16_t fc, uint16_t bc, char c)
{
    int k = (c - 32) * 16;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (asc[k + i] & (0x80 >> j))
                TFT_DrawPoint(tft, x + j, y + i, fc);
            else
                TFT_DrawPoint(tft, x + j, y + i, bc);
        }
    }
}

/* ---------- 字符串 ---------- */

void TFT_ShowString(TFT_Model_t *tft,
                    uint8_t x, uint8_t y,
                    uint16_t fc, uint16_t bc, char *c)
{
    int t = strlen(c);
    for (int i = 0; i < t; i++) {
        if (x >= 128) {
            x = 0;
            y += 16;
        }
        TFT_ShowChar(tft, x, y, fc, bc, c[i]);
        x += 8;
    }
}

/* ---------- 数字 ---------- */

void TFT_ShowNumber(TFT_Model_t *tft,
                    uint8_t x, uint8_t y,
                    uint16_t fc, uint16_t bc,
                    long long num)
{
    uint8_t k = 0;
    char s[20];
    long long t = num;

    while (t) {
        t /= 10;
        k++;
    }

    if (num < 0) {
        s[0]     = '-';
        s[k + 1] = '\0';
        num *= -1;
    } else {
        s[k] = '\0';
        k -= 1;
    }
    while (num) {
        s[k--] = '0' + num % 10;
        num /= 10;
    }
    TFT_ShowString(tft, x, y, fc, bc, s);
}

/* ---------- 中文 ---------- */

void TFT_ShowChinese(TFT_Model_t *tft,
                     uint8_t x, uint8_t y,
                     uint16_t fc, uint16_t bc, char *c)
{
    int t = strlen(c);
    for (int n = 0; n < t; n++) {
        if (c[n] > 31 && c[n] < 127) {
            if (x + 8 >= 128) {
                x = 0;
                y += 16;
            }
            TFT_ShowChar(tft, x, y, fc, bc, c[n]);
            x += 8;
            continue;
        }

        char tem[4];
        tem[0] = c[n];
        tem[1] = c[n + 1];
        tem[2] = c[n + 2];
        tem[3] = '\0';
        int k  = map(tem);

        if (k == -1) {
            if (x + 8 >= 128) {
                x = 0;
                y += 16;
            }
            TFT_ShowChar(tft, x, y, TFT_YELLOW, TFT_RED, '?');
            x += 8;
            n += 2;
            continue;
        }

        if (x + 16 >= 128) {
            x = 0;
            y += 16;
        }

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                if (chinese_font[k * 32 + 2 * i] & (0x80 >> j))
                    TFT_DrawPoint(tft, x + j, y + i, fc);
                else
                    TFT_DrawPoint(tft, x + j, y + i, bc);
            }
            for (int j = 0; j < 8; j++) {
                if (chinese_font[k * 32 + 2 * i + 1] & (0x80 >> j))
                    TFT_DrawPoint(tft, x + j + 8, y + i, fc);
                else
                    TFT_DrawPoint(tft, x + j + 8, y + i, bc);
            }
        }
        x += 16;
        n += 2;
    }
}

/* ---------- map（查找汉字在样例数组中的位置） ---------- */

int map(char *c)
{
    int l1 = strlen(font_sample);
    for (int i = 0; i < l1; i += 3) {
        if (font_sample[i] == c[0] &&
            font_sample[i + 1] == c[1] &&
            font_sample[i + 2] == c[2])
            return i / 3;
    }
    return -1;
}
