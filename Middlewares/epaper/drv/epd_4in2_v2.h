/*****************************************************************************
* | File      	:   EPD_4in2_V2.h
* | Author      :   Waveshare team
* | Function    :   4.2inch e-paper V2
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2023-09-11
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef _EPD_4IN2_V2_H_
#define _EPD_4IN2_V2_H_

// Display resolution
#define EPD_4IN2_V2_WIDTH  400
#define EPD_4IN2_V2_HEIGHT 300

#define IMAGE_SIZE         (((EPD_4IN2_V2_WIDTH % 8 == 0) \
                         ? (EPD_4IN2_V2_WIDTH / 8)        \
                         : (EPD_4IN2_V2_WIDTH / 8 + 1)) * \
                    EPD_4IN2_V2_HEIGHT)

#include "DEV_Config.h"
#include "epaper.h"

typedef enum {
    Seconds_1_5S = 0, /* 1.5s */
    Seconds_1S   = 1, /* 1s */
} EPD_FastInitTime_t;

EPaper_Err_t EPD_4IN2_V2_Init(EPaper_Model_t *epd);
EPaper_Err_t EPD_4IN2_V2_Init_Fast(EPaper_Model_t *epd, UBYTE Mode);
EPaper_Err_t EPD_4IN2_V2_Init_4Gray(EPaper_Model_t *epd);
EPaper_Err_t EPD_4IN2_V2_Clear(EPaper_Model_t *epd);
EPaper_Err_t EPD_4IN2_V2_Display(EPaper_Model_t *epd, UBYTE *Image);
EPaper_Err_t EPD_4IN2_V2_Display_Fast(EPaper_Model_t *epd, UBYTE *Image);
EPaper_Err_t EPD_4IN2_V2_Display_4Gray(EPaper_Model_t *epd, UBYTE *Image);
EPaper_Err_t EPD_4IN2_V2_PartialDisplay(
    EPaper_Model_t *epd, UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void EPD_4IN2_V2_Sleep(EPaper_Model_t *epd);

#endif
