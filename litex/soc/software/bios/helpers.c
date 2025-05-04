// This file is Copyright (c) 2017 Florent Kermarrec <florent@enjoy-digital.fr>

// SPDX-License-Identifier: BSD-Source-Code

#include <stdio.h>
#include <string.h>

#include <libbase/crc.h>

#include "helpers.h"
#include "init.h"

extern unsigned int _ftext, _edata_rom;

#define NUMBER_OF_BYTES_ON_A_LINE 16
void crcbios(void)
{
	unsigned long offset_bios;
	unsigned long length;
	unsigned int expected_crc;
	unsigned int actual_crc;

	/*
	 * _edata_rom is located right after the end of the flat
	 * binary image. The CRC tool writes the 32-bit CRC here.
	 * We also use the address of _edata_rom to know the length
	 * of our code.
	 */
	offset_bios = (unsigned long)&_ftext;
	expected_crc = _edata_rom;
	length = (unsigned long)&_edata_rom - offset_bios;
	actual_crc = crc32((unsigned char *)offset_bios, length);
	if (expected_crc == actual_crc)
		printf(" BIOS CRC passed (%08x)\n", actual_crc);
	else {
		printf(" BIOS CRC failed (expected %08x, got %08x)\n", expected_crc, actual_crc);
		printf(" The system will continue, but expect problems.\n");
	}
}

void init_dispatcher(void)
{
	for (const init_func* fp = __bios_init_start; fp != __bios_init_end; fp++) {
		(*fp)();
	}
}
