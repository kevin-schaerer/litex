// This file is Copyright (c) 2014-2021 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2018 Ewen McNeill <ewen@naos.co.nz>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2017 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2018 William D. Jones <thor0505@comcast.net>
// This file is Copyright (c) 2025 Kevin Schaerer <kevin.schaerer@nettimelogic.com>
// License: BSD

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <system.h>
#include <string.h>
#include <irq.h>

#include <generated/mem.h>
#include <generated/csr.h>
#include <generated/soc.h>

#include "boot.h"

#include <libbase/crc.h>
#include <libbase/uart.h>

/*-----------------------------------------------------------------------*/
/* Boot                                                                  */
/*-----------------------------------------------------------------------*/

extern void boot_helper(unsigned long r1, unsigned long r2, unsigned long r3, unsigned long addr);

void __attribute__((noreturn)) boot(unsigned long r1, unsigned long r2, unsigned long r3, unsigned long addr)
{
	printf("--============= \e[1mLiftoff!\e[0m ===============--\n");
#ifdef CSR_UART_BASE
	uart_sync();
#endif
#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(0);
#endif
	flush_cpu_icache();
	flush_cpu_dcache();
	flush_l2_cache();

	boot_helper(r1, r2, r3, addr);
	while(1);
}

enum {
	ACK_TIMEOUT,
	ACK_CANCELLED,
	ACK_OK
};

/*-----------------------------------------------------------------------*/
/* Flash Boot                                                            */
/*-----------------------------------------------------------------------*/

// This function is used to copy images from SPI flash to RAM.
// It checks the image length and CRC, and if valid, copies the image
// from the SPI flash to the specified RAM address.
// The function returns the length of the image if successful, or 0 if
// the image is invalid.
// The function assumes that the images are stored in a specific format
// in the SPI flash, with the first 4 bytes containing the length of the
// image, and the next 4 bytes containing the CRC32 checksum of the
// image data.
// MAIN_RAM_BASE is the base address of the RAM where the image will
// be copied.
// SPIFLASH_BASE is the base address of the SPI flash
// FLASH_MAPPING_OFFSET is the offset of the flash mapping table in the
// SPI flash.
#if defined(MAIN_RAM_BASE) && defined(SPIFLASH_BASE) && defined(FLASH_MAPPING_OFFSET)
static unsigned int check_image_in_flash(unsigned int base_address)
{
	uint32_t length;
	uint32_t crc;
	uint32_t got_crc;

	length = MMPTR(base_address);
	if((length < 32) || (length > 16*1024*1024)) {
		printf("Error: Invalid image length 0x%08x\n", length);
		return 0;
	}

	crc = MMPTR(base_address + 4);
	got_crc = crc32((unsigned char *)(base_address + 8), length);
	if(crc != got_crc) {
		printf("CRC failed (expected %08x, got %08x)\n", crc, got_crc);
		return 0;
	}

	return length;
}

static int copy_image_from_flash_to_ram(unsigned int flash_address, unsigned long ram_address)
{
	uint32_t length;
	uint32_t offset;

	length = check_image_in_flash(flash_address);
	if(length > 0) {
		printf("Copying 0x%08x to 0x%08lx (%d bytes)...\n", flash_address, ram_address, length);
		offset = 0;
		while (length > 0) {
			uint32_t chunk_length;
			chunk_length = min(length, 0x8000); /* 32KB chunks */
			memcpy((void *) ram_address + offset, (void*) flash_address + offset + 8, chunk_length);
			offset += chunk_length;
			length -= chunk_length;
		}
		printf("\n");
		return 1;
	}

	return 0;
}

void flashboot(void)
{
	uint32_t length, result, num_mappings;
	uint32_t src_addr, dst_addr, boot_addr = 0;
	uint32_t offset = 8;

	length = check_image_in_flash((unsigned int) SPIFLASH_BASE + FLASH_MAPPING_OFFSET);
	if (length == 0) {
		printf("Error: No mapping table available - aborting\n");
		return;
	}
	if (length % 8 != 0) {
		printf("Error: Invalid mapping table length 0x%08x\n", length);
		return;
	}

	num_mappings = length >> 3;

#if defined(FLASH_MAX_MAPPINGS)
	if (num_mappings > FLASH_MAX_MAPPINGS) {
		printf("Error: Mapping table too large (%d mappings)\n", num_mappings);
		return;
	}
#endif

	for (uint32_t i = 0; i < num_mappings; i++) {
		src_addr = MMPTR((unsigned int) SPIFLASH_BASE + FLASH_MAPPING_OFFSET + offset);
		dst_addr = MMPTR((unsigned int) SPIFLASH_BASE + FLASH_MAPPING_OFFSET + offset + 4);
		if (src_addr == 0) {
			printf("Warning: no more mappings\n");
			break;
		}

		if (src_addr < SPIFLASH_BASE || src_addr >= SPIFLASH_BASE + SPIFLASH_SIZE) {
			printf("Error: Source address 0x%08x is not in flash\n", src_addr);
			return;
		}
		if (dst_addr < MAIN_RAM_BASE || dst_addr >= MAIN_RAM_BASE + MAIN_RAM_SIZE) {
			printf("Error: Destination address 0x%08x is not in RAM\n", dst_addr);
			return;
		}

		if (i == 0) boot_addr = dst_addr;

		printf("Mapping %d: 0x%08x -> 0x%08x\n", i, src_addr, dst_addr);
		result = copy_image_from_flash_to_ram(src_addr, dst_addr);

		if (!result) {
			printf("Error: Failed to copy image from 0x%08x to 0x%08x\n", src_addr, dst_addr);
			return;
		}

		offset += 8;
	}

	if (boot_addr == 0) {
		printf("Error: No boot address found in mapping table\n");
		return;
	}
	boot(0, 0, 0, boot_addr);
}

#endif
