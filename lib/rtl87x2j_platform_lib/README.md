# rtl87x2j_platform_lib

Static library (`.a`) that provides the RTL87x2J SoC platform initialization
for use by Zephyr / MCUboot, replacing the Realtek proprietary bootloader
`boot_patch_entry()` + `SystemInit_zephyr()` sequence.

## Public API

```c
#include "rtl87x2j_platform_init.h"

void rtl87x2j_platform_init(void);
```

Call `rtl87x2j_platform_init()` once from the MCUboot / Zephyr entry path
before the OS scheduler starts. The function performs:

| Step | Function | Note |
|------|----------|------|
| 1 | MBISR RAM repair | reads repair info from eFlash NVR |
| 2 | Assert enable | |
| 3 | eFlash function pointer assignment | |
| 4 | RXI300 bus fabric init | conditional on AON register |
| 5 | ROM config from OCCD | |
| 6 | ROT key / FPK / system / debug auth / SWD | |
| 7 | OTP protection setup | read + write protect |
| 8–15 | `SystemInit_zephyr()` body | **without** `image_entry()` |

**Omitted vs. original bootloader:**
- `secure_boot_entry()` — handled by MCUboot
- `image_entry()` — handled by MCUboot
- `ram_init()` — Keil scatter-load; not needed (Zephyr startup handles data/BSS)

## Building (standalone)

```bash
cd realtek-zephyr-project/lib/rtl87x2j_platform_lib

make \
  CC=arm-zephyr-eabi-gcc \
  AR=arm-zephyr-eabi-ar \
  CFLAGS="-mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -Os" \
  RTK_BASE=/mnt/d/zcode/bee5-zephyr/realtek
```

Output: `lib/librtl87x2j_platform_lib.a`

## Integration into Zephyr application

In your application or MCUboot `CMakeLists.txt` (after `find_package(Zephyr ...)`):

```cmake
add_subdirectory(/path/to/realtek-zephyr-project/lib/rtl87x2j_platform_lib
                 ${CMAKE_CURRENT_BINARY_DIR}/rtl87x2j_platform_lib)

target_link_libraries(app PUBLIC rtl87x2j_platform_lib)
```

## Source dependencies

All `.c` files are compiled from the Realtek `bee5-zephyr` source tree.
Set `RTK_BASE` (default: `/mnt/d/zcode/bee5-zephyr/realtek`) to point to
your checkout.

## Excluded modules

The following Keil project modules are **not** compiled into this library:

- `secure_boot/` — `secure_boot.c`, `image_decryption.c`, `signature_verify.c`, `mac_verify.c`, `secure_version.c`
- `driver/crypto/` — AES/SHA2 engine, ECC, uECC (only used by secure_boot)
- `dfu/` — `comp_file.c`, `boot_fw_activation.c`, `dfu_cfg.c`
- `external/lzma1806/LzmaDec.c` — DFU compression
- `boot_patch_entry.c` — replaced by `src/rtl87x2j_platform_init.c`
- `SystemInit_zephyr()` is compiled in (via `system_init.c`) but **not called directly**;
  individual platform init calls are made from `rtl87x2j_platform_init.c` instead
