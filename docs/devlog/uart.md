实际上重定向 `printf` 函数时不应当使用中断发送：

```c
/*
 * 此函数依赖于 SYS_USE_UART
 */
int _write(int file, char *ptr, int len)
{
    GIVEUP(file);

    /* 对于 printf, 应当使用这种阻塞的方式 */
    HAL_UART_Transmit(SYS_USE_UART, (uint8_t *)ptr, len, 2500);
    while (__HAL_UART_GET_FLAG(SYS_USE_UART, UART_FLAG_TC) == RESET);

    return len;
}
```

应当采用如上形式的阻塞式发送，否则无法发送出去

此外比较特殊的一点：

```c
UART_Err_t uart_send(UART_Model_t *m, const uint8_t *data, uint16_t len)
{
    UART_Err_t ret            = UART_Err_Ok;
    UART_HandleTypeDef *h     = (UART_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (m->config.mode) {
        case UART_Mode_Polling:
            hal_ret = HAL_UART_Transmit(h, data, len, UART_MAX_TIMEOUT);
            break;
        /* 
        TODO: 下面这些模式应当借助环形缓冲区实现，
              中断发送会在一段时间内返回 HAL_BUSY，
              此时如果紧接着是另一个中断发送，则该发送可能无法被发送
        */
        case UART_Mode_Interrupt:
            hal_ret = HAL_UART_Transmit_IT(h, data, len);
            break;
        case UART_Mode_DMA:
            hal_ret = HAL_UART_Transmit_DMA(h, data, len);
            break;
    }

    if (hal_ret == HAL_TIMEOUT)
        ret = UART_Err_Timeout;
    else if (hal_ret != HAL_OK)
        ret = UART_Err_Generic;

    return ret;
}
```

也就是说我们需要实现环形缓冲区，用于发送时塞入数据缓冲，避免说有数据没被发出去的情况

可以借鉴 armfly （硬汉嵌入式） 的 bsp_msg 代码，是环形缓冲区相关的

此外考虑到将 SPI Flash 作为日志写入的话……，最好也要使用这种缓冲区，毕竟写操作会损耗 Flash, 此外应当考虑日志头之类的？比如日志日期，长度，方便读取一段日志和从新位置写日志。

这种需求的话我个人认为不应当选择读写是以 sector 为单位的 Flash。

TODO: 后面再说吧，本来 Flash 也不应该属于这章（笑）
