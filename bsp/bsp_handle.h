#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * 用于错误处理，断言语句等比较常用的操作
 */

/* 设置 bit 宏 */
#define BIT_ON(reg, bit) ((reg) |= (bit))
/* 清除 bit 宏 */
#define BIT_OFF(reg, bit) ((reg) &= ~(bit))
/* 此 bit 是否被设置 */
#define IS_BIT_ON(reg, bit) ((reg) & (bit))

#define LEN(arr)            (sizeof(arr) / sizeof((arr)[0]))

/* 错误处理相关 ------------------------------------- */

/* 用于放弃某个变量，常用于带参函数的占位符 */
#define GIVEUP(x) (void)(x)

/* 以下为 GCC 特有的 `__attribute__` 语法 */
#ifdef __GNUC__
    /* 取消自动内存对齐 */
    #define __PACKED            __attribute__((packed))
    /* 强制声明某个可能未使用函数/变量为已使用，避免编译器优化 */
    #define __USED              __attribute__((used))
    /* 指定数据存储段为 sec_name */
    #define __SECTION(sec_name) __attribute__((section(sec_name)))
    /* 声明某个变量或函数未被使用，避免编译器警告 */
    #define __NOT_USED          __attribute__((unused))
    /* 声明某个函数已过时，并提供提示信息 */
    #define __DEPRECATED(msg)   __attribute__((deprecated(msg)))
    /* 强制按照 x 字节对齐 */
    #define __ALIGNED(x)        __attribute__((aligned(x)))
    /* 强制内联 */
    #define __ALWAYS_INLINE     __attribute__((always_inline))
    /* 声明函数不会返回 */
    #define __NO_RETURN         __attribute__((noreturn))
#else
    #define __PACKED
    #define __USED
    #define __SECTION(sec_name)
    #define __NOT_USED
    #define __DEPRECATED(msg)
    #define __ALIGNED(x)
    #define __ALWAYS_INLINE
    #define __NO_RETURN
#endif

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
