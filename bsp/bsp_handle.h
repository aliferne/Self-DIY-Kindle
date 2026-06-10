#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * 用于错误处理，断言语句等比较常用的操作
 */

/* 设置 bit 宏 */
#define SET_REG(reg, bit) ((reg) |= (bit))
/* 清除 bit 宏 */
#define CLEAR_REG(reg, bit) ((reg) &= ~(bit))
/* 此 bit 是否被设置 */
#define IS_REG_SET(reg, bit) ((reg) & (bit))

#define LEN(arr)             (sizeof(arr) / sizeof((arr)[0]))

/* 错误处理相关 ------------------------------------- */

/* 用于放弃某个变量，常用于带参函数的占位符 */
#define GIVEUP(x) (void)(x)
/* 成功断言宏，断言 cond 一定为真，否则执行 actions 操作 */
#define ASSERT(cond, actions) \
    do {                      \
        if (!(cond)) {        \
            actions;          \
        }                     \
    } while (0)
/* 失败断言宏，断言 cond 一定为假，否则执行 actions 操作 */
#define ASSERT_FAIL(cond, actions) \
    do {                           \
        if ((cond)) {              \
            actions;               \
        }                          \
    } while (0)

/* 是否打印日志 */
#define USE_ERR_LOG 0
#if USE_ERR_LOG == 1
#define ERROR_LOG(msg) \
    "|- File %s -|- Line %d -|\tError: %s\n", __FILE__, __LINE__, msg
#define ERR_PRINT(msg) printf(ERROR_LOG(msg));
#else
#define ERR_PRINT(msg)
#endif
/* 错误处理宏，当满足 errcond 时，执行特定操作 */
#define HANDLE_ERROR(errcond, msg, acts_when_failed) \
    ASSERT(errcond,                                  \
           ERR_PRINT(msg);                           \
           acts_when_failed)
