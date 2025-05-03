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

#include "sfl.h"
#include "boot.h"

#include <libbase/crc.h>
#include <libbase/progress.h>
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

#ifdef FLASH_BOOT_ADDRESS

static unsigned int check_image_in_flash(unsigned int base_address)
{
	uint32_t length;
	uint32_t crc;
	uint32_t got_crc;

	length = MMPTR(base_address);
	if((length < 32) || (length > 16*1024*1024)) {
		printf("Error: Invalid image length 0x%08lx\n", length);
		return 0;
	}

	crc = MMPTR(base_address + 4);
	got_crc = crc32((unsigned char *)(base_address + 8), length);
	if(crc != got_crc) {
		printf("CRC failed (expected %08lx, got %08lx)\n", crc, got_crc);
		return 0;
	}

	return length;
}

#if defined(MAIN_RAM_BASE) && defined(FLASH_BOOT_ADDRESS)
static int copy_image_from_flash_to_ram(unsigned int flash_address, unsigned long ram_address, uint32_t length)
{
	//uint32_t length;
	uint32_t offset;

	//length = check_image_in_flash(flash_address);
	if(length > 0) {
		printf("Copying 0x%08x to 0x%08lx (%ld bytes)...\n", flash_address, ram_address, length);
		offset = 0;
		init_progression_bar(length);
		while (length > 0) {
			uint32_t chunk_length;
			chunk_length = min(length, 0x8000); /* 32KB chunks */
			memcpy((void *) ram_address + offset, (void*) flash_address + offset + 8, chunk_length);
			offset += chunk_length;
			length -= chunk_length;
			show_progress(offset);
		}
		show_progress(offset);
		printf("\n");
		return 1;
	}

	return 0;
}
#endif

#define KERNEL_IMAGE_OFFSET 0x0
#define ROOTFS_IMAGE_OFFSET 0x600000
#define OPENSBI_IMAGE_OFFSET 0xB00000
#define DTB_IMAGE_OFFSET 0xB50000

void flashboot(void)
{
	//uint32_t length;
	uint32_t result;

	//length = check_image_in_flash(FLASH_BOOT_ADDRESS);
	//if(!length)
	//	return;

	/* When Main RAM is available, copy the code from the Flash and execute it
	from Main RAM since faster */
	result = copy_image_from_flash_to_ram(FLASH_BOOT_ADDRESS + KERNEL_IMAGE_OFFSET, 0x40000000, 5724296);
	if(!result)
		return;
	result = copy_image_from_flash_to_ram(FLASH_BOOT_ADDRESS + ROOTFS_IMAGE_OFFSET, 0x41000000, 4925932);
	if(!result)
		return;
	result = copy_image_from_flash_to_ram(FLASH_BOOT_ADDRESS + OPENSBI_IMAGE_OFFSET, 0x40f00000, 263652);
	if(!result)
		return;
	result = copy_image_from_flash_to_ram(FLASH_BOOT_ADDRESS + DTB_IMAGE_OFFSET, 0x40ef0000, 3199);
	if(!result)
		return;
	boot(0, 0, 0, 0x40f00000);
}

#endif
