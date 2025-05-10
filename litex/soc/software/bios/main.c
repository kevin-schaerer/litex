// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2015 Yann Sionneau <ys@m-labs.hk>
// This file is Copyright (c) 2015 whitequark <whitequark@whitequark.org>
// This file is Copyright (c) 2019 Ambroz Bizjak <ambrop7@gmail.com>
// This file is Copyright (c) 2019 Caleb Jamison <cbjamo@gmail.com>
// This file is Copyright (c) 2018 Dolu1990 <charles.papon.90@gmail.com>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lambdaconcept.fr>
// This file is Copyright (c) 2018 Sergiusz Bazanski <q3k@q3k.org>
// This file is Copyright (c) 2016 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2020 Franck Jullien <franck.jullien@gmail.com>
// This file is Copyright (c) 2020 Antmicro <www.antmicro.com>
// This file is Copyright (c) 2025 Kevin Schaerer <kevin.schaerer@nettimelogic.com>

// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <irq.h>

#include "boot.h"
#include "helpers.h"

#include <generated/csr.h>
#include <generated/soc.h>
#include <generated/mem.h>
#include <generated/git.h>

#include <libbase/crc.h>
#include <libbase/memtest.h>

#include <libbase/uart.h>

#include <liblitedram/sdram.h>
#include <liblitedram/utils.h>

#include <libliteeth/udp.h>

#include <liblitespi/spiflash.h>

#ifndef CONFIG_BIOS_NO_BOOT
static void boot_sequence(void)
{
#if defined(SPIFLASH_BASE)
	flashboot();
#endif
	printf("No boot medium found\n");
}
#endif

__attribute__((__used__)) int main(int i, char **c)
{
	int sdr_ok;

#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
#ifdef CSR_UART_BASE
	uart_init();
#endif

#ifndef CONFIG_BIOS_NO_PROMPT
	printf(" Hive-S BIOS\n");
	crcbios();
	printf("\n");
#endif

    sdr_ok = 1;

#if defined(CSR_ETHMAC_BASE) || defined(MAIN_RAM_BASE) || defined(CSR_SPIFLASH_CORE_BASE)
    printf("--========== \e[1mInitialization\e[0m ============--\n");
#ifdef CSR_ETHMAC_BASE
	eth_init();
#endif

	/* Initialize and test DRAM */
#ifdef CSR_SDRAM_BASE
	sdr_ok = sdram_init();
#else
	/* Test Main RAM when present and not pre-initialized */
#ifdef MAIN_RAM_BASE
#ifndef CONFIG_MAIN_RAM_INIT
	sdr_ok = memtest((unsigned int *) MAIN_RAM_BASE, min(MAIN_RAM_SIZE, MEMTEST_DATA_SIZE));
	memspeed((unsigned int *) MAIN_RAM_BASE, min(MAIN_RAM_SIZE, MEMTEST_DATA_SIZE), false, 0);
#endif
#endif
#endif
	if (sdr_ok != 1)
		printf("Memory initialization failed\n");
#endif

	/* Initialize and test SPIFLASH */
#ifdef CSR_SPIFLASH_CORE_BASE
	spiflash_init();
#endif
	printf("\n");


	/* Initialize Video Framebuffer FIXME: Move */
#ifdef CSR_VIDEO_FRAMEBUFFER_BASE
	video_framebuffer_vtg_enable_write(0);
	video_framebuffer_dma_enable_write(0);
	video_framebuffer_vtg_enable_write(1);
	video_framebuffer_dma_enable_write(1);
#endif

	/* Execute  initialization functions */
	init_dispatcher();

	/* Execute Boot sequence */
#ifndef CONFIG_BIOS_NO_BOOT
	if(sdr_ok) {
		printf("--============== \e[1mBoot\e[0m ==================--\n");
		boot_sequence();
		printf("\n");
	}
#endif
	return 0;
}
