#include "srv_config.h"
#include "storage_srv.h"

Storage_t sdcard;

void service_init(void)
{
    storage_srv_init(&sdcard);
}