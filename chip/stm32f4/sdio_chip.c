/*
 * chip/stm32f4/sdio_chip.c
 *
 * 本文件为 SDIO 的芯片层实现
 *
 * 该文件假定单片机只读写一个 SD 卡，即只存在一个 SDIO 实例
 * 封装了 HAL SDIO 驱动的初始化和读写操作，并将模式分为：
 *   - Polling 模式：阻塞等待传输完成
 *   - IT 模式：立即返回，等待 ISR 清 busy 位
 *   - DMA 模式：使用 DMA 传输
 */

#include "bsp_sdio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sd.h"

#define SDIO_TIMEOUT 2500

/* 假定芯片只使用一块 SD 卡，因此只需要单例 */
static SDIO_Model_t *s_active = NULL;

SDIO_Err_t sdio_init(SDIO_Model_t *m, SDIO_Handle_t handle,
                     const SDIO_Config_t *cfg)
{
    if (m == NULL || handle == NULL || cfg == NULL)
        return SDIO_Err_Param;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)handle;

    m->handle = handle;
    m->config = *cfg;
    m->busy   = 0;
    m->error  = 0;

    /* 获取卡片信息 */
    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(h, &info) != HAL_OK) {
        m->block_size  = 512;
        m->block_count = 0;
    } else {
        /* 一般来说固定为 512 */
        m->block_size = info.BlockSize;
        /* 这个则视存储卡内存大小而定 */
        m->block_count = info.BlockNbr;
    }

    /* 配置总线宽度 */
    if (cfg->wide_bus)
        HAL_SD_ConfigWideBusOperation(h, SDIO_BUS_WIDE_4B);
    else
        HAL_SD_ConfigWideBusOperation(h, SDIO_BUS_WIDE_1B);

    s_active = m;

    return SDIO_Err_Ok;
}

static void wait_for_completion(SDIO_Model_t *m)
{
    /* 等待传输完成 */
    if (m->config.mode == SDIO_Mode_Polling)
        return;

    while (m->busy) {
        /* FreeRTOS 调度器运行其他任务，ISR 返回后本任务继续 */
    }
}

/* ============================================================
 * 内部辅助：等待 SD 卡进入 Transfer State
 *
 * HAL 要求在每次读写操作前检查卡状态。此函数自旋等待
 * 直到卡回到可传输状态，超时返回 SDIO_Err_Timeout。
 *
 * 超时值复用 SDIO_TIMEOUT（5000ms）。
 * ============================================================ */

static SDIO_Err_t wait_card_ready(SDIO_Model_t *m)
{
    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    uint32_t deadline   = HAL_GetTick() + SDIO_TIMEOUT;
    uint32_t tick       = HAL_GetTick();

    while (HAL_SD_GetState(h) != HAL_SD_STATE_READY) {
        if (tick >= deadline)
            return SDIO_Err_Timeout;

        tick = HAL_GetTick();
    }
    return SDIO_Err_Ok;
}

SDIO_Err_t sdio_read_blocks(SDIO_Model_t *m, uint8_t *buf,
                            uint32_t sector, uint32_t count)
{
    if (m == NULL || buf == NULL)
        return SDIO_Err_Param;

    if (m->busy)
        return SDIO_Err_Busy;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret;

    m->busy  = 1;
    m->error = 0;

    switch (m->config.mode) {
        case SDIO_Mode_Polling: {
            uint32_t timeout = count * 1000;
            if (timeout < SDIO_TIMEOUT)
                timeout = SDIO_TIMEOUT;

            hal_ret = HAL_SD_ReadBlocks(h, buf, sector, count, timeout);
            m->busy = 0;

            if (hal_ret == HAL_TIMEOUT)
                return SDIO_Err_Timeout;
            if (hal_ret != HAL_OK) {
                m->error = 1;
                return SDIO_Err_Generic;
            }
            return SDIO_Err_Ok;
        }

        case SDIO_Mode_IT:
            hal_ret = HAL_SD_ReadBlocks_IT(h, buf, sector, count);
            if (hal_ret != HAL_OK) {
                m->busy  = 0;
                m->error = 1;
                return SDIO_Err_Generic;
            }

            wait_for_completion(m);
            return m->error ? SDIO_Err_Generic : SDIO_Err_Ok;

        case SDIO_Mode_DMA:
            hal_ret = HAL_SD_ReadBlocks_DMA(h, buf, sector, count);
            if (hal_ret != HAL_OK) {
                m->busy  = 0;
                m->error = 1;
                return SDIO_Err_DMA;
            }
            wait_for_completion(m);
            return m->error ? SDIO_Err_DMA : SDIO_Err_Ok;

        default:
            m->busy = 0;
            return SDIO_Err_Param;
    }
}

SDIO_Err_t sdio_write_blocks(SDIO_Model_t *m, const uint8_t *buf,
                             uint32_t sector, uint32_t count)
{
    if (m == NULL || buf == NULL)
        return SDIO_Err_Param;

    if (m->busy)
        return SDIO_Err_Busy;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret;

    m->busy  = 1;
    m->error = 0;

    switch (m->config.mode) {
        case SDIO_Mode_Polling: {
            uint32_t timeout = count * 1000;
            if (timeout < SDIO_TIMEOUT)
                timeout = SDIO_TIMEOUT;

            hal_ret = HAL_SD_WriteBlocks(h, (uint8_t *)buf, sector, count, timeout);
            m->busy = 0;

            if (hal_ret == HAL_TIMEOUT)
                return SDIO_Err_Timeout;
            if (hal_ret != HAL_OK) {
                m->error = 1;
                return SDIO_Err_Generic;
            }
            return SDIO_Err_Ok;
        }

        case SDIO_Mode_IT:
            hal_ret = HAL_SD_WriteBlocks_IT(h, (uint8_t *)buf, sector, count);
            if (hal_ret != HAL_OK) {
                m->busy  = 0;
                m->error = 1;
                return SDIO_Err_Generic;
            }
            wait_for_completion(m);
            return m->error ? SDIO_Err_Generic : SDIO_Err_Ok;

        case SDIO_Mode_DMA:
            hal_ret = HAL_SD_WriteBlocks_DMA(h, (uint8_t *)buf, sector, count);
            if (hal_ret != HAL_OK) {
                m->busy  = 0;
                m->error = 1;
                return SDIO_Err_DMA;
            }
            wait_for_completion(m);
            return m->error ? SDIO_Err_DMA : SDIO_Err_Ok;

        default:
            m->busy = 0;
            return SDIO_Err_Param;
    }
}

SDIO_Err_t sdio_erase_blocks(SDIO_Model_t *m, uint32_t sector, uint32_t count)
{
    if (m == NULL)
        return SDIO_Err_Param;

    if (m->busy)
        return SDIO_Err_Busy;

    uint32_t start = sector;
    uint32_t end   = sector + count - 1;

    SD_HandleTypeDef *h   = (SD_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef ret = HAL_SD_Erase(h, start, end);

    return (ret == HAL_OK) ? SDIO_Err_Ok : SDIO_Err_Generic;
}

uint8_t sdio_is_busy(SDIO_Model_t *m)
{
    return (m != NULL) ? m->busy : 0;
}

void sdio_get_info(SDIO_Model_t *m, uint32_t *block_size, uint32_t *block_count)
{
    if (m == NULL)
        return;

    if (block_size)
        *block_size = m->block_size;
    if (block_count)
        *block_count = m->block_count;
}

/* 以下均为 HAL 库内部的弱符号定义的回调函数 */

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    if (s_active == NULL || s_active->handle != (SDIO_Handle_t)hsd)
        return;

    s_active->busy  = 0;
    s_active->error = 0;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    if (s_active == NULL || s_active->handle != (SDIO_Handle_t)hsd)
        return;

    s_active->busy  = 0;
    s_active->error = 0;
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    if (s_active == NULL || s_active->handle != (SDIO_Handle_t)hsd)
        return;

    s_active->busy  = 0;
    s_active->error = 1;
}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    if (s_active == NULL || s_active->handle != (SDIO_Handle_t)hsd)
        return;

    s_active->busy  = 0;
    s_active->error = 1;
}
