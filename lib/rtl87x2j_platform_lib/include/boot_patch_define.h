/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * This is a Linux-path-compatible replacement of the Keil bootloader
 * boot_patch_define.h for use in the rtl87x2j_platform_lib build.
 * Backslash-relative includes from the original have been replaced with
 * flat header names resolved via -I flags in the Makefile.
 */
#ifndef BOOT_PATCH_DEFINE_H
#define BOOT_PATCH_DEFINE_H

/* These headers are found via -I paths set in Makefile */
#include "mem_config.h"
#include "mem_cfg_int.h"
#include "flash_map.h"
#include "bee5_rom_defines.h"

///////////////////////////////////////////////////////////////////
#define BOOT_PATCH_FLASH_ADR                BOOT_PATCH_ADDR
#define BOOT_PATCH_FLASH_SIZE               BOOT_PATCH_SIZE

/* Zephyr platform lib: single-bank, XIP (no RAM code encryption) */
#define BOOT_PATCH_SINGLE_OTA_BANK          0
#define BOOT_PATCH_FEATURE_RAM_CODE         1
#define BOOT_PATCH_RAM_CODE_ENCRYPTED       0

#define BOOT_PATCH_FEATURE_FLASH_SEC        0

#define SUPPORT_SINGLE_OTA_BANK             1

#define SYS_ROM_PCSM_REDUCE_DELAY_TIME      0
#define ZEPHYR_SUPPORT                      1

/* RAM code region */
#define BOOT_PATCH_RAM_TEXT_ADDR            DATA_RAM_BOOT_PATCH_ADDR
#if BOOT_PATCH_FEATURE_RAM_CODE == 1
#define BOOT_PATCH_RAM_TEXT_SIZE            (7 * 1024)
#else
#define BOOT_PATCH_RAM_TEXT_SIZE            (0 * 1024)
#endif

/* RW & ZI */
#define BOOT_PATCH_RAM_DATA_ADDR            (BOOT_PATCH_RAM_TEXT_ADDR + BOOT_PATCH_RAM_TEXT_SIZE)
#define BOOT_PATCH_RAM_DATA_SIZE            (DATA_RAM_BOOT_PATCH_SIZE - BOOT_PATCH_RAM_TEXT_SIZE)

#endif /* BOOT_PATCH_DEFINE_H */
