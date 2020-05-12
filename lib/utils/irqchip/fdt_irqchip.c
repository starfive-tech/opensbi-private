/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_scratch.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>

extern struct fdt_irqchip fdt_irqchip_plic;

static struct fdt_irqchip *irqchip_drivers[] = {
	&fdt_irqchip_plic
};

static struct fdt_irqchip *current_driver = NULL;

void fdt_irqchip_exit(void)
{
	if (current_driver && current_driver->exit)
		current_driver->exit();
}

static int fdt_irqchip_warm_init(void)
{
	if (current_driver && current_driver->warm_init)
		return current_driver->warm_init();
	return 0;
}

static int fdt_irqchip_cold_init(void)
{
	int pos, noff, rc;
	struct fdt_irqchip *drv;
	const struct fdt_match *match;
	void *fdt = sbi_scratch_thishart_arg1_ptr();

	for (pos = 0; pos < array_size(irqchip_drivers); pos++) {
		drv = irqchip_drivers[pos];

		noff = fdt_find_match(fdt, -1, drv->match_table, &match);
		if (noff < 0)
			continue;

		if (drv->cold_init) {
			rc = drv->cold_init(fdt, noff, match);
			if (rc)
				return rc;
		}
		current_driver = drv;
		break;
	}

	return 0;
}

int fdt_irqchip_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = fdt_irqchip_cold_init();
		if (rc)
			return rc;
	}

	return fdt_irqchip_warm_init();
}
