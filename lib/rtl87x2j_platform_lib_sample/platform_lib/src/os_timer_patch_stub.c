/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Compatibility storage for the OS timer patch hook normally defined by
 * platform_interface.c. The Zephyr OS interface installs the real callback
 * during os_zephyr_patch_init().
 */

#include <stddef.h>
#include <stdint.h>

uint32_t (*patch_os_timer_max_num_get)(uint32_t *) = NULL;
