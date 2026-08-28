/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file rtl87x2j_platform_init.h
 * @brief RTL87x2J platform initialization interface for Zephyr/MCUboot.
 *
 * Call rtl87x2j_platform_init() once from the MCUboot entry before handing
 * control to the Zephyr image. The function performs all hardware / SoC
 * initialization that would normally be done by the Realtek bootloader
 * (boot_patch_entry + SystemInit_zephyr), with the exception of:
 *   - secure_boot_entry()  -> handled by MCUboot
 *   - image_entry()        -> handled by MCUboot
 *   - ram_init()           -> handled by Zephyr startup (scatter-load not needed)
 */

#ifndef RTL87X2J_PLATFORM_INIT_H
#define RTL87X2J_PLATFORM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Perform full RTL87x2J SoC platform initialization.
 *
 * Sequence (mirrors Realtek boot_patch_entry + SystemInit_zephyr):
 *  1. MBISR RAM repair
 *  2. Assert handler enable
 *  3. eFlash function pointer assignment
 *  4. RXI300 init (if not disabled in AON)
 *  5. ROM config parsing from OCCD
 *  6. ROT key / FPK / system control / debug auth / SWD control
 *  7. OTP read/write protection setup
 *  8. Disable IRQs (until Zephyr scheduler starts)
 *  9. FT-OTP init
 * 10. PCK600, schedule-plan, PMU voltage, RAP clock, TM temperature
 * 11. Clock OSC calibration
 * 12. RXI300 clock-rate function update
 * 13. Hardware & CPU setup (RAM power, MPU, FPU)
 * 14. Platform drivers: main_full() with ZEPHYR_SUPPORT (sets buf_output ptr)
 * 15. Timer IRQ init
 */
void rtl87x2j_platform_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RTL87X2J_PLATFORM_INIT_H */
