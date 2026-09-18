#ifndef BOOT_PATCH_SECTION_H
#define BOOT_PATCH_SECTION_H

#define BOOT_PATCH_HEADER_SIZE 0x1000

#define SECTION(_name)                    __attribute__ ((__section__(_name)))
#define ENC_ALIGN_SECTION                 __attribute__((aligned(16), used, section(".enc.dummy.align")))

#define BOOT_PATCH_FLASH_HEADER           __attribute__((used, section(".boot_patch.flash.header")))
#define BOOT_PATCH_FLASH_START_SECTION    SECTION(".boot_patch.flash.start")
#define BOOT_PATCH_FLASH_TEXT_SECTION     SECTION(".boot_patch.flash.text")
#define BOOT_PATCH_RAM_START_SECTION      SECTION(".boot_patch.ram.start")
#define BOOT_PATCH_RAM_TEXT_SECTION       SECTION(".boot_patch.ram.text")

#endif
