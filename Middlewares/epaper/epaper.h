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
    EPaper_Normal_Init = 0,
    EPaper_Fast_Init   = 1,
    EPaper_4Gray_Init  = 2,
} EPaper_Init_Mode_t;

typedef enum {
    EPaper_Err_OK        = 0,
    EPaper_Err_Busy      = 1, /**< 屏幕忙，操作未完成 */
    EPaper_Err_IO        = 2, /**< SPI/GPIO 通信错误 */
    EPaper_Err_Param     = 3, /**< 参数错误 */
    EPaper_Err_HWNotInit = 4, /**< 模型未初始化 */
} EPaper_Err_t;

/* ============================================================
 * 墨水屏模型
 * ============================================================ */

typedef struct {
    EPaper_Init_Mode_t init_mode;
    /* 
     * 支持 1 和 1.5 两个选项，单位为秒
     * 传入非 1 或 1.5 的值将被设置为 1
     */
    float fast_init_time; 
} EPaper_Config_t;

typedef struct {
    SPI_Model_t spi;   /**< SPI 句柄（含 CS、SCK、MOSI、MISO） */
    GPIO_Model_t dc;   /**< Data/Command 选择线 */
    GPIO_Model_t rst;  /**< 复位线 */
    GPIO_Model_t busy; /**< 忙检测线（输入） */
    EPaper_Config_t cfg;
    Painter_Model_t painter; /**< 画笔模型 */
    uint8_t *canvas;         /**< 画布（数组）, 暂未被实际使用  */

    uint8_t is_sleeping: 1; /**< 休眠状态标志 */
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
void epaper_register(
    EPaper_Model_t *m,
    GPIO_Port_t sck_port, GPIO_Pin_t sck_pin,
    GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
    GPIO_Port_t miso_port, GPIO_Pin_t miso_pin,
    GPIO_Port_t cs_port, GPIO_Pin_t cs_pin,
    GPIO_Port_t dc_port, GPIO_Pin_t dc_pin,
    GPIO_Port_t rst_port, GPIO_Pin_t rst_pin,
    GPIO_Port_t busy_port, GPIO_Pin_t busy_pin);

/**
 * 初始化 epaper，并自动设置画布和辅助清屏
 *
 * 必须在 epaper_register 之后调用。
 *
 * @return EPaper_Err_OK 成功，其他为错误码。
 */
EPaper_Err_t epaper_init(EPaper_Model_t *m, EPaper_Config_t *cfg);

/**
 * 进入休眠模式，可选是否清除画布
 * need_clear: 1 - 清除画布，0 - 不清除
 */
void epaper_sleep(EPaper_Model_t *m, uint8_t need_clear);

/**
 * 去初始化。
 */
void epaper_deinit(EPaper_Model_t *m);
