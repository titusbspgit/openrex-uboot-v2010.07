/*
 * Copyright (C) 2017, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define GP_PHY_RD0	IMX_GPIO_NR(5, 0)
#define GP_PHY_RD1	IMX_GPIO_NR(5, 1)
#define GP_PHY_RD2	IMX_GPIO_NR(5, 2)
#define GP_PHY_RD3	IMX_GPIO_NR(5, 3)
#define GP_PHY_RX_CTL	IMX_GPIO_NR(5, 4)
#define GP_PHY_RXC	IMX_GPIO_NR(5, 5)

#define GP_PHY2_RD0	IMX_GPIO_NR(5, 12)
#define GP_PHY2_RD1	IMX_GPIO_NR(5, 13)
#define GP_PHY2_RD2	IMX_GPIO_NR(5, 14)
#define GP_PHY2_RD3	IMX_GPIO_NR(5, 15)
#define GP_PHY2_RX_CTL	IMX_GPIO_NR(5, 16)
#define GP_PHY2_RXC	IMX_GPIO_NR(5, 17)

#ifndef STRAP_AR8035
#define STRAP_AR8035	((0x28 | (CONFIG_FEC_MXC_PHYADDR & 3)) | ((0x28 | ((CONFIG_FEC_MXC_PHYADDR + 1) & 3)) << 6))
#endif

#ifdef CONFIG_PHY_ATHEROS
static const iomux_v3_cfg_t enet_ar8035_gpio_pads[] = {
	IOMUX_PAD_CTRL(RGMII1_RD0__GPIO5_IO_0, PULL_GP(STRAP_AR8035, 0)),
	IOMUX_PAD_CTRL(RGMII1_RD1__GPIO5_IO_1, PULL_GP(STRAP_AR8035, 1)),
	IOMUX_PAD_CTRL(RGMII1_RD2__GPIO5_IO_2, PULL_GP(STRAP_AR8035, 2)),
	IOMUX_PAD_CTRL(RGMII1_RD3__GPIO5_IO_3, PULL_GP(STRAP_AR8035, 3)),
	IOMUX_PAD_CTRL(RGMII1_RX_CTL__GPIO5_IO_4, PULL_GP(STRAP_AR8035, 4)),
	/* 1.8V(1)/1.5V select(0) */
	IOMUX_PAD_CTRL(RGMII1_RXC__GPIO5_IO_5, PULL_GP(STRAP_AR8035, 5)),

	IOMUX_PAD_CTRL(RGMII2_RD0__GPIO5_IO_12, PULL_GP(STRAP_AR8035, 6)),
	IOMUX_PAD_CTRL(RGMII2_RD1__GPIO5_IO_13, PULL_GP(STRAP_AR8035, 7)),
	IOMUX_PAD_CTRL(RGMII2_RD2__GPIO5_IO_14, PULL_GP(STRAP_AR8035, 8)),
	/* MODE 2 is LED_100, Internal pull up */
	IOMUX_PAD_CTRL(RGMII2_RD3__GPIO5_IO_15, PULL_GP(STRAP_AR8035, 9)),
	IOMUX_PAD_CTRL(RGMII2_RX_CTL__GPIO5_IO_16, PULL_GP(STRAP_AR8035, 10)),
	/* 1.8V(1)/1.5V select(0) */
	IOMUX_PAD_CTRL(RGMII2_RXC__GPIO5_IO_17, PULL_GP(STRAP_AR8035, 11)),
};

static const iomux_v3_cfg_t enet_ar8035_pads[] = {
	IOMUX_PAD_CTRL(RGMII1_RD0__ENET1_RX_DATA_0, PULL_ENET(STRAP_AR8035, 0)),
	IOMUX_PAD_CTRL(RGMII1_RD1__ENET1_RX_DATA_1, PULL_ENET(STRAP_AR8035, 1)),
	IOMUX_PAD_CTRL(RGMII1_RD2__ENET1_RX_DATA_2, PULL_ENET(STRAP_AR8035, 2)),
	IOMUX_PAD_CTRL(RGMII1_RD3__ENET1_RX_DATA_3, PULL_ENET(STRAP_AR8035, 3)),
	IOMUX_PAD_CTRL(RGMII1_RX_CTL__ENET1_RX_EN, PULL_ENET(STRAP_AR8035, 4)),
	IOMUX_PAD_CTRL(RGMII1_RXC__ENET1_RX_CLK, PULL_ENET(STRAP_AR8035, 5)),

	IOMUX_PAD_CTRL(RGMII2_RD0__ENET2_RX_DATA_0, PULL_ENET(STRAP_AR8035, 6)),
	IOMUX_PAD_CTRL(RGMII2_RD1__ENET2_RX_DATA_1, PULL_ENET(STRAP_AR8035, 7)),
	IOMUX_PAD_CTRL(RGMII2_RD2__ENET2_RX_DATA_2, PULL_ENET(STRAP_AR8035, 8)),
	IOMUX_PAD_CTRL(RGMII2_RD3__ENET2_RX_DATA_3, PULL_ENET(STRAP_AR8035, 9)),
	IOMUX_PAD_CTRL(RGMII2_RX_CTL__ENET2_RX_EN, PULL_ENET(STRAP_AR8035, 10)),
	IOMUX_PAD_CTRL(RGMII2_RXC__ENET2_RX_CLK, PULL_ENET(STRAP_AR8035, 11)),
};
#endif
