/*
 * ============================================================
 * chip/stm32f4/spi_chip.c
 *
 * BSP SPI 抽象的 STM32F4 芯片实现。
 *
 * 软件 SPI (bit-bang) 支持 Mode 0/1/2/3：
 *   - CS   → Output_PP
 *   - SCLK → Output_PP
 *   - MOSI → Output_PP
 *   - MISO → Input
 *   - SCLK 空闲电平由模式决定（Mode 0/1 低，Mode 2/3 高）
 *   使用 bsp_gpio API 控制引脚电平。
 * ============================================================
 */

#include "bsp_spi.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_spi.h"

static SPI_Err_t hal_status_to_spi_err(HAL_StatusTypeDef stat);

/* 硬件 SPI 传输实现（通过虚函数表调用，外部不直接使用） */
static SPI_Err_t spi_hw_write_read(SPI_Model_t *m,
                                   const uint8_t *tx, uint8_t *rx, uint16_t len);
static SPI_Err_t spi_hw_write(SPI_Model_t *m,
                              const uint8_t *tx, uint16_t len);
static SPI_Err_t spi_hw_read(SPI_Model_t *m,
                             uint8_t *rx, uint16_t len);

/*
 * 以下三个函数实现在 bsp_spi.c 中，标记为 static。
 * chip 层通过函数指针引用，此处用 extern  声明以通过编译。
 */
extern SPI_Err_t spi_sw_write_read(SPI_Model_t *m,
                                   const uint8_t *tx, uint8_t *rx, uint16_t len);
extern SPI_Err_t spi_sw_write(SPI_Model_t *m, const uint8_t *tx, uint16_t len);
extern SPI_Err_t spi_sw_read(SPI_Model_t *m, uint8_t *rx, uint16_t len);

/* ============================================================
 * API 实现
 * ============================================================ */

SPI_Err_t spi_init(SPI_Model_t *m, SPI_Register_Cfg_t *reg_cfg, const SPI_Config_t *cfg)
{
    GPIO_Config_t gpio_cfg = {
        .mode      = GPIO_Mode_Output_PP,
        .pull      = GPIO_Pull_None,
        .speed     = cfg->sw.speed,
        .alternate = 0,
    };

    m->drv  = reg_cfg->drv;
    m->busy = 0;

    if (reg_cfg->drv == SPI_Driver_SW) {
        /* ---- 软件 SPI：注册并初始化四个 GPIO 引脚 ---- */
        gpio_init(&m->src.sw.cs, reg_cfg->src.sw.cs_port, reg_cfg->src.sw.cs_pin, &gpio_cfg);
        gpio_init(&m->src.sw.sclk, reg_cfg->src.sw.sck_port, reg_cfg->src.sw.sck_pin, &gpio_cfg);
        gpio_init(&m->src.sw.mosi, reg_cfg->src.sw.mosi_port, reg_cfg->src.sw.mosi_pin, &gpio_cfg);

        gpio_cfg.mode = GPIO_Mode_Input;
        gpio_init(&m->src.sw.miso, reg_cfg->src.sw.miso_port, reg_cfg->src.sw.miso_pin, &gpio_cfg);

        /* 填入 SW 虚函数表 */
        m->write_read = spi_sw_write_read;
        m->write      = spi_sw_write;
        m->read       = spi_sw_read;
    } else {
        /* ---- 硬件 SPI：保存外设句柄 ---- */
        m->src.hw = reg_cfg->src.hw;

        /* 填入 HW 虚函数表 */
        m->write_read = spi_hw_write_read;
        m->write      = spi_hw_write;
        m->read       = spi_hw_read;
    }

    m->config = *cfg;
    m->busy   = 0;

    /* ================================================================
     * 预计算：将 mode 解析为 CPOL/CPHA，推导 sclk_idle / sclk_active / cpha。
     * 枚举值与 (CPOL<<1)|CPHA 对齐，直接用位运算取出。
     * ================================================================ */
    {
        uint8_t cpol = SPI_CPOL(cfg->sw.mode);
        uint8_t cpha = SPI_CPHA(cfg->sw.mode);

        m->config.sw.sclk_idle   = cpol ? GPIO_Level_High : GPIO_Level_Low;
        m->config.sw.sclk_active = cpol ? GPIO_Level_Low : GPIO_Level_High;
        m->config.sw.cpha        = cpha;
    }

    /* 默认状态：CS 取消选中，SCLK 设置为空闲电平 */
    spi_cs_deselect(m);
    (void)gpio_write(&m->src.sw.sclk, m->config.sw.sclk_idle);

    return SPI_Err_OK;
}

SPI_Err_t spi_deinit(SPI_Model_t *m)
{
    gpio_deinit(&m->src.sw.cs);
    gpio_deinit(&m->src.sw.sclk);
    gpio_deinit(&m->src.sw.mosi);
    gpio_deinit(&m->src.sw.miso);
    m->busy = 0;
    return SPI_Err_OK;
}

static SPI_Err_t spi_hw_read(SPI_Model_t *m, uint8_t *rx, uint16_t len)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)m->src.hw;
    HAL_StatusTypeDef stat  = HAL_SPI_Receive(hspi, rx, len, m->config.hw.timeout);
    return hal_status_to_spi_err(stat);
}

static SPI_Err_t spi_hw_write(SPI_Model_t *m, const uint8_t *tx, uint16_t len)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)m->src.hw;
    HAL_StatusTypeDef stat  = HAL_SPI_Transmit(hspi, tx, len, m->config.hw.timeout);
    return hal_status_to_spi_err(stat);
}

static SPI_Err_t spi_hw_write_read(SPI_Model_t *m, const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)m->src.hw;
    HAL_StatusTypeDef stat  = HAL_SPI_TransmitReceive(hspi, tx, rx, len, m->config.hw.timeout);
    return hal_status_to_spi_err(stat);
}

static SPI_Err_t hal_status_to_spi_err(HAL_StatusTypeDef stat)
{
    switch (stat) {
        case HAL_ERROR:
            return SPI_Err_Error;
        case HAL_BUSY:
            return SPI_Err_Busy;
        case HAL_TIMEOUT:
            return SPI_Err_Timeout;
        default:
            return SPI_Err_OK;
    }
}
