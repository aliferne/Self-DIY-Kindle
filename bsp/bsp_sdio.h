#pragma once

/*
 * ============================================================
 * bsp_sdio.h  —  SDIO 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 *
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

#include <stdint.h>
#include <stddef.h>

/* ============================================================
 * 枚举与类型定义
 * ============================================================ */

typedef enum {
    SDIO_Err_Ok       = 0,
    SDIO_Err_Param    = 1,   /**< 参数错误（NULL 指针等） */
    SDIO_Err_Timeout  = 2,   /**< 超时（仅 Polling 模式） */
    SDIO_Err_Busy     = 3,   /**< 上一次传输仍在进行 */
    SDIO_Err_DMA      = 4,   /**< DMA 传输错误 */
    SDIO_Err_Generic  = 5,   /**< HAL 返回 HAL_ERROR */
} SDIO_Err_t;

typedef enum {
    SDIO_Mode_Polling = 0,   /**< 阻塞轮询 */
    SDIO_Mode_IT      = 1,   /**< 中断，chip 层内部等 busy 清零 */
    SDIO_Mode_DMA     = 2,   /**< DMA，chip 层内部等 busy 清零 */
} SDIO_Mode_t;

/** HAL 句柄的透明包装（chip 层内部转型为 SD_HandleTypeDef*） */
typedef void *SDIO_Handle_t;

/* ============================================================
 * 配置结构体
 * ============================================================ */

typedef struct {
    SDIO_Mode_t mode;         /**< 传输模式 */
    uint8_t wide_bus : 1;    /**< 0=1Bit, 1=4Bit */
} SDIO_Config_t;

/* ============================================================
 * SDIO 模型
 * ============================================================ */

typedef struct {
    SDIO_Config_t config;
    SDIO_Handle_t handle;         /**< SD_HandleTypeDef*（CubeMX 初始化） */
    volatile uint8_t busy  : 1;   /**< 1=正在传输 */
    volatile uint8_t error : 1;   /**< 1=上一次传输出错 */

    /* 卡片信息，sdio_init 时由 HAL_SD_GetCardInfo 填充 */
    uint32_t block_size;          /**< 逻辑块大小（字节），通常 512 */
    uint32_t block_count;         /**< 总块数 */
} SDIO_Model_t;

/* ============================================================
 * API
 * ============================================================ */

/**
 * 初始化 SDIO 模型。
 *
 * @param m           SDIO 模型指针
 * @param hal_handle  SD_HandleTypeDef*，必须已由 MX_SDIO_SD_Init() 初始化
 * @param cfg         配置（传输模式、时钟分频、总线宽度）
 */
SDIO_Err_t sdio_init(SDIO_Model_t *m, SDIO_Handle_t hal_handle,
                     const SDIO_Config_t *cfg);

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
SDIO_Err_t sdio_read_blocks(SDIO_Model_t *m, uint8_t *buf,
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
SDIO_Err_t sdio_write_blocks(SDIO_Model_t *m, const uint8_t *buf,
                             uint32_t sector, uint32_t count);

/**
 * 擦除块（Polling 模式，同步阻塞）。
 *
 * 擦除操作是低频操作，使用 Polling 模式即可。
 *
 * @param m       SDIO 模型
 * @param sector  起始扇区（LBA）
 * @param count   扇区数
 */
SDIO_Err_t sdio_erase_blocks(SDIO_Model_t *m, uint32_t sector, uint32_t count);

/**
 * 查询是否正在传输。
 */
uint8_t sdio_is_busy(SDIO_Model_t *m);

/**
 * 获取卡片信息。
 */
void sdio_get_info(SDIO_Model_t *m, uint32_t *block_size, uint32_t *block_count);
