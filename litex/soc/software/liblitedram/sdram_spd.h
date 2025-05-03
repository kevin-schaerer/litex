// This file is Copyright (c) 2023 Antmicro <www.antmicro.com>
// License: BSD

#ifndef __SDRAM_SPD_H
#define __SDRAM_SPD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <generated/csr.h>

bool sdram_read_spd(uint8_t spd, uint16_t addr, uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SDRAM_SPD_H */
