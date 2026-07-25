#pragma once

/*
 * 设计说明：
 *   SDIO 的物理引脚是固定的（STM32F4 上为 PC8-12 + PD2），
 *   因此本抽象不处理引脚映射 —— 这由 CubeMX 的 HAL_SD_MspInit()
 *   生成代码处理。
 *
 *   本层抽象的是传输接口。
 *   所有 sdio_read_blocks / sdio_write_blocks 都是同步的：
 *     - Polling 模式：HAL 内部阻塞直到完成
 *     - IT/DMA 模式：在 chip 层内部忙等 busy 位清零（ISR 中清除）
 *     调用者角度来看，函数返回时传输已经完成。
 *
 *   此外必须注意：
 *   在 DMA 模式下：
 *     - buf 必须 4 字节对齐（SDIO FIFO 是 32 位宽）
 *     - buf 不能位于 CCMRAM（DMA 无法访问 CCM）
 *
 * 使用示例：
 *
 *   #include "bsp_sdio.h"
 *
 *   extern SD_HandleTypeDef hsd;      // CubeMX 生成
 *
 *   SDIO_Model_t sd;
 *   sdio_init(&sd, (SDIO_Handle_t)&hsd, &(SDIO_Config_t){
 *       .mode      = SDIO_Mode_DMA,
 *       .wide_bus  = 1,              // 4Bit
 *   });
 *
 *   __ALIGN_BEGIN uint8_t buf[512] __ALIGN_END;
 *   sdio_read_blocks(&sd, buf, 0, 1);  // 阻塞直到完成
 */

/* TODO: SDIO 使用 DMA 时的四字节对齐问题可以参考安富莱电子的解决方案（即使用 FIFO） */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    SDIO_Err_Ok           = 0,
    SDIO_Err_Param        = 1, /**< 参数错误（NULL 指针等） */
    SDIO_Err_Timeout      = 2, /**< 超时（仅 Polling 模式） */
    SDIO_Err_Busy         = 3, /**< 上一次传输仍在进行 */
    SDIO_Err_DMA          = 4, /**< DMA 传输错误 */
    SDIO_Err_Generic      = 5, /**< HAL 返回 HAL_ERROR */
    SDIO_Err_NoCriticalCB = 6, /**< 没有提供进/出临界回调 */
    SDIO_Err_CardOut      = 7, /**< 卡片未插入 */
} sdio_err_t;

typedef enum {
    SDIO_Mode_Polling = 0, /**< 阻塞轮询 */
    SDIO_Mode_IT      = 1, /**< 中断，chip 层内部等 busy 清零 */
    SDIO_Mode_DMA     = 2, /**< DMA，chip 层内部等 busy 清零 */
} sdio_mode_t;

typedef void *sdio_handle_t;

typedef struct {
    sdio_mode_t mode;     /**< 传输模式 */
    uint8_t wide_bus : 1; /**< 0=1Bit, 1=4Bit */
} sdio_cfg_t;

typedef struct {
    sdio_cfg_t config;
    sdio_handle_t handle; /**< SD_HandleTypeDef*（CubeMX 初始化） */
    /* 卡片信息，sdio_init 时由 HAL_SD_GetCardInfo 填充 */
    uint32_t block_size;  /**< 逻辑块大小（字节），通常 512 */
    uint32_t block_count; /**< 总块数 */

    /**
     * @brief 进入临界区/退出临界区的回调
     *
     * 考虑到 sd 卡读写时序较快，且当软件处理速率无法跟上硬件时会触发 RX_OVERRUN
     * 因此有必要在读写操作时进入临界区以保护 sd 卡优先正常读写，将其变为原子操作
     *
     * 对于裸机环境，可使用 `__disable_irq` 和 `__enable_irq`
     * 对于 OS 环境，则根据对应的 OS 提供的进出临界区的函数来使用
     * 无论使用什么函数，都要保证 sd 卡读写的原子性，以及不可中断
     *
     * @warning 必须提供！
     */
    void (*enter_critical_cb)(void);
    void (*exit_critical_cb)(void);

    /* 卡片检测回调，返回卡片插入状态 */
    bool (*sdcard_det_cb)(void);
} sdio_t;

/**
 * 初始化 SDIO 模型。
 *
 * @param m           SDIO 模型指针
 * @param hal_handle  SD_HandleTypeDef*
 * @param cfg         配置（传输模式、时钟分频、总线宽度）
 */
sdio_err_t sdio_init(sdio_t *m, sdio_handle_t hal_handle,
                     const sdio_cfg_t *cfg);

/**
 * 读取块数据。
 *
 * 无论哪种模式，返回时传输已完成。
 *
 * @param m       SDIO 模型
 * @param buf     缓冲区（DMA 模式需 4 字节对齐、不可在 CCMRAM）
 * @param sector  起始扇区（LBA）
 * @param count   扇区数
 */
sdio_err_t sdio_read_blocks(sdio_t *m, uint8_t *buf,
                            uint32_t sector, uint32_t count);

/**
 * 写入块数据。
 *
 * 无论哪种模式，返回时传输已完成。
 *
 * @param m       SDIO 模型
 * @param buf     缓冲区（DMA 模式需 4 字节对齐、不可在 CCMRAM）
 * @param sector  起始扇区（LBA）
 * @param count   扇区数
 */
sdio_err_t sdio_write_blocks(sdio_t *m, const uint8_t *buf,
                             uint32_t sector, uint32_t count);

/**
 * 擦除块操作
 *
 * @param m       SDIO 模型
 * @param sector  起始扇区（LBA）
 * @param count   扇区数
 */
sdio_err_t sdio_erase_blocks(sdio_t *m, uint32_t sector, uint32_t count);

/**
 * 获取卡片信息。
 */
void sdio_get_info(sdio_t *m, uint32_t *block_size, uint32_t *block_count);

/**
 * 检查 SD 卡是否插入。
 */
static inline bool sdio_is_card_inserted(sdio_t *m)
{
    if (m == NULL || m->sdcard_det_cb == NULL)
        return false;
    return m->sdcard_det_cb();
}
