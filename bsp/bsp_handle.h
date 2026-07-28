#pragma once

#include <stdbool.h>
#include "rtt_srv.h"

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
#include "cmsis_gcc.h"
/* 声明为弱函数 */
#define __WEAK __attribute__((weak))
/* 强制声明某个可能未使用函数/变量为已使用，避免编译器优化 */
#define __USED __attribute__((used))
/* 指定数据存储段为 sec_name */
#define __SECTION(sec_name) __attribute__((section(sec_name)))
/* 声明某个变量或函数未被使用，避免编译器警告 */
#define __NOT_USED __attribute__((unused))
/* 声明某个函数已过时，并提供提示信息 */
#define __DEPRECATED(msg) __attribute__((deprecated(msg)))
/* 在调用某个函数时产生警告信息（给出提示） */
#define __WARNING(msg) __attribute__((warning(msg)))
/* 在调用某个函数时强制报错（给出提示） */
#define __ERROR(msg) __attribute__((error(msg)))
/* 强制按照 x 字节对齐 */
#define __ALIGNED(x) __attribute__((aligned(x)))
/* 强制内联 */
#define __ALWAYS_INLINE __attribute__((always_inline))
#else
#define __WEAK
#define __PACKED
#define __USED
#define __SECTION(sec_name)
#define __NOT_USED
#define __DEPRECATED(msg)
#define __WARNING(msg)
#define __ERROR(msg)
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

/*
 * 各种输出日志的方法，
 * 这里统一换成 LOG_XXX 的形式，
 * 后续如果没了 RTT 可以换成其他的，
 * 这里暂时不实现
 */

#ifdef __RTT_SRV_H__
#define LOG_INFO(msg, ...)     RTT_LOG_INFO(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)    RTT_LOG_DEBUG(msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)     RTT_LOG_WARN(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)    RTT_LOG_ERROR(msg, ##__VA_ARGS__)
#define LOG_NORMAL(msg, ...)   RTT_LOG_NORMAL(msg, ##__VA_ARGS__)
#define LOG_GET_CHAR()         RTT_GET_CHAR()
#define LOG_GET_STR(buf, size) RTT_GET_STR((buf), (size))
/* 这个是 RTT 独有的，用其他输出方式的话应该不需要实现 */
#define LOG_CLEAR_SCREEN() rtt_clear()
#else
#define LOG_INFO(msg, ...)
#define LOG_DEBUG(msg, ...)
#define LOG_WARN(msg, ...)
#define LOG_ERROR(msg, ...)
#define LOG_NORMAL(msg, ...)
#define LOG_GET_CHAR()
#define LOG_GET_STR(buf, size)
#define LOG_CLEAR_SCREEN()
#endif

/* 当不满足 cond 时，打印日志并执行特定操作 */
#define LOG_WHEN_FAILED(cond, acts_when_failed, msg, ...) \
    ASSERT_FAIL(cond,                                     \
                LOG_ERROR(msg, ##__VA_ARGS__);            \
                acts_when_failed)
