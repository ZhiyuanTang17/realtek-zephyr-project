/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file rtl87x2j_platform_init.h
 * @brief RTL87x2J platform initialization interface for Zephyr / MCUboot.
 *
 * Two-phase initialization model
 * ────────────────────────────────────────────────────────────────────────────
 * Phase 1 – rtl87x2j_platform_early_init()
 *   Called once from the MCUboot / Zephyr pre-kernel entry point (before the
 *   Zephyr scheduler starts).  Mirrors the original Realtek
 *   boot_patch_entry + SystemInit_zephyr sequence, with the following steps
 *   deliberately omitted because Zephyr / MCUboot takes ownership:
 *     - secure_boot_entry()  → MCUboot image authentication
 *     - image_entry()        → MCUboot image selection and jump
 *     - ram_init()           → Zephyr startup (.data copy, .bss zero-init)
 *     - timestamp_init()     → TIMESTAMP_IRQ registered on the Zephyr side
 *     - log subsystem init   → deferred to the Zephyr application layer
 *
 *   Step sequence in early_init:
 *     E1.  MBISR RAM repair (data + buffer SRAMs)
 *     E2.  Assert handler enable
 *     E3.  eFlash IRQ function-pointer assignment
 *     E4.  RXI300 bus-fabric init (skipped if AON IS_RXI300_DISABLE is set)
 *     E5.  ROM config parsing from OCCD flash partition
 *     E6.  ROT key load, FPK config, general security control
 *     E7.  ROT debug authentication + SWD access control
 *     E8.  OTP read / write protection for all ranges
 *     E9.  FT-OTP factory trim data init
 *     E10. Active-mode clock source selection
 *     E11. RXI300 clock-rate callback registration
 *     E12. PCK600 power-domain controller + scheduling-plan table init
 *     E13. SI-flow (thermal-calibration) data init
 *     E14. PMU voltage adjustment from OTP trim data
 *     E15. RAP clock enable + temperature conversion table init
 *     E16. Oscillator calibration (RC / crystal trim from OTP)
 *     E17. Hardware & CPU setup (RAM power gating, MPU, FPU)
 *     E18. Buffered-log flush callback installation
 *
 * Phase 2 – rtl87x2j_platform_late_init()
 *   Called from a Zephyr SYS_INIT() late-init hook, after the kernel
 *   scheduler and memory allocator are fully operational.  Drivers here
 *   depend on Zephyr OS services (e.g. k_timer, k_heap) or need to register
 *   Zephyr interrupt handlers rather than raw ROM vectors.
 *
 *   Step sequence in late_init:
 *     L1. Wakeup-source init
 *     L2. Platform power-manager init
 *     L3. Thermal meter hardware init
 *     L4. RF PHY hardware-control block + full PHY stack init
 *     L5. Thermal tracking (TMETER_FW_IRQn handler)
 *     L6. AMU script load + measurement engine start
 *     L7. Log UART clock switched to auto-gate mode
 *     L8. Hardware timer ISR registration (TIMER0_CH0/CH1)
 */

#ifndef RTL87X2J_PLATFORM_INIT_H
#define RTL87X2J_PLATFORM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pre-kernel SoC platform initialization (early stage).
 *
 * Performs all hardware and SoC initialization that must complete before the
 * Zephyr scheduler starts (or before MCUboot hands off to the Zephyr image).
 * Safe to call with interrupts disabled; does not use any OS services.
 *
 * @note Must be called exactly once, before rtl87x2j_platform_late_init().
 */
void rtl87x2j_platform_early_init(void);

/**
 * @brief Post-kernel SoC platform initialization (late stage).
 *
 * Performs SoC driver initialization that depends on Zephyr OS services or
 * requires Zephyr-side IRQ registration.  Must be called from a
 * SYS_INIT(, APPLICATION, …) hook after the scheduler is running.
 *
 * @note Must be called after rtl87x2j_platform_early_init() has returned.
 */
void rtl87x2j_platform_late_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RTL87X2J_PLATFORM_INIT_H */
