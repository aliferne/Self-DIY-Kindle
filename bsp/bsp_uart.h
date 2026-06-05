#pragma once

/* =========================
 * bsp_uart.h
 *
 * UART driver header file
 * =========================
 */

#include <stdint.h>

typedef enum {
    UART_OK = 0,
    UART_ERR_INVALID_ARG,
    UART_ERR_TIMEOUT,
    UART_ERR_HW_FAILURE,
} UART_Err_t;

typedef enum {
    UART_Mode_Polling = 0,
    UART_Mode_Interrupt,
    UART_Mode_DMA,
} UART_Mode_t;

typedef void *UART_Handle_t;

typedef struct {
    UART_Mode_t mode;
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
} UART_Config_t;

typedef struct {
    UART_Config_t config;
    UART_Handle_t handle;
} UART_Model_t;

UART_Err_t uart_init(UART_Model_t *m, const UART_Config_t *cfg);
UART_Err_t uart_deinit(UART_Model_t *m);
UART_Err_t uart_send(UART_Model_t *m, const uint8_t* data, uint16_t len);
UART_Err_t uart_recv(UART_Model_t *m, uint8_t *data, uint16_t len);
UART_Err_t uart_attach_irq(UART_Model_t *m, void (*callback)(void *));
UART_Err_t uart_detach_irq(UART_Model_t *m);
