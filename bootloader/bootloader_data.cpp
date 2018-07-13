#include "bootloader.h"

enum bootloader_type_ids
{
  ID_NONE = 0,
  ID_JRK_UMC04A_30V_BOOTLOADER,
  ID_JRK_UMC04A_40V_BOOTLOADER,
  ID_JRK_UMC05A_30V_BOOTLOADER,
  ID_JRK_UMC05A_40V_BOOTLOADER,
  ID_JRK_UMC06A_BOOTLOADER,
};

const std::vector<bootloader_type> bootloader_types = {
  {
    /* id */ ID_JRK_UMC04A_30V_BOOTLOADER,
    /* usb_vendor_id */ 0x1FFB,
    /* usb_product_id */ 0x00B6,
    /* name */ "Jrk G2 18v27 Bootloader",
    /* short_name */ "18v27",
    /* app_address */ 0x2000,
    /* app_size */ 0x6000,
    /* write_block_size */ 0x40,
    /* eeprom_address */ 0,
    /* eeprom_address_hex_file */ 0xF00000,
    /* eeprom_size */ 0x100,
  },
  {
    /* id */ ID_JRK_UMC04A_40V_BOOTLOADER,
    /* usb_vendor_id */ 0x1FFB,
    /* usb_product_id */ 0x00B8,
    /* name */ "Jrk G2 24v21 Bootloader",
    /* short_name */ "24v21",
    /* app_address */ 0x2000,
    /* app_size */ 0x6000,
    /* write_block_size */ 0x40,
    /* eeprom_address */ 0,
    /* eeprom_address_hex_file */ 0xF00000,
    /* eeprom_size */ 0x100,
  },
  {
    /* id */ ID_JRK_UMC05A_30V_BOOTLOADER,
    /* usb_vendor_id */ 0x1FFB,
    /* usb_product_id */ 0x00BE,
    /* name */ "Jrk G2 18v19 Bootloader",
    /* short_name */ "18v19",
    /* app_address */ 0x2000,
    /* app_size */ 0x6000,
    /* write_block_size */ 0x40,
    /* eeprom_address */ 0,
    /* eeprom_address_hex_file */ 0xF00000,
    /* eeprom_size */ 0x100,
  },
  {
    /* id */ ID_JRK_UMC05A_40V_BOOTLOADER,
    /* usb_vendor_id */ 0x1FFB,
    /* usb_product_id */ 0x00C0,
    /* name */ "Jrk G2 24v13 Bootloader",
    /* short_name */ "24v13",
    /* app_address */ 0x2000,
    /* app_size */ 0x6000,
    /* write_block_size */ 0x40,
    /* eeprom_address */ 0,
    /* eeprom_address_hex_file */ 0xF00000,
    /* eeprom_size */ 0x100,
  },
  {
    /* id */ ID_JRK_UMC06A_BOOTLOADER,
    /* usb_vendor_id */ 0x1FFB,
    /* usb_product_id */ 0x00C4,
    /* name */ "Jrk G2 TB9051FTG Bootloader",  // TODO: real name
    /* short_name */ "TB9051FTG",  // TODO: real name
    /* app_address */ 0x2000,
    /* app_size */ 0x6000,
    /* write_block_size */ 0x40,
    /* eeprom_address */ 0,
    /* eeprom_address_hex_file */ 0xF00000,
    /* eeprom_size */ 0x100,
  },
};
