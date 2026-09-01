#pragma once

/*
 * I2S related
 */
 
/*
   some function prototypes:

HAL_StatusTypeDef HAL_I2S_Init(I2S_HandleTypeDef *hi2s);
HAL_StatusTypeDef HAL_I2S_DeInit(I2S_HandleTypeDef *hi2s);
void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s);
void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s);

HAL_StatusTypeDef HAL_I2S_Transmit(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2S_Receive(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout);

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s);
 */

typedef enum {
    i2s_ok = 0,
    i2s_err,
} i2s_err_t;

typedef struct {
    void *handle;

    bool (*write)(uint16_t *buf, uint16_t size);
    bool (*read)(uint16_t *buf, uint16_t size);
} i2s_t;

i2s_err_t i2s_init(i2s_t *d, void *handle);
i2s_err_t i2s_write(i2s_t *d, uint16_t *buf, uint16_t size);
i2s_err_t i2s_read(i2s_t *d, uint16_t *buf, uint16_t size);


