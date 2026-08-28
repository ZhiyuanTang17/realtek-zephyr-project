/*
 * Copyright (c) 2025 Realtek Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * RTL87x2J platform initializer for Zephyr / MCUboot.
 *
 * Derived from:
 *   realtek/soc/flash_proj/bootloader/src/boot_patch_entry.c  (boot_patch_entry)
 *   realtek/soc/boot/eflash_boot/system_init.c                (SystemInit_zephyr)
 *
 * Omissions vs. the original bootloader:
 *   - secure_boot_entry()  : handled by MCUboot image verification
 *   - image_entry()        : handled by MCUboot before jumping to Zephyr
 *   - ram_init()           : Keil scatter-load; not needed (Zephyr startup
 *                            handles .data copy and .bss zero-init)
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "rtl87x2j_platform_init.h"

/* SoC / platform headers */
#include "aon_reg.h"
#include "platform_reg.h"
#include "eflash_driver.h"
#include "rxi300_ctrl.h"
#include "occd_parser.h"
#include "rot_ctrl.h"
#include "rot_preliminary.h"
#include "otp_api.h"
#include "otp_table.h"
#include "image_info.h"
#include "boot_cfg.h"
#include "dbg_cfg.h"
#include "rom_cfg.h"
#include "clock_manager_cal.h"
#include "clock_manager.h"
#include "cpu_cfg.h"
#include "ft_otp_driver.h"
#include "pck600.h"
#include "schedule_plan.h"
#include "pmu.h"
#include "rtl_rcc.h"
#include "rtl_rap.h"
#include "rtl_tm.h"
#include "mpu.h"
#include "fpu.h"
#include "ram_pwr_ctrl.h"
#include "rxi300_timeout.h"
#include "system_init.h"
#include "soc_log.h"
#include "assert_handler.h"
#include "utils.h"

/* -------------------------------------------------------------------------
 * MBISR RAM repair  (verbatim from boot_patch_entry.c, made static)
 * ----------------------------------------------------------------------- */

#define MBISR_REPAIR_INFO_START_ADDR            (EFLASH_AUTO_MODE_NVR_CFG_ADDR + 0x5 * 4)
#define MBISR_DATA_RAM0_REPAIR_INFO_BIT_OFFSET  (0)
#define MBISR_DATA_RAM1_REPAIR_INFO_BIT_OFFSET  (8)
#define MBISR_DATA_RAM2_REPAIR_INFO_BIT_OFFSET  (16)
#define MBISR_BUFFER_RAM_REPAIR_INFO_BIT_OFFSET (24)
#define MBISR_DATA_RAM_REPAIR_INFO_BIT_MASK     (0x3F)
#define MBISR_BUFFER_RAM_REPAIR_INFO_BIT_MASK   (0x7F)

static void mbisr_with_repair_info(void)
{
    /* step 1-1: read repair info from eflash */
    uint32_t all_ram_mbisr_repair_info = HAL_READ32(MBISR_REPAIR_INFO_START_ADDR, 0);
    uint32_t data_sram_0_repair_info   = (all_ram_mbisr_repair_info >>
                                          MBISR_DATA_RAM0_REPAIR_INFO_BIT_OFFSET)
                                         & MBISR_DATA_RAM_REPAIR_INFO_BIT_MASK;
    uint32_t data_sram_1_repair_info   = (all_ram_mbisr_repair_info >>
                                          MBISR_DATA_RAM1_REPAIR_INFO_BIT_OFFSET)
                                         & MBISR_DATA_RAM_REPAIR_INFO_BIT_MASK;
    uint32_t data_sram_2_repair_info   = (all_ram_mbisr_repair_info >>
                                          MBISR_DATA_RAM2_REPAIR_INFO_BIT_OFFSET)
                                         & MBISR_DATA_RAM_REPAIR_INFO_BIT_MASK;
    uint32_t buffer_sram_0_repair_info = (all_ram_mbisr_repair_info >>
                                          MBISR_BUFFER_RAM_REPAIR_INFO_BIT_OFFSET)
                                         & MBISR_BUFFER_RAM_REPAIR_INFO_BIT_MASK;

    /* step 1-2: store repair info to platform reg */
    uint32_t data_sram_repair_info = (data_sram_0_repair_info << 12) |
                                     (data_sram_1_repair_info << 6)  |
                                      data_sram_2_repair_info;
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_DATA_SRAM_CTRL, bisr_remap_sig_data_ram,
                                data_sram_repair_info);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_BUF_SRAM_CTRL, bisr_remap_sig_buffer_ram,
                                buffer_sram_0_repair_info);

    /* step 2-1: enable mbisr clock */
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_BIST_MODE, bist_mode_data_ram, 1);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_BT_BIST_MODE, bist_mode_buffer_ram, 1);

    /* step 2-2: de-assert remap_rstn */
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_DATA_SRAM_CTRL, bisr_remap_rstn_data_ram, 1);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_BUF_SRAM_CTRL, bisr_remap_rstn_buffer_ram, 1);

    /* step 2-3: enable repair function */
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_DATA_SRAM_CTRL, bisr_load_fuse_data_ram, 1);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_BUF_SRAM_CTRL, bisr_load_fuse_buffer_ram, 1);

    /* step 2-4: wait repair done */
    platform_delay_us(2);

    /* step 3-1: disable mbisr clock */
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_BIST_MODE, bist_mode_data_ram, 0);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_BT_BIST_MODE, bist_mode_buffer_ram, 0);

    /* step 3-2: disable repair function */
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_DATA_SRAM_CTRL, bisr_load_fuse_data_ram, 0);
    PLATFORM_REG_WRITE_BITFIELD(PLATFORM_REG_MBISR_BUF_SRAM_CTRL, bisr_load_fuse_buffer_ram, 0);
}

/* -------------------------------------------------------------------------
 * OCCD ROM-config parsing  (verbatim from boot_patch_entry.c, made static)
 * ----------------------------------------------------------------------- */

static const MODULE_INFO boot_rom_cfg_info[MODULE_ID_BOOT_MAX] =
{
    [MODULE_ID_DBG] = {
        .base = (uint32_t *) &dbg_cfg,
        .size = sizeof(dbg_cfg),
    },
    [MODULE_ID_BOOT_ROM] = {
        .base = (uint32_t *) &boot_rom_cfg,
        .size = sizeof(boot_rom_cfg),
    },
    [MODULE_ID_BOOT] = {
        .base = (uint32_t *) &boot_cfg,
        .size = sizeof(boot_cfg),
    },
    [MODULE_ID_CLOCK_CAL] = {
        .base = (uint32_t *) &clk_cal_cfg,
        .size = sizeof(clk_cal_cfg),
    },
    [MODULE_ID_CLOCK] = {
        .base = (uint32_t *) &clk_cfg,
        .size = sizeof(clk_cfg),
    },
    [MODULE_ID_CPU] = {
        .base = (uint32_t *) &cpu_cfg,
        .size = sizeof(cpu_cfg),
    },
#if (CONFIG_REALTEK_SINGLE_BANK_DFU == 1)
    [MODULE_ID_DFU] = {
        .base = (uint32_t *) &dfu_cfg,
        .size = sizeof(dfu_cfg),
    },
#endif
};

static void parse_boot_rom_from_occd(uint32_t occd_payload_addr, uint32_t occd_payload_size)
{
    if ((sizeof(boot_rom_cfg_info) / sizeof(MODULE_INFO)) ==
        MODULE_ID_BOOT_MAX - MODULE_ID_DBG)
    {
        PARSING_PARAMETER_FORMAT parsing_info = {
            .signature = SYS_CFG_SIGNATURE,
            .max_size  = occd_payload_size,
            .sys_cfg   = (void *)(occd_payload_addr),
        };
        for (size_t i = MODULE_ID_DBG; i < MODULE_ID_BOOT_MAX; i++)
        {
            if (parse_sys_cfg_to_mem(&parsing_info, i,
                                     (uint8_t *)boot_rom_cfg_info[i].base,
                                     boot_rom_cfg_info[i].size) > PARSE_ERROR_NO_CONFIG)
            {
                DIRECT_LOG("Parsing Error! CFG_Header:0x%x", i);
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * Public entry point
 * ----------------------------------------------------------------------- */

void rtl87x2j_platform_init(void)
{
    /* 1. Repair data RAM and buffer RAM using MBISR info from eFlash NVR */
    mbisr_with_repair_info();

    DBG_DIRECT(">>> RTL87x2J Platform Init (Zephyr)");

    /* 2. Enable assertion handler */
    assert_enable_set(true);

    /* 3. Update eFlash IRQ lock/unlock function pointers */
    eflash_assign_func_pointer();

    /* 4. RXI300 bus fabric init (conditional on AON OTP flag) */
    if (AON_REG_READ_BITFIELD(AON_REG_FW_GENERAL_REG6X, IS_RXI300_DISABLE) == 0)
    {
        rxi300_init();
    }

    /* 5. Parse ROM module configurations from OCCD flash partition */
    parse_boot_rom_from_occd(image_payload_addr_get(IMG_OCCD),
                             image_payload_size_get(IMG_OCCD));

    /* 6. ROT (Root-of-Trust) key and system control */
    rot_set_key_auto_load();
    rot_set_fpk();
    rot_system_general_control();

    /*
     * NOTE: secure_boot_entry() is intentionally omitted.
     * MCUboot handles image authentication and signature verification.
     */

    /* 7. ROT debug authentication and SWD access control */
    rot_set_debug_authentication_control();
    rot_swd_control(false);

    /* 8. OTP protection: configure read/write protection for each range */
    for (EFLASH_PROTECTION_RANGE_IDX_TYPE idx = EFLASH_PROTECTION_RANGE_0;
         idx < EFLASH_PROTECTION_RANGE_MAX; idx++)
    {
        uint16_t otp_offset_begin =
            ((uint16_t *)otp_ram->map.otp_protect_addr)[idx * 2];
        uint16_t otp_offset_end   =
            ((uint16_t *)otp_ram->map.otp_protect_addr)[idx * 2 + 1];
        otp_set_read_protect(idx, otp_offset_begin, otp_offset_end,
                             AON_REG_READ_BITFIELD(AON_REG_FW_GENERAL_REG14X,
                                                   IS_ENABLE_OTP_READ_PROTECT));
        otp_set_read_protect_lock(idx,
                                  AON_REG_READ_BITFIELD(AON_REG_FW_GENERAL_REG14X,
                                                        IS_OTP_READ_PROTECT_LOCK));
    }
    otp_set_write_protect(EFLASH_PROTECTION_RANGE_0,
                          0x0,
                          0x0 + OTP_TOTAL_SIZE - 1,
                          AON_REG_READ_BITFIELD(AON_REG_FW_GENERAL_REG14X,
                                               IS_ENABLE_OTP_WRITE_PROTECT));
    otp_set_write_protect_lock(EFLASH_PROTECTION_RANGE_0,
                               AON_REG_READ_BITFIELD(AON_REG_FW_GENERAL_REG14X,
                                                     IS_OTP_WRITE_PROTECT_LOCK));

    /* ======================================================================
     * SystemInit_zephyr() body — all steps except image_entry()
     * ==================================================================== */

    /* Disable IRQs until Zephyr scheduler starts (sync with FreeRTOS pre_main) */
    __disable_irq();

    /* 9. FT-OTP factory trim data init */
    ft_otp_init();

    /* 10. Platform power / clock subsystems */
    pck600_init();
    schedule_plan_init(); /* IRQ disabled: safe even in system-off wake path */

    si_flow_data_init();

    pmu_apply_voltage_tune();

    RCC_ClockCmd(RAP_CLOCK, ENABLE);
    RAP_Cmd(ENABLE);
    TM_ConvertTemperatureInit();

    /* 11. Oscillator calibration */
    clock_osc_cal_init();

    /* 12. Wire up RXI300 clock-rate query function pointer */
    update_rxi300_get_clk_unit_rate_function(clock_get_unit_rate);

    /* 13. Hardware and CPU setup (RAM power gating, MPU, FPU) */
    hal_setup_hardware();
    hal_setup_cpu();

    /* Set buffered-log output function pointer */
    extern void (*buf_output)(void);
    extern void print_buffered_log(void);
    buf_output = print_buffered_log;

    /*
     * 14. main_full() — with ZEPHYR_SUPPORT defined this simply sets
     *     buf_output and returns 0; wakeup / phy / amu init not performed
     *     here (Zephyr owns those subsystems).
     */
    extern int main_full(void);
    main_full();

    /* 15. Register hardware timer ISR handlers */
    extern void TIMER_IRQInit(void);
    TIMER_IRQInit();

    /*
     * NOTE: image_entry(IMG_MCUAPP, get_active_bank_image_addr_by_img_id(...))
     * is intentionally omitted.  MCUboot verifies and jumps to the Zephyr image.
     */

    DBG_DIRECT("<<< RTL87x2J Platform Init done — ready for Zephyr");
}
