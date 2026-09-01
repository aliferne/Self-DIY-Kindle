#include "bsp_gpio.h"
#include "bsp_i2s.h"
#include "bsp_handle.h"

#define ASSERT_DEV_EXISTS(d) LOG_WHEN_FAILED(d == NULL, "i2s device should not be NULL")

i2s_err_t i2s_init(i2s_t *d, void *handle)
{
    ASSERT_DEV_EXISTS(d);
    if (handle == NULL) {
        LOG_ERROR("Should give I2S handler");
        return i2s_err;
    }

    d->handle = handle;
    // TODO:
    d->write = NULL;
    d->read = NULL;

    return i2s_ok;
}

i2s_err_t i2s_write(i2s_t *d, uint16_t *buf, uint16_t size)
{
    ASSERT_DEV_EXISTS(d);
    if (buf == NULL || size == 0) {
        LOG_ERROR("Nothing to write");
        return i2s_err;
    }

    bool ret = false;
    if (d->write == NULL) {
        LOG_ERROR("Can not write I2S, nullptr detected");
        return i2s_err;
    }
    ret = d->write(buf, size);

    return (ret == true) ? i2s_ok : i2s_err;
}

i2s_err_t i2s_read(i2s_t *d, uint16_t *buf, uint16_t size)
{
    ASSERT_DEV_EXISTS(d);
    if (buf == NULL || size == 0) {
        LOG_ERROR("Can not read into a NULL buffer or 0 length");
        return i2s_err;
    }

    bool ret = false;
    if (d->read == NULL) {
        LOG_ERROR("Can not read I2S, nullptr detected");
        return i2s_err;
    }
    ret = d->read(buf, size);

    return (ret == true) ? i2s_ok : i2s_err;
}
