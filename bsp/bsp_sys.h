#pragma once

/* =================================================
 * 板级系统层，需要在 chip 层中实现一些 API
 * ================================================= */

#include <stdint.h>
#include <stddef.h>
/* 用于放弃某个变量，常用于带参函数的占位符 */
#define GIVEUP(x) (void)(x)
/* 断言宏，当条件不满足时执行 actions 操作 */
#define ASSERT(cond, actions) \
    if (!(cond)) { actions; }
/* 设置状态宏，用于设置结构体中的状态位 */
#define SET_STATE(state, bit) ((state) |= (1 << (bit)))
/* 清除状态宏，用于清除结构体中的状态位 */
#define CLEAR_STATE(state, bit) ((state) &= ~(1 << (bit)))

#define ERROR_LOG(msg, errcode) \
    "|- File %s -|- Line %d -|\tError: %s (errcode: %d)\n", __FILE__, __LINE__, msg, errcode
/*
 * 错误处理宏，当 errcode 不等于 okcode 时，
 * use_errlog 决定是否打印错误日志，
 * 并根据是否出错而执行特定操作
 */
#define HANDLE_ERROR(                                                 \
    errcode, okcode, use_errlog, msg, acts_when_failed, acts_when_ok) \
    if (errcode != okcode) {                                          \
        if (use_errlog) printf(ERROR_LOG(msg, errcode));              \
        acts_when_failed;                                             \
    } else {                                                          \
        acts_when_ok;                                                 \
    }

/*
 * 初始化一些相关的系统外设，如调试串口等
 *
 * \warning:
 *  目前已使用外设:
 *  - `huart1`
 *      用于重定向 printf 等函数，作为系统日志输出
 */
void sys_chip_init(void);

uint32_t chip_get_tick(void);

/* 以下的延时为阻塞式的 */

void chip_delay_ms(uint32_t ms);

/* 以下的延时为非阻塞式的，通常由 RTOS 层提供 */

uint8_t chip_till_max_delay(uint32_t start_time, uint32_t max_delay);
void os_delay_ms(uint32_t ms);
void os_delay_until(uint32_t *prv_wake_time, uint32_t ms);

/* 以下为特定内核的单片机才有的外设，如果没有则移除宏定义 */
/* NOTE: 此处暂时没想到如何抽象 DWT 外设 */
#define USE_DWT_DELAY

#ifdef USE_DWT_DELAY

/* 使用 dwt 模块之前一定需要先初始化 */

void dwt_init(void);
void dwt_delay_us(uint32_t us);
void dwt_delay_ms(uint32_t ms);

#endif
