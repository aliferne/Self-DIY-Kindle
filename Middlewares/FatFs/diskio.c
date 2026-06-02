/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Basic definitions of FatFs */
#include "diskio.h" /* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "bsp_config.h"
#include "bsp_sdio.h"
#include <stdint.h>

/* Example: Mapping of physical drive number for each drive */
// #define DEV_FLASH	0	/* Map FTL to physical drive 0 */
// #define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Map USB MSD to physical drive 2 */
#define DEV_SD 0

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv) {
        case DEV_SD:
            if (bsp_init_storage() != SDIO_Err_Ok) {
                return STA_NOINIT;
            }
    }
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    int res;

    switch (pdrv) {
        case DEV_SD:
            res = (int)sdio_read_blocks(&storage, buff, sector, count);
            if (res == (int)(SDIO_Err_Ok))
                return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    int res;

    switch (pdrv) {
        case DEV_SD:
            res = (int)sdio_write_blocks(&storage, (uint8_t *)buff, (uint32_t)sector, (uint32_t)count);
            if (res == (int)(SDIO_Err_Ok))
                return RES_OK;
    }

    return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    DRESULT result = RES_ERROR;

    uint32_t block_size  = 0;
    uint32_t block_count = 0;

    sdio_get_info(&storage, &block_size, &block_count);

    switch (pdrv) {
        case DEV_SD:
            switch (cmd) {
                case CTRL_SYNC:
                    result = RES_OK;
                    break;
                case GET_SECTOR_SIZE:
                    *(DWORD *)buff = 512;
                    result         = RES_OK;
                    break;
                case GET_BLOCK_SIZE:
                    *(DWORD *)buff = block_size;
                    result         = RES_OK;
                    break;
                case GET_SECTOR_COUNT:
                    *(DWORD *)buff = block_count;
                    result         = RES_OK;
                    break;

                default:
                    result = RES_PARERR;
            }
    }

    return result;
}

/* TODO: 先暂时把 get_fattime 放在这里 */
DWORD get_fattime()
{
    return 0;
}
