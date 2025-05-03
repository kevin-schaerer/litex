// This file is Copyright (c) 2023 Antmicro <www.antmicro.com>
// License: BSD

#include <stdio.h>

#include <liblitedram/utils.h>
#include <liblitedram/sdram_spd.h>

#define KIB 1024
#define MIB (KIB*1024)
#define GIB (MIB*1024)

void print_size(uint64_t size) {
	if (size < KIB)
		printf("%" PRIu64 "B", size);
	else if (size < MIB)
		printf("%" PRIu64 ".%" PRIu64 "KiB", size/KIB, (size/1   - KIB*(size/KIB))/(KIB/10));
	else if (size < GIB)
		printf("%" PRIu64 ".%" PRIu64 "MiB", size/MIB, (size/KIB - KIB*(size/MIB))/(KIB/10));
	else
		printf("%" PRIu64 ".%" PRIu64 "GiB", size/GIB, (size/MIB - KIB*(size/GIB))/(KIB/10));
}

void print_progress(const char * header, uint64_t origin, uint64_t size)
{
	printf("%s 0x%" PRIx64 "-0x%" PRIx64 " ", header, origin, origin + size);
	print_size(size);
	printf("   \r");
}

#ifdef CSR_SDRAM_BASE

#include <generated/sdram_phy.h>

uint64_t sdram_get_supported_memory(void) {
	return SDRAM_PHY_SUPPORTED_MEMORY;
}

#endif
