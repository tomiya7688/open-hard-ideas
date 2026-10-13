# USB Storage -> SATA: embedded reference core

This directory contains the first embedded-oriented reference implementation for the proposed **USB Mass Storage -> SATA Device Bridge**.

It is deliberately written in portable C rather than Python so the core logic can move onto an MCU, SoC, RTOS, or bare-metal target without being rewritten.

## What this implements

The current code implements the middle of the bridge:

```text
USB Mass Storage / SCSI backend
          |
          | block_device_t
          v
+-----------------------------+
| ATA target command core     |
|                             |
| IDENTIFY DEVICE             |
| READ SECTORS EXT            |
| READ DMA EXT                |
| WRITE SECTORS EXT           |
| WRITE DMA EXT               |
| FLUSH CACHE EXT             |
+-----------------------------+
          |
          v
SATA Device transport / PHY
```

The backend is an abstract block device. On actual hardware it can be connected to a USB Host Mass Storage stack. The SATA transport can later call `ata_target_execute()` after decoding commands/FISes.

## Embedded constraints

The core is intended to remain friendly to embedded systems:

- C11
- no heap allocation
- no OS dependency
- no filesystem dependency
- no threads required
- block I/O supplied through callbacks
- fixed-size ATA identity strings
- currently fixed to 512-byte logical sectors

Python may still be useful for external testbenches, but it is not required by the device-side core.

## What this does not implement yet

This is **not yet a complete SATA device**. It does not currently implement:

- SATA PHY / SerDes
- SATA OOB signalling
- SATA link state machine
- FIS encode/decode
- NCQ
- SMART
- TRIM / DSM
- USB enumeration or USB Mass Storage itself

Those should be provided by existing hardware/IP/firmware where possible. Only missing pieces should be implemented here.

## Building the host-side test

The test uses a RAM-backed block device, but the exact same ATA target core is used.

```sh
make test
```

Expected result:

```text
PASS: ATA target block bridge reference tests
```

## Next steps

1. Add an adapter for an embedded USB Host MSC stack.
2. Define a small SATA Device transport interface around this command core.
3. Add a FIS-level simulator/BFM and exercise IDENTIFY/READ/WRITE through it.
4. Select actual MCU/SoC/FPGA parts and decide where the SATA Device transport lives.
