#ifndef OPEN_HARD_BLOCK_DEVICE_H
#define OPEN_HARD_BLOCK_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BLOCK_DEVICE_OK = 0,
    BLOCK_DEVICE_ERR_ARGUMENT = -1,
    BLOCK_DEVICE_ERR_IO = -2,
    BLOCK_DEVICE_ERR_RANGE = -3,
    BLOCK_DEVICE_ERR_READ_ONLY = -4,
    BLOCK_DEVICE_ERR_UNSUPPORTED = -5
} block_device_status_t;

typedef block_device_status_t (*block_device_read_fn)(
    void *context,
    uint64_t lba,
    uint32_t block_count,
    void *destination);

typedef block_device_status_t (*block_device_write_fn)(
    void *context,
    uint64_t lba,
    uint32_t block_count,
    const void *source);

typedef block_device_status_t (*block_device_flush_fn)(void *context);

typedef struct {
    void *context;
    uint32_t logical_block_size;
    uint64_t logical_block_count;
    bool read_only;
    block_device_read_fn read;
    block_device_write_fn write;
    block_device_flush_fn flush;
} block_device_t;

#endif
