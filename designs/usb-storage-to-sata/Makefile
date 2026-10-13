CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
CPPFLAGS ?= -Iinclude

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_ata_target

.PHONY: all test clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): src/ata_target.c tests/test_ata_target.c include/ata_target.h include/block_device.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/ata_target.c tests/test_ata_target.c -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
