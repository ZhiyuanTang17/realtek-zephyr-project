/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr stub for main_full().
 *
 * With ZEPHYR_SUPPORT defined, the upstream main_full.c merely sets the
 * buf_output function pointer and returns 0. The actual code is wrapped in
 * #ifndef ZEPHYR_SUPPORT guards, so all the PHY / power / wakeup includes
 * in that file are dead code.  Using this stub avoids pulling in the PHY
 * header chain which contains GCC-incompatible empty variadic-macro usages.
 */

/* The buf_output pointer is declared in system_init.c and set here. */
extern void (*buf_output)(void);
extern void print_buffered_log(void);

bool (*app_main)(void);

int main_full(void)
{
    buf_output = print_buffered_log;
    return 0;
}
