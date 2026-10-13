#include "ata_target.h"

static void zero_bytes(uint8_t *destination, size_t size)
{
    size_t i;
    for (i = 0; i < size; ++i) {
        destination[i] = 0u;
    }
}

static void copy_string(char *destination, size_t destination_size, const char *source)
{
    size_t i = 0u;

    if (destination_size == 0u) {
        return;
    }

    if (source != NULL) {
        while ((i + 1u) < destination_size && source[i] != '\0') {
            destination[i] = source[i];
            ++i;
        }
    }

    destination[i] = '\0';
    ++i;
    while (i < destination_size) {
        destination[i] = '\0';
        ++i;
    }
}

static void set_word_le(uint8_t *buffer, size_t word_index, uint16_t value)
{
    const size_t offset = word_index * 2u;
    buffer[offset] = (uint8_t)(value & 0xffu);
    buffer[offset + 1u] = (uint8_t)((value >> 8u) & 0xffu);
}

/* ATA strings are stored as big-endian byte pairs inside little-endian words. */
static void set_ata_string(
    uint8_t *identify,
    size_t first_word,
    size_t word_count,
    const char *text)
{
    size_t byte_index;
    const size_t field_bytes = word_count * 2u;

    for (byte_index = 0u; byte_index < field_bytes; ++byte_index) {
        const size_t source_index = byte_index;
        uint8_t value = (uint8_t)' ';
        const size_t word_offset = first_word * 2u + (byte_index & ~(size_t)1u);
        const size_t destination_offset = word_offset + (1u - (byte_index & 1u));

        if (text != NULL && text[source_index] != '\0') {
            value = (uint8_t)text[source_index];
        }

        identify[destination_offset] = value;

        if (text == NULL || text[source_index] == '\0') {
            size_t rest;
            for (rest = byte_index + 1u; rest < field_bytes; ++rest) {
                const size_t rest_word_offset = first_word * 2u + (rest & ~(size_t)1u);
                const size_t rest_destination_offset = rest_word_offset + (1u - (rest & 1u));
                identify[rest_destination_offset] = (uint8_t)' ';
            }
            break;
        }
    }
}

static int request_in_range(const ata_target_t *target, uint64_t lba, uint32_t sector_count)
{
    const uint64_t total = target->backend->logical_block_count;

    if (sector_count == 0u) {
        return 0;
    }
    if (lba >= total) {
        return 0;
    }
    if ((uint64_t)sector_count > (total - lba)) {
        return 0;
    }
    return 1;
}

static ata_target_status_t map_backend_status(block_device_status_t status)
{
    switch (status) {
    case BLOCK_DEVICE_OK:
        return ATA_TARGET_OK;
    case BLOCK_DEVICE_ERR_RANGE:
        return ATA_TARGET_ERR_RANGE;
    case BLOCK_DEVICE_ERR_READ_ONLY:
        return ATA_TARGET_ERR_READ_ONLY;
    case BLOCK_DEVICE_ERR_UNSUPPORTED:
        return ATA_TARGET_ERR_UNSUPPORTED;
    case BLOCK_DEVICE_ERR_ARGUMENT:
        return ATA_TARGET_ERR_ARGUMENT;
    case BLOCK_DEVICE_ERR_IO:
    default:
        return ATA_TARGET_ERR_BACKEND;
    }
}

ata_target_status_t ata_target_init(
    ata_target_t *target,
    block_device_t *backend,
    const char *serial,
    const char *firmware,
    const char *model)
{
    if (target == NULL || backend == NULL || backend->read == NULL) {
        return ATA_TARGET_ERR_ARGUMENT;
    }
    if (backend->logical_block_size != ATA_LOGICAL_SECTOR_SIZE ||
        backend->logical_block_count == 0u) {
        return ATA_TARGET_ERR_UNSUPPORTED;
    }
    if (!backend->read_only && backend->write == NULL) {
        return ATA_TARGET_ERR_ARGUMENT;
    }

    target->backend = backend;
    copy_string(target->serial, sizeof(target->serial), serial);
    copy_string(target->firmware, sizeof(target->firmware), firmware);
    copy_string(target->model, sizeof(target->model), model);
    return ATA_TARGET_OK;
}

ata_target_status_t ata_target_build_identify(
    const ata_target_t *target,
    uint8_t identify[ATA_IDENTIFY_DEVICE_SIZE])
{
    uint64_t sectors;
    uint32_t sectors_28;
    size_t i;

    if (target == NULL || target->backend == NULL || identify == NULL) {
        return ATA_TARGET_ERR_ARGUMENT;
    }

    zero_bytes(identify, ATA_IDENTIFY_DEVICE_SIZE);
    sectors = target->backend->logical_block_count;
    sectors_28 = sectors > 0x0fffffffull ? 0x0fffffffu : (uint32_t)sectors;

    set_word_le(identify, 0u, 0x0040u);
    set_ata_string(identify, 10u, 10u, target->serial);
    set_ata_string(identify, 23u, 4u, target->firmware);
    set_ata_string(identify, 27u, 20u, target->model);

    /* LBA supported. */
    set_word_le(identify, 49u, 0x0200u);

    /* 28-bit total number of user-addressable sectors. */
    set_word_le(identify, 60u, (uint16_t)(sectors_28 & 0xffffu));
    set_word_le(identify, 61u, (uint16_t)((sectors_28 >> 16u) & 0xffffu));

    /* 48-bit LBA supported and enabled. */
    set_word_le(identify, 83u, (uint16_t)(1u << 10u));
    set_word_le(identify, 86u, (uint16_t)(1u << 10u));

    /* 48-bit total number of user-addressable sectors (words 100-103). */
    for (i = 0u; i < 4u; ++i) {
        set_word_le(identify, 100u + i, (uint16_t)((sectors >> (i * 16u)) & 0xffffu));
    }

    return ATA_TARGET_OK;
}

ata_target_status_t ata_target_execute(
    ata_target_t *target,
    const ata_target_request_t *request)
{
    block_device_status_t backend_status;

    if (target == NULL || target->backend == NULL || request == NULL) {
        return ATA_TARGET_ERR_ARGUMENT;
    }

    switch (request->command) {
    case ATA_CMD_IDENTIFY_DEVICE:
        if (request->buffer == NULL) {
            return ATA_TARGET_ERR_ARGUMENT;
        }
        return ata_target_build_identify(target, (uint8_t *)request->buffer);

    case ATA_CMD_READ_SECTORS_EXT:
    case ATA_CMD_READ_DMA_EXT:
        if (request->buffer == NULL || !request_in_range(target, request->lba, request->sector_count)) {
            return ATA_TARGET_ERR_RANGE;
        }
        backend_status = target->backend->read(
            target->backend->context,
            request->lba,
            request->sector_count,
            request->buffer);
        return map_backend_status(backend_status);

    case ATA_CMD_WRITE_SECTORS_EXT:
    case ATA_CMD_WRITE_DMA_EXT:
        if (request->buffer == NULL || !request_in_range(target, request->lba, request->sector_count)) {
            return ATA_TARGET_ERR_RANGE;
        }
        if (target->backend->read_only) {
            return ATA_TARGET_ERR_READ_ONLY;
        }
        backend_status = target->backend->write(
            target->backend->context,
            request->lba,
            request->sector_count,
            request->buffer);
        return map_backend_status(backend_status);

    case ATA_CMD_FLUSH_CACHE_EXT:
        if (target->backend->flush == NULL) {
            return ATA_TARGET_OK;
        }
        backend_status = target->backend->flush(target->backend->context);
        return map_backend_status(backend_status);

    default:
        return ATA_TARGET_ERR_UNSUPPORTED;
    }
}
