/*
 * Copyright (C) 2017, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
						/* pin kz9021,	pin ar8035 */
#define GP_PHY_RD0	IMX_GPIO_NR(6, 25)	/* 32 MODE0,	29 AD0 */
#define GP_PHY_RD1	IMX_GPIO_NR(6, 27)	/* 31 MODE1,	28 AD1 */
#define GP_PHY_RD2	IMX_GPIO_NR(6, 28)	/* 28 MODE2,	26 MODE1 */
#define GP_PHY_RD3	IMX_GPIO_NR(6, 29)	/* 27 MODE3,	25 MODE3 */
#define GP_PHY_RX_CTL	IMX_GPIO_NR(6, 24)	/* 33 CLK125_EN, 30 MODE0 */
#define GP_PHY_RXC	IMX_GPIO_NR(6, 30)	/* 35 AD2,	31 1P8_SEL */

#ifndef STRAP_AR8035
#define STRAP_AR8035	(0x28 | (CONFIG_FEC_MXC_PHYADDR & 3))
#endif

#ifdef CONFIG_PHY_ATHEROS
static const iomux_v3_cfg_t enet_ar8035_gpio_pads[] = {
	IOMUX_PAD_CTRL(RGMII_RD0__GPIO6_IO25, PULL_GP(STRAP_AR8035, 0)),
	IOMUX_PAD_CTRL(RGMII_RD1__GPIO6_IO27, PULL_GP(STRAP_AR8035, 1)),
	/* mode = 1100 - plloff mode */
	IOMUX_PAD_CTRL(RGMII_RD2__GPIO6_IO28, PULL_GP(STRAP_AR8035, 2)),
	IOMUX_PAD_CTRL(RGMII_RD3__GPIO6_IO29, PULL_GP(STRAP_AR8035, 3)),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__GPIO6_IO24, PULL_GP(STRAP_AR8035, 4)),
	IOMUX_PAD_CTRL(RGMII_RXC__GPIO6_IO30, PULL_GP(STRAP_AR8035, 5)),
};

static const iomux_v3_cfg_t enet_ar8035_pads[] = {
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, PULL_ENET(STRAP_AR8035, 0)),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, PULL_ENET(STRAP_AR8035, 1)),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, PULL_ENET(STRAP_AR8035, 2)),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, PULL_ENET(STRAP_AR8035, 3)),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, PULL_ENET(STRAP_AR8035, 4)),
	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, PULL_ENET(STRAP_AR8035, 5)),
};
#endif

#define STRAP_KSZ9021	(0x1f | ((CONFIG_FEC_MXC_PHYADDR & 4) ? 0x20 : 0))

#ifdef CONFIG_PHY_MICREL_KSZ9021
static const iomux_v3_cfg_t enet_ksz9021_gpio_pads[] = {
	IOMUX_PAD_CTRL(RGMII_RD0__GPIO6_IO25, PULL_GP(STRAP_KSZ9021, 0)),
	IOMUX_PAD_CTRL(RGMII_RD1__GPIO6_IO27, PULL_GP(STRAP_KSZ9021, 1)),
	IOMUX_PAD_CTRL(RGMII_RD2__GPIO6_IO28, PULL_GP(STRAP_KSZ9021, 2)),
	IOMUX_PAD_CTRL(RGMII_RD3__GPIO6_IO29, PULL_GP(STRAP_KSZ9021, 3)),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__GPIO6_IO24, PULL_GP(STRAP_KSZ9021, 4)),
	IOMUX_PAD_CTRL(RGMII_RXC__GPIO6_IO30, PULL_GP(STRAP_KSZ9021, 4)),
};

static const iomux_v3_cfg_t enet_ksz9021_pads[] = {
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, PULL_ENET(STRAP_KSZ9021, 0)),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, PULL_ENET(STRAP_KSZ9021, 1)),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, PULL_ENET(STRAP_KSZ9021, 2)),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, PULL_ENET(STRAP_KSZ9021, 3)),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, PULL_ENET(STRAP_KSZ9021, 4)),
	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, PULL_ENET(STRAP_KSZ9021, 5)),
};
#endif
