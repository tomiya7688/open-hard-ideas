#ifndef OPEN_HARD_ATA_TARGET_H
#define OPEN_HARD_ATA_TARGET_H

#include <stddef.h>
#include <stdint.h>

#include "block_device.h"

#define ATA_LOGICAL_SECTOR_SIZE 512u
#define ATA_IDENTIFY_DEVICE_SIZE 512u

#define ATA_CMD_READ_SECTORS_EXT  0x24u
#define ATA_CMD_READ_DMA_EXT      0x25u
#define ATA_CMD_WRITE_SECTORS_EXT 0x34u
#define ATA_CMD_WRITE_DMA_EXT     0x35u
#define ATA_CMD_FLUSH_CACHE_EXT   0xEAu
#define ATA_CMD_IDENTIFY_DEVICE   0xECu

typedef enum {
    ATA_TARGET_OK = 0,
    ATA_TARGET_ERR_ARGUMENT = -1,
    ATA_TARGET_ERR_BACKEND = -2,
    ATA_TARGET_ERR_RANGE = -3,
    ATA_TARGET_ERR_READ_ONLY = -4,
    ATA_TARGET_ERR_UNSUPPORTED = -5
} ata_target_status_t;

typedef struct {
    block_device_t *backend;
    char serial[21];
    char firmware[9];
    char model[41];
} ata_target_t;

typedef struct {
    uint8_t command;
    uint64_t lba;
    uint32_t sector_count;
    void *buffer;
} ata_target_request_t;

ata_target_status_t ata_target_init(
    ata_target_t *target,
    block_device_t *backend,
    const char *serial,
    const char *firmware,
    const char *model);

ata_target_status_t ata_target_build_identify(
    const ata_target_t *target,
    uint8_t identify[ATA_IDENTIFY_DEVICE_SIZE]);

ata_target_status_t ata_target_execute(
    ata_target_t *target,
    const ata_target_request_t *request);

#endif
