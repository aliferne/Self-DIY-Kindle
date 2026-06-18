# 开发进度

- [x] 软件 SPI 写完
- [x] 屏幕驱动移完
- [x] 测试屏幕
- [ ] 移植 LVGL

目前还遇到个图片刷不出来的问题，不过先不管这个，电子阅读器本来也不在乎图片

再造一层简单的封装就可以开始 FatFs 这个工作了，不然干不完了

---

屏幕改为 1.8 inch 的 ST7735S 驱动的 TFT 屏幕，一晚上完成基本程序了

---

LVGL 显示驱动已完全移植完毕

在使用 ST7735S 时实际上借用的是原来驱动直接提供的 `TFT_*` 的代码，但是感觉不好，如果换屏幕了实际上不好迁移，因此还是需要抽象层。

创建 disp_drv.h 头文件，要求驱动实现这个头文件内部的函数，从而保证在上层调用的都是同样的 API，只是说底层重写还是比较麻烦的，但至少不用到处改。

此外突然发现其实 lvgl 已经为我们造好兼容层了，即只需要重写 `lv_port_*.c` 文件就可以等效于我的 `srv` 层实现，所以这里我把原来的 `display_srv` 文件删了，反正 lvgl 是不可能换的，定死了也无所谓。只是屏幕可能会换，所以兼容层需要对屏幕做，正如之前的 bsp/chip 层一样。

此外是关于显示屏刷新的性能问题：

我一开始的代码是：

```c
/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (disp_flush_enabled) {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

        /* TODO: 可以想想如何优化速度 */
        int32_t x;
        int32_t y;
        for (y = area->y1; y <= area->y2; y++) {
            for (x = area->x1; x <= area->x2; x++) {
                /*Put a pixel to the display. For example:*/
                /*put_px(x, y, *color_p)*/
                TFT_DrawPoint(&tft, x, y, color_p->full);
                color_p++;
            }
        }
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
```

它是点刷的，实际上如果这么去刷新的话速度会非常慢，基本要个一秒钟左右（不过也比墨水屏快了…）

优化方案其实有两个，我目前想到的，一个是点刷改局刷，降低函数反复调用的开销，另一个是硬件 SPI + DMA 等提速。

改局刷是比较快的，其原理就是以空间换时间，将一大堆数据存好了直接一次性刷出去，避免反复片选/切DC等导致的性能损耗：

```c
/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (disp_flush_enabled) {
        /*
         * 这种刷新方法确实速度快了不少，
         * 不过需要启用 `LV_COLOR_16_SWAP = 1`，
         * 原因下面讲
         * 之后可以考虑启用 SPI DMA
         */
        int32_t w = area->x2 - area->x1 + 1;
        int32_t h = area->y2 - area->y1 + 1;

        display_write_pixels(
            &display, area->x1, area->y1, w, h,
            (const uint16_t *)color_p);
    }
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
```

由于批量刷新最省事的方法自然是直接把 `uint16_t *` 转成 `uint8_t *`，那么打个比方： `{0xABFF}`，就会变为 `{0xFF, 0xAB}`(因为 Cortex-M 是小端序的)，那么就会先发送低 8 位，然而驱动渲染颜色需要先发高八位，此时就会导致颜色不对头的问题，所以需要在 LVGL 里面开一下 SWAP 的宏调换一下顺序。

```c
// LCD写GRAM(批量)
// RGB_Codes:颜色缓冲区
// len:像素点数
static void LCD_WriteMultiRAM(uint16_t *RGB_Codes, uint32_t len)
{
    ASSERT_FAIL(lcd_src == NULL || lcd_src->dc_pin == NULL, return);
    gpio_write(lcd_src->dc_pin, GPIO_Level_High);
    spi_cs_select(lcd_src->spi);
    spi_write(lcd_src->spi, (uint8_t *)RGB_Codes, len * sizeof(RGB_Codes[0]));
    spi_cs_deselect(lcd_src->spi);
}
```

因为这里用了批量刷新会导致需要 SWAP，那么在没有 LVGL 支持的情况下也就不难想到到时候画图如果批量操作的话颜色肯定是不对的。

不过我的评价是反正都移植 LVGL 了，那就这样吧，怎么方便怎么来

SPI + DMA 还没改，现在已经很好用了，后面再说

此外突然发现 LVGL 好像暂时无法显示中文，会显示成方块字，亟待解决

# 遇到的神人问题

ui_task 疑似炸堆栈了，检查 HFSR 等寄存器发现触发总线错误，STKERR 被置 1

还真是， SB DeepSeek 给我在任务里面开了个 30000 大小的数组，整个 UI Task 直接炸掉了
