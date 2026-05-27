/*
 * ============================================================
 * Middlewares/epaper/epaper.h
 *
 * 墨水屏硬件抽象模型。
 *
 * 将 SPI + GPIO 引脚打包为 EPaper_Model_t，
 * 电子纸驱动通过该结构体操作硬件，不直接依赖 BSP 全局变量。
 *
 * 设计要点：
 *   - SPI_Model_t 内嵌，不是指针 — 所有权清晰，生命周期跟随 epaper 模型
 *   - CS 已包含在 SPI_Model_t 中，不再单独声明
 *   - MOSI/MISO 复用同一物理引脚（电子纸极少需要回读，但偶尔需要时临时切换方向）
 *   - 本文件不 include 任何芯片厂商的头文件
 * ============================================================
 */

#pragma once

#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "gui_paint.h"

/* ============================================================
 * 错误码
 * ============================================================ */

typedef enum {
    EPaper_Err_OK        = 0,
    EPaper_Err_Busy      = 1, /**< 屏幕忙，操作未完成 */
    EPaper_Err_IO        = 2, /**< SPI/GPIO 通信错误 */
    EPaper_Err_Param     = 3, /**< 参数错误 */
    EPaper_Err_HWNotInit = 4, /**< 模型未初始化 */
} EPaper_Err_t;

/* ============================================================
 * 墨水屏硬件模型
 * ============================================================ */

typedef struct EPaper_Model {
    SPI_Model_t spi;        /**< SPI 句柄（含 CS、SCK、MOSI、MISO） */
    GPIO_Model_t dc;        /**< Data/Command 选择线 */
    GPIO_Model_t rst;       /**< 复位线 */
    GPIO_Model_t busy;      /**< 忙检测线（输入） */
    Paint_Model_t paint;    /**< 画布模型 */
    uint8_t hw_initialized; /**< 是否已调用 epaper_hw_init */
} EPaper_Model_t;

extern EPaper_Model_t e_paper;

/* ============================================================
 * API
 * ============================================================ */

/**
 * 注册 epaper 硬件引脚。
 *
 * 将各个引脚绑定到 GPIO_Model_t，但不初始化硬件（由 epaper_hw_init 完成）。
 * 注意：miso_port/pin 可复用 mosi 的物理引脚（电子纸通常只写不读）。
 *
 * @param sck_port/pin   SPI 时钟
 * @param mosi_port/pin  SPI MOSI（数据输出）
 * @param miso_port/pin  SPI MISO（可复用 mosi 引脚，用作临时回读）
 * @param cs_port/pin    SPI 片选
 * @param dc_port/pin    Data/Command 选择
 * @param rst_port/pin   复位
 * @param busy_port/pin  忙检测
 */
void epaper_hw_register(
    EPaper_Model_t *m,
    GPIO_Port_t sck_port, GPIO_Pin_t sck_pin,
    GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
    GPIO_Port_t miso_port, GPIO_Pin_t miso_pin,
    GPIO_Port_t cs_port, GPIO_Pin_t cs_pin,
    GPIO_Port_t dc_port, GPIO_Pin_t dc_pin,
    GPIO_Port_t rst_port, GPIO_Pin_t rst_pin,
    GPIO_Port_t busy_port, GPIO_Pin_t busy_pin);

/**
 * 初始化 epaper 硬件（GPIO + SPI 配置）。
 *
 * 必须在 epaper_hw_register 之后调用。
 *
 * @return EPaper_Err_OK 成功，其他为错误码。
 */
EPaper_Err_t epaper_hw_init(EPaper_Model_t *m);

/**
 * 去初始化。
 */
void epaper_hw_deinit(EPaper_Model_t *m);
