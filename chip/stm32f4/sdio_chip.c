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
 *
 * 需要注意的是， DMA 传输时需要满足内存地址对齐的要求，
 * 否则有可能会导致 DMA 传输失败，
 * 需要使用 __ALIGN_BEGIN 和 __ALIGN_END 宏对要使用 DMA 的数据进行对齐
 */

#include "bsp_sdio.h"
#include "bsp_sys.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_sd.h"
#include <stdint.h>
#include <string.h>

#define SD_BLOCKSIZE 512U
#define SDIO_TIMEOUT 2500U

/* 假定芯片只使用一块 SD 卡，因此只需要单例 */
static sdio_t *s_active = NULL;

/*
TODO:
    需要查清为何无法使用中断和 DMA
    中断模式下 check_card_state 会 Error，
    并且跑第二次代码时需要重新拔插 SD 卡才能正常 MX_SD_SDIO_Init
*/

sdio_err_t sdio_init(sdio_t *m, sdio_handle_t handle,
                     const sdio_cfg_t *cfg)
{
    if (m == NULL || handle == NULL || cfg == NULL)
        return SDIO_Err_Param;

    if (m->enter_critical_cb == NULL || m->exit_critical_cb == NULL)
        return SDIO_Err_NoCriticalCB;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)handle;
    m->handle           = handle;
    m->config           = *cfg;

    /*
     * 我们不能在此处进行初始化操作，由于 FatFs 会在挂盘的时候创建互斥锁，
     * 导致 BASEPRI 寄存器的值变为 0x50，将系统时钟屏蔽，
     * 进而导致 `HAL_SD_Init` 内部某个函数调用 `HAL_Delay` 时无法进入系统时钟中断，
     * 导致卡死在初始化，此处需要通过外部声明 SD 卡插入检测函数来完成初始化操作
     */
#if 0
    if (m->sdcard_det_cb && !m->sdcard_det_cb()) {
        LOG_ERROR("SD card not inserted\n");
        return SDIO_Err_CardOut;
    }

    h->Instance                 = SDIO;
    h->Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    h->Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    h->Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
    h->Init.BusWide             = SDIO_BUS_WIDE_4B;
    h->Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    h->Init.ClockDiv            = 0;

    if (HAL_SD_Init(h) != HAL_OK) {
        LOG_ERROR("SD card initialization failed\n");
        return SDIO_Err_Generic;
    }
#endif
    /* 配置总线宽度 */
    HAL_StatusTypeDef ret = HAL_SD_ConfigWideBusOperation(h, cfg->wide_bus ? SDIO_BUS_WIDE_4B : SDIO_BUS_WIDE_1B);
    if (ret != HAL_OK) {
        LOG_ERROR("SD card set bus width failed\n");
        return SDIO_Err_Generic;
    }

    /* 获取卡片信息 */
    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(h, &info) != HAL_OK) {
        m->block_size  = SD_BLOCKSIZE;
        m->block_count = 0;
    } else {
        /* 一般来说固定为 512 */
        m->block_size = info.BlockSize;
        /* 这个则视存储卡内存大小而定 */
        m->block_count = info.BlockNbr;
    }

    s_active = m;

    return SDIO_Err_Ok;
}

/* ============================================================
 * 内部辅助：等待 SD 卡进入 Transfer State
 *
 * HAL 要求在每次读写操作前检查卡状态。此函数循环等待
 * 直到卡回到可传输状态，超时返回 SDIO_Err_Timeout。
 * 超时时自动置 error 标志位
 * ============================================================ */

static sdio_err_t check_card_state(sdio_t *m)
{
    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    sdio_err_t stat     = SDIO_Err_Ok;

    static uint32_t start_time = 0;
    if (start_time == 0) start_time = chip_get_tick();

    while (HAL_SD_GetCardState(h) != HAL_SD_CARD_TRANSFER) {
        if (chip_till_max_delay(start_time, SDIO_TIMEOUT)) {
            stat = SDIO_Err_Timeout;
            break;
        }
    }
    /* 清空起始时间以等待下次使用 */
    start_time = 0;

    return stat;
}

sdio_err_t sdio_read_blocks(sdio_t *m, uint8_t *buf,
                            uint32_t sector, uint32_t count)
{
    if (m == NULL || buf == NULL)
        return SDIO_Err_Param;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret;

    switch (m->config.mode) {
        case SDIO_Mode_Polling: {
            uint32_t timeout = count * 1000;
            if (timeout < SDIO_TIMEOUT)
                timeout = SDIO_TIMEOUT;

            if (m->enter_critical_cb) m->enter_critical_cb();
            hal_ret = HAL_SD_ReadBlocks(h, buf, sector, count, timeout);
            if (m->exit_critical_cb) m->exit_critical_cb();

            if (hal_ret == HAL_TIMEOUT)
                return SDIO_Err_Timeout;
            if (hal_ret != HAL_OK)
                return SDIO_Err_Generic;

            return check_card_state(m);
        }

        case SDIO_Mode_IT:
            hal_ret = HAL_SD_ReadBlocks_IT(h, buf, sector, count);
            if (hal_ret != HAL_OK)
                return SDIO_Err_Generic;

            return check_card_state(m);
        case SDIO_Mode_DMA:
            hal_ret = HAL_SD_ReadBlocks_DMA(h, buf, sector, count);
            if (hal_ret != HAL_OK)
                return SDIO_Err_DMA;

            return check_card_state(m);
        default:
            return SDIO_Err_Param;
    }
}

sdio_err_t sdio_write_blocks(sdio_t *m, const uint8_t *buf,
                             uint32_t sector, uint32_t count)
{
    if (m == NULL || buf == NULL)
        return SDIO_Err_Param;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret;

    switch (m->config.mode) {
        case SDIO_Mode_Polling: {
            uint32_t timeout = count * 1000;
            if (timeout < SDIO_TIMEOUT)
                timeout = SDIO_TIMEOUT;

            if (m->enter_critical_cb) m->enter_critical_cb();
            hal_ret = HAL_SD_WriteBlocks(h, (uint8_t *)buf, sector, count, timeout);
            if (m->exit_critical_cb) m->exit_critical_cb();

            if (hal_ret == HAL_TIMEOUT)
                return SDIO_Err_Timeout;
            if (hal_ret != HAL_OK)
                return SDIO_Err_Generic;

            return check_card_state(m);
        }

        case SDIO_Mode_IT:
            hal_ret = HAL_SD_WriteBlocks_IT(h, (uint8_t *)buf, sector, count);
            if (hal_ret != HAL_OK)
                return SDIO_Err_Generic;

            return check_card_state(m);
        case SDIO_Mode_DMA:
            hal_ret = HAL_SD_WriteBlocks_DMA(h, (uint8_t *)buf, sector, count);
            if (hal_ret != HAL_OK)
                return SDIO_Err_DMA;

            return check_card_state(m);
        default:
            return SDIO_Err_Param;
    }
}

sdio_err_t sdio_erase_blocks(sdio_t *m, uint32_t sector, uint32_t count)
{
    if (m == NULL)
        return SDIO_Err_Param;

    uint32_t start = sector;
    uint32_t end   = sector + count - 1;

    SD_HandleTypeDef *h = (SD_HandleTypeDef *)m->handle;

    if (m->enter_critical_cb) m->enter_critical_cb();
    HAL_StatusTypeDef ret = HAL_SD_Erase(h, start, end);
    if (m->exit_critical_cb) m->exit_critical_cb();

    if (ret != HAL_OK) return SDIO_Err_Generic;

    return check_card_state(m);
}

void sdio_get_info(sdio_t *m, uint32_t *block_size, uint32_t *block_count)
{
    if (m == NULL)
        return;

    if (block_size)
        *block_size = m->block_size;
    if (block_count)
        *block_count = m->block_count;
}
