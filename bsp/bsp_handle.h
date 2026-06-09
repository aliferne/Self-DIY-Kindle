#pragma once

/*
 * 用于错误处理，断言语句等比较常用的操作
 */

#define LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

/* 错误处理相关 ------------------------------------- */

/* 用于放弃某个变量，常用于带参函数的占位符 */
#define GIVEUP(x) (void)(x)
/* 成功断言宏，当条件满足时执行 actions 操作 */
#define ASSERT(cond, actions) \
    do {                      \
        if ((cond)) {         \
            actions;          \
        }                     \
    } while (0)
/* 失败断言宏，当条件不满足时执行 actions 操作 */
#define ASSERT_FAIL(cond, actions) \
    do {                           \
        if (!(cond)) {             \
            actions;               \
        }                          \
    } while (0)
/* 设置状态宏，用于设置结构体中的状态位 */
#define SET_STATE(val, bit) ((val) |= (1 << (bit)))
/* 清除状态宏，用于清除结构体中的状态位 */
#define CLEAR_STATE(val, bit) ((val) &= ~(1 << (bit)))
/* 检验是否为此状态 */
#define IS_STATE(val, bit) ((val) & (1 << (bit)))

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
