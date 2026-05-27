#pragma once

/*
 * ============================================================
 * bsp_spi.h  —  SPI 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 当前实现软件 SPI (bit-bang)，支持全部四种模式 (Mode 0/1/2/3)。后续可扩展硬件 SPI。
 *
 * 使用示例：
 *
 *   SPI_Model_t display;
 *   spi_register(&display,
 *       GPIOA, GPIO_PIN_4,   // CS
 *       GPIOA, GPIO_PIN_5,   // SCLK
 *       GPIOA, GPIO_PIN_7,   // MOSI
 *       GPIOA, GPIO_PIN_6);  // MISO
 *   spi_init(&display, &(SPI_Config_t){ .sw = {
 *       .mode         = SPI_Mode_0,
 *       .bit_delay_us = 1,
 *   }});
 *
 *   uint8_t tx[2] = {0x90, 0x00}, rx[2];
 *   spi_transmit_receive(&display, tx, rx, 2);
 *   // 或仅发送：
 *   spi_transmit(&display, tx, 2);
 *   // 或仅接收：
 *   spi_receive(&display, rx, 2);
 */

#include "bsp_gpio.h"

/* ============================================================
 * 枚举定义
 * ============================================================ */

typedef enum {
    SPI_Err_OK = 0,
} SPI_Err_t;

/*
 * SPI 模式枚举 —— bit[1]=CPOL, bit[0]=CPHA
 *
 * 此处刻意让枚举值与 (CPOL << 1) | CPHA 对齐，
 * 使得 spi_init 中可以直接用位运算解析，无需查表。
 */
typedef enum {
    SPI_Mode_0 = 0, /**< CPOL=0, CPHA=0 — SCLK 空闲低, 上升沿采样 */
    SPI_Mode_1 = 1, /**< CPOL=0, CPHA=1 — SCLK 空闲低, 下降沿采样 */
    SPI_Mode_2 = 2, /**< CPOL=1, CPHA=0 — SCLK 空闲高, 下降沿采样 */
    SPI_Mode_3 = 3, /**< CPOL=1, CPHA=1 — SCLK 空闲高, 上升沿采样 */
} SPI_Mode_t;

#define SPI_CPOL(mode) (((mode) >> 1) & 1) /**< 从枚举值提取 CPOL */
#define SPI_CPHA(mode) (((mode) >> 0) & 1) /**< 从枚举值提取 CPHA */

/* ============================================================
 * 结构体定义
 * ============================================================ */

/* ---------- 软件 SPI 配置 ---------- */
/*
 * 设计思路：
 *   - mode、bit_delay_us 由用户填入
 *   - spi_init() 根据 mode 预计算 sclk_idle / sclk_active / cpha，
 *     存入结构体。运行时 spi_sw_xfer_byte 直接使用预计算值，
 *     避免每个 bit 都查表/分支。
 */
typedef struct {
    SPI_Mode_t mode;       /**< SPI 模式 0-3（用户填入） */
    GPIO_Speed_t speed;    /**< GPIO 速度配置 */
    uint16_t bit_delay_us; /**< 半位周期延时 (μs)（用户填入） */
    /* ---- 以下由 spi_init 预计算，运行时只读 ---- */
    GPIO_Level_t sclk_idle;   /**< CPOL → SCLK 空闲电平 */
    GPIO_Level_t sclk_active; /**< !CPOL → SCLK 活跃电平 */
    uint8_t cpha;             /**< CPHA: 0=数据在第一个沿前准备好, 1=数据在第一个沿改变 */
} SPI_SW_Config_t;

/* ---------- 软件 SPI 模型（引脚） ---------- */
typedef struct {
    GPIO_Model_t cs;
    GPIO_Model_t sclk;
    GPIO_Model_t mosi;
    GPIO_Model_t miso;
} SPI_SW_Model_t;
/*
目前看来软件 SPI 的硬件模型可以和硬件 SPI 一致，本质都是引脚
但是配置的功能势必是不同的，硬件 SPI 要求必须使用 AF 功能
后面如果做硬件 SPI 的话再思考如何扩展
 */

/*
 * SPI_Config_t — union 形式，后续可扩展 hw 配置
 */
typedef union {
    SPI_SW_Config_t sw;
} SPI_Config_t;

/*
 * SPI_Model_t — 运行时模型
 */
typedef struct {
    union {
        SPI_SW_Model_t sw; /**< 软件 SPI: 四个 GPIO 引脚 */
        /* TODO: hw_spi 外设 */
    } src;
    SPI_Config_t config;
    volatile uint8_t busy : 1;
} SPI_Model_t;

/* ============================================================
 * 外部函数声明（由芯片层实现）
 * ============================================================ */

SPI_Err_t spi_register(SPI_Model_t *m,
                       GPIO_Port_t cs_port, GPIO_Pin_t cs_pin,
                       GPIO_Port_t sclk_port, GPIO_Pin_t sclk_pin,
                       GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
                       GPIO_Port_t miso_port, GPIO_Pin_t miso_pin);

SPI_Err_t spi_init(SPI_Model_t *m, const SPI_Config_t *cfg);
SPI_Err_t spi_deinit(SPI_Model_t *m);

SPI_Err_t spi_hw_write_read(SPI_Model_t *m,
                            const uint8_t *tx, uint8_t *rx, uint16_t len);

SPI_Err_t spi_hw_write(SPI_Model_t *m, const uint8_t *tx, uint16_t len);
SPI_Err_t spi_hw_read(SPI_Model_t *m, uint8_t *rx, uint16_t len);

/* ============================================================
 * 外部函数声明，软件 SPI 可在 BSP 层直接实现
 * ============================================================ */

SPI_Err_t spi_sw_write_read(SPI_Model_t *m,
                            const uint8_t *tx, uint8_t *rx, uint16_t len);

SPI_Err_t spi_sw_write(SPI_Model_t *m, const uint8_t *tx, uint16_t len);
SPI_Err_t spi_sw_read(SPI_Model_t *m, uint8_t *rx, uint16_t len);

/* ============================================================
 * 芯片级原语
 *
 * 直接操作 GPIO.
 * ============================================================ */

void spi_sw_cs_select(SPI_Model_t *m);
void spi_sw_cs_deselect(SPI_Model_t *m);
uint8_t spi_sw_xfer_byte_cpha0(
    SPI_Model_t *m,
    uint8_t tx_byte,
    uint32_t delay,
    GPIO_Level_t idle,
    GPIO_Level_t active);

uint8_t spi_sw_xfer_byte_cpha1(
    SPI_Model_t *m,
    uint8_t tx_byte,
    uint32_t delay,
    GPIO_Level_t idle,
    GPIO_Level_t active);
uint8_t spi_sw_xfer_byte(SPI_Model_t *m, uint8_t tx_byte);
