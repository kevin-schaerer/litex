// This file is Copyright (c) 2013 Werner Almesberger <werner@almesberger.net>
// This file is Copyright (c) 2014-2015 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lse.epita.fr>
// This file is Copyright (c) 2013 Robert Jordens <jordens@gmail.com>
// License: BSD

#include <generated/csr.h>

#include <stdio.h>

#ifdef CSR_ETHMAC_BASE

#include <libliteeth/udp.h>

void eth_init(void)
{
	printf("Ethernet init...\n");
#ifdef CSR_ETHPHY_CRG_RESET_ADDR
#ifndef ETH_PHY_NO_RESET
	ethphy_crg_reset_write(1);
	busy_wait(200);
	ethphy_crg_reset_write(0);
	busy_wait(200);
#endif
#endif
}

#endif
