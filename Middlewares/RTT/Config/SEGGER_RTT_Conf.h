/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
*                                                                    *
*        SEGGER RTT * Real Time Transfer for embedded targets        *
*                  https://github.com/SEGGERMicro/RTT                *
*                                                                    *
**********************************************************************

---------------------------END-OF-HEADER------------------------------
Purpose : User configuration file for RTT.
          For available configuration,
          refer to SEGGER_RTT_ConfDefaults.h.

----------------------------------------------------------------------
*/

#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H


/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/**
 * @ref https://www.cnblogs.com/time-light/p/19972319
 */

#define BUFFER_SIZE_UP                        1024  // MCU -> PC
#define BUFFER_SIZE_DOWN                      16    // PC -> MCU
#define SEGGER_RTT_PRINTF_BUFFER_SIZE         64    // 格式化临时缓冲区

#endif
/*************************** End of file ****************************/
