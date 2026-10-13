#include "ata_target.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_SECTORS 32u

static uint8_t storage[TEST_SECTORS][ATA_LOGICAL_SECTOR_SIZE];

static block_device_status_t ram_read(
    void *context,
    uint64_t lba,
    uint32_t block_count,
    void *destination)
{
    uint8_t (*disk)[ATA_LOGICAL_SECTOR_SIZE] = context;
    if (lba >= TEST_SECTORS || block_count > TEST_SECTORS - lba) {
        return BLOCK_DEVICE_ERR_RANGE;
    }
    memcpy(destination, &disk[lba][0], (size_t)block_count * ATA_LOGICAL_SECTOR_SIZE);
    return BLOCK_DEVICE_OK;
}

static block_device_status_t ram_write(
    void *context,
    uint64_t lba,
    uint32_t block_count,
    const void *source)
{
    uint8_t (*disk)[ATA_LOGICAL_SECTOR_SIZE] = context;
    if (lba >= TEST_SECTORS || block_count > TEST_SECTORS - lba) {
        return BLOCK_DEVICE_ERR_RANGE;
    }
    memcpy(&disk[lba][0], source, (size_t)block_count * ATA_LOGICAL_SECTOR_SIZE);
    return BLOCK_DEVICE_OK;
}

static block_device_status_t ram_flush(void *context)
{
    (void)context;
    return BLOCK_DEVICE_OK;
}

static uint16_t word_le(const uint8_t *data, size_t word_index)
{
    size_t offset = word_index * 2u;
    return (uint16_t)data[offset] | (uint16_t)((uint16_t)data[offset + 1u] << 8u);
}

int main(void)
{
    block_device_t backend = {
        .context = storage,
        .logical_block_size = ATA_LOGICAL_SECTOR_SIZE,
        .logical_block_count = TEST_SECTORS,
        .read_only = false,
        .read = ram_read,
        .write = ram_write,
        .flush = ram_flush,
    };
    ata_target_t target;
    ata_target_request_t request;
    uint8_t buffer[ATA_LOGICAL_SECTOR_SIZE];
    uint8_t write_data[ATA_LOGICAL_SECTOR_SIZE];
    uint8_t identify[ATA_IDENTIFY_DEVICE_SIZE];
    size_t i;

    for (i = 0u; i < ATA_LOGICAL_SECTOR_SIZE; ++i) {
        storage[3u][i] = (uint8_t)(i & 0xffu);
        write_data[i] = (uint8_t)(0xa5u ^ (uint8_t)i);
    }

    assert(ata_target_init(
               &target,
               &backend,
               "OH0000000001",
               "0.1",
               "Open Hard USB Storage to SATA") == ATA_TARGET_OK);

    request.command = ATA_CMD_IDENTIFY_DEVICE;
    request.lba = 0u;
    request.sector_count = 1u;
    request.buffer = identify;
    assert(ata_target_execute(&target, &request) == ATA_TARGET_OK);
    assert(word_le(identify, 60u) == TEST_SECTORS);
    assert((word_le(identify, 83u) & (1u << 10u)) != 0u);

    memset(buffer, 0, sizeof(buffer));
    request.command = ATA_CMD_READ_DMA_EXT;
    request.lba = 3u;
    request.sector_count = 1u;
    request.buffer = buffer;
    assert(ata_target_execute(&target, &request) == ATA_TARGET_OK);
    assert(memcmp(buffer, storage[3u], sizeof(buffer)) == 0);

    request.command = ATA_CMD_WRITE_DMA_EXT;
    request.lba = 4u;
    request.sector_count = 1u;
    request.buffer = write_data;
    assert(ata_target_execute(&target, &request) == ATA_TARGET_OK);
    assert(memcmp(storage[4u], write_data, sizeof(write_data)) == 0);

    request.command = ATA_CMD_FLUSH_CACHE_EXT;
    request.lba = 0u;
    request.sector_count = 0u;
    request.buffer = NULL;
    assert(ata_target_execute(&target, &request) == ATA_TARGET_OK);

    request.command = ATA_CMD_READ_DMA_EXT;
    request.lba = TEST_SECTORS;
    request.sector_count = 1u;
    request.buffer = buffer;
    assert(ata_target_execute(&target, &request) == ATA_TARGET_ERR_RANGE);

    puts("PASS: ATA target block bridge reference tests");
    return 0;
}
