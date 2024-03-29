/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/fbpanel.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <i2c.h>
#include <input.h>
#include <splash.h>
#include <usb/ehci-ci.h>
#include "../common/bd_common.h"
#include "../common/padctrl.h"

DECLARE_GLOBAL_DATA_PTR;

#define AUD_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define RGB_PAD_CTRL	PAD_CTL_DSE_120ohm

#define SPI_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

/* micro sd - 3.3v - 80ohm @ 1.8v settings is 40ohm at 3.3v */
#define USDHC1_PAD_CTRL	(PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

/* wifi - 1.8v */
#define USDHC2_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

/* eMMC - 1.8v */
#define USDHC4_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

/*
 *
 */
static const iomux_v3_cfg_t init_pads[] = {
	/* AUDMUX */
	IOMUX_PAD_CTRL(CSI0_DAT7__AUD3_RXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT4__AUD3_TXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT5__AUD3_TXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT6__AUD3_TXFS, AUD_PAD_CTRL),

	IOMUX_PAD_CTRL(DISP0_DAT23__AUD4_RXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT19__AUD4_RXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT18__AUD4_RXFS, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT20__AUD4_TXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT21__AUD4_TXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT22__AUD4_TXFS, AUD_PAD_CTRL),

	/* bt_rfkill */
#define GP_BT_RFKILL_RESET	IMX_GPIO_NR(6, 16)
	IOMUX_PAD_CTRL(NANDF_CS3__GPIO6_IO16, WEAK_PULLDN),

	/* ECSPI1 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI1_NOR_CS	IMX_GPIO_NR(3, 19)
	IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, WEAK_PULLUP),

	/* ECSPI2 - J55 db */
	IOMUX_PAD_CTRL(CSI0_DAT10__ECSPI2_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT9__ECSPI2_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT8__ECSPI2_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI2_SS0		IMX_GPIO_NR(5, 29)
	IOMUX_PAD_CTRL(CSI0_DAT11__GPIO5_IO29, WEAK_PULLUP),
#define GP_ECSPI2_SS1		IMX_GPIO_NR(2, 27)
	IOMUX_PAD_CTRL(EIM_LBA__GPIO2_IO27, WEAK_PULLUP),

	/* ECSPI3 */
	IOMUX_PAD_CTRL(DISP0_DAT2__ECSPI3_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT1__ECSPI3_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT0__ECSPI3_SCLK, SPI_PAD_CTRL),
	/* ECSPI3_SS0	XR20M1170IL16-F - spi uart */
#define GP_ECSPI3_UART		IMX_GPIO_NR(4, 24)
	IOMUX_PAD_CTRL(DISP0_DAT3__GPIO4_IO24, WEAK_PULLUP),

	IOMUX_PAD_CTRL(NANDF_CS2__CCM_CLKO2, WEAK_PULLDN),
#define GPIRQ_SPI_UART		IMX_GPIO_NR(2, 1)
	IOMUX_PAD_CTRL(NANDF_D1__GPIO2_IO01, WEAK_PULLUP),
#define GP_SPI_UART_RESET	IMX_GPIO_NR(2, 2)
	IOMUX_PAD_CTRL(NANDF_D2__GPIO2_IO02, WEAK_PULLDN),

	/* ENET pads that don't change for PHY reset */
	IOMUX_PAD_CTRL(ENET_MDIO__ENET_MDIO, PAD_CTRL_ENET_MDIO),
	IOMUX_PAD_CTRL(ENET_MDC__ENET_MDC, PAD_CTRL_ENET_MDC),
	IOMUX_PAD_CTRL(RGMII_TXC__RGMII_TXC, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII_TD0__RGMII_TD0, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII_TD1__RGMII_TD1, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII_TD2__RGMII_TD2, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII_TD3__RGMII_TD3, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII_TX_CTL__RGMII_TX_CTL, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(ENET_REF_CLK__ENET_TX_CLK, PAD_CTRL_ENET_TX),
	/* pin 42 PHY nRST */
#define GP_RGMII_PHY_RESET	IMX_GPIO_NR(1, 27)
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, WEAK_PULLDN),
#define GPIRQ_ENET_PHY		IMX_GPIO_NR(1, 28)
	IOMUX_PAD_CTRL(ENET_TX_EN__GPIO1_IO28, WEAK_PULLUP),

	/* FLEXCAN */
	IOMUX_PAD_CTRL(KEY_COL2__FLEXCAN1_TX, WEAK_PULLUP),
	IOMUX_PAD_CTRL(KEY_ROW2__FLEXCAN1_RX, WEAK_PULLUP),
#define GP_FLEXCAN_STANDBY	IMX_GPIO_NR(1, 2)
	IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, WEAK_PULLUP_OUTPUT),

	/* GPIO Outputs */
#define GP_GPO_1		IMX_GPIO_NR(3, 0)
	IOMUX_PAD_CTRL(EIM_DA0__GPIO3_IO00, WEAK_PULLUP),
#define GP_GPO_2		IMX_GPIO_NR(3, 1)
	IOMUX_PAD_CTRL(EIM_DA1__GPIO3_IO01, WEAK_PULLUP),
#define GP_GPO_3		IMX_GPIO_NR(3, 2)
	IOMUX_PAD_CTRL(EIM_DA2__GPIO3_IO02, WEAK_PULLUP),
#define GP_GPO_4		IMX_GPIO_NR(3, 3)
	IOMUX_PAD_CTRL(EIM_DA3__GPIO3_IO03, WEAK_PULLUP),
#define GP_GPO_5		IMX_GPIO_NR(3, 4)
	IOMUX_PAD_CTRL(EIM_DA4__GPIO3_IO04, WEAK_PULLUP),
#define GP_GPO_6		IMX_GPIO_NR(3, 5)
	IOMUX_PAD_CTRL(EIM_DA5__GPIO3_IO05, WEAK_PULLUP),
#define GP_GPO_7		IMX_GPIO_NR(3, 6)
	IOMUX_PAD_CTRL(EIM_DA6__GPIO3_IO06, WEAK_PULLUP),
#define GP_GPO_8		IMX_GPIO_NR(3, 7)
	IOMUX_PAD_CTRL(EIM_DA7__GPIO3_IO07, WEAK_PULLUP),

	/* GPIO Inputs */
#define GP_GPI_1		IMX_GPIO_NR(3, 8)
	IOMUX_PAD_CTRL(EIM_DA8__GPIO3_IO08, WEAK_PULLUP),
#define GP_GPI_2		IMX_GPIO_NR(3, 9)
	IOMUX_PAD_CTRL(EIM_DA9__GPIO3_IO09, WEAK_PULLUP),
#define GP_GPI_3		IMX_GPIO_NR(3, 10)
	IOMUX_PAD_CTRL(EIM_DA10__GPIO3_IO10, WEAK_PULLUP),
#define GP_GPI_4		IMX_GPIO_NR(3, 11)
	IOMUX_PAD_CTRL(EIM_DA11__GPIO3_IO11, WEAK_PULLUP),
#define GP_GPI_5		IMX_GPIO_NR(3, 12)
	IOMUX_PAD_CTRL(EIM_DA12__GPIO3_IO12, WEAK_PULLUP),
#define GP_GPI_6		IMX_GPIO_NR(3, 13)
	IOMUX_PAD_CTRL(EIM_DA13__GPIO3_IO13, WEAK_PULLUP),
#define GP_GPI_7		IMX_GPIO_NR(3, 14)
	IOMUX_PAD_CTRL(EIM_DA14__GPIO3_IO14, WEAK_PULLUP),
#define GP_GPI_8		IMX_GPIO_NR(3, 15)
	IOMUX_PAD_CTRL(EIM_DA15__GPIO3_IO15, WEAK_PULLUP),
#define GP_GPI_9		IMX_GPIO_NR(4, 15)
	IOMUX_PAD_CTRL(KEY_ROW4__GPIO4_IO15, WEAK_PULLUP),
#define GP_GPI_10		IMX_GPIO_NR(4, 14)
	IOMUX_PAD_CTRL(KEY_COL4__GPIO4_IO14, WEAK_PULLUP),
#define GP_GPI_11		IMX_GPIO_NR(1, 4)
	IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, WEAK_PULLUP),
#define GP_GPI_12		IMX_GPIO_NR(4, 7)
	IOMUX_PAD_CTRL(KEY_ROW0__GPIO4_IO07, WEAK_PULLUP),
#define GP_GPI_13		IMX_GPIO_NR(1, 7)
	IOMUX_PAD_CTRL(GPIO_7__GPIO1_IO07, WEAK_PULLUP),
#define GP_GPI_14		IMX_GPIO_NR(3, 29)
	IOMUX_PAD_CTRL(EIM_D29__GPIO3_IO29, WEAK_PULLUP),
#define GP_GPI_15		IMX_GPIO_NR(2, 25)
	IOMUX_PAD_CTRL(EIM_OE__GPIO2_IO25, WEAK_PULLUP),
#define GP_GPI_16		IMX_GPIO_NR(2, 26)
	IOMUX_PAD_CTRL(EIM_RW__GPIO2_IO26, WEAK_PULLUP),
#define GP_GPI_EN_N		IMX_GPIO_NR(2, 31)
	IOMUX_PAD_CTRL(EIM_EB3__GPIO2_IO31, WEAK_PULLUP),

	/* GPIO test points */
#define GP_TP71			IMX_GPIO_NR(1, 30)
	IOMUX_PAD_CTRL(ENET_TXD0__GPIO1_IO30, WEAK_PULLUP),
#define GP_TP72			IMX_GPIO_NR(4, 8)
	IOMUX_PAD_CTRL(KEY_COL1__GPIO4_IO08, WEAK_PULLUP),
#define GP_TP73			IMX_GPIO_NR(4, 9)
	IOMUX_PAD_CTRL(KEY_ROW1__GPIO4_IO09, WEAK_PULLUP),
#define GP_TP74			IMX_GPIO_NR(2, 7)
	IOMUX_PAD_CTRL(NANDF_D7__GPIO2_IO07, WEAK_PULLUP),
#define GP_TP84			IMX_GPIO_NR(1, 3)
	IOMUX_PAD_CTRL(GPIO_3__GPIO1_IO03, WEAK_PULLUP),
#define GP_TP88			IMX_GPIO_NR(4, 5)
	IOMUX_PAD_CTRL(GPIO_19__GPIO4_IO05, WEAK_PULLUP),
#define GP_TP89			IMX_GPIO_NR(7, 8)
	IOMUX_PAD_CTRL(SD3_RST__GPIO7_IO08, WEAK_PULLUP),
#define GP_TP95			IMX_GPIO_NR(2, 30)
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, WEAK_PULLUP),
#define GP_LVDS_CTRL		IMX_GPIO_NR(2, 0)
	IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP),


	/* i2c1_rv4162 rtc */
#define GPIRQ_RTC_RV4162	IMX_GPIO_NR(4, 6)
	IOMUX_PAD_CTRL(KEY_COL0__GPIO4_IO06, WEAK_PULLUP),

	/* i2c1_sgtl5000 */
	IOMUX_PAD_CTRL(GPIO_0__CCM_CLKO1, OUTPUT_40OHM),	/* SGTL5000 sys_mclk */

#define GPIRQ_I2C3A_J6		IMX_GPIO_NR(7, 13)
	IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, BUTTON_PAD_CTRL),
#define GP_I2C3A_J6_RESET		IMX_GPIO_NR(1, 8)
	IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, WEAK_PULLUP),

	/* i2c3a - J6 lvds connector (behind PCA9540B mux) */
#define GPIRQ_LIGHT_SENSOR	IMX_GPIO_NR(7, 6)
	IOMUX_PAD_CTRL(SD3_DAT2__GPIO7_IO06, WEAK_PULLUP),

	/* PCIe */
#define GP_PCIE_RESET		IMX_GPIO_NR(6, 31)
	IOMUX_PAD_CTRL(EIM_BCLK__GPIO6_IO31, WEAK_PULLDN),
#define GP_PCIE_DISABLE		IMX_GPIO_NR(2, 28)
	IOMUX_PAD_CTRL(EIM_EB0__GPIO2_IO28, WEAK_PULLDN),

	/* PWM1 - J55, pin B30 */
#define GP_J55_PWM1		IMX_GPIO_NR(1, 21)
	IOMUX_PAD_CTRL(GPIO_9__GPIO1_IO09, WEAK_PULLDN),

	/* PWM2 - Backlight on LVDS2 connector: J6, pin 30 */
#define GP_BACKLIGHT_LVDS	IMX_GPIO_NR(1, 1)
	IOMUX_PAD_CTRL(GPIO_1__GPIO1_IO01, WEAK_PULLDN),

	/* reg_usbotg_vbus */
#define GP_REG_USBOTG		IMX_GPIO_NR(3, 22)
	IOMUX_PAD_CTRL(EIM_D22__GPIO3_IO22, WEAK_PULLDN),

	/* reg_wlan_en */
#define GP_REG_WLAN_EN		IMX_GPIO_NR(2, 5)
	IOMUX_PAD_CTRL(NANDF_D5__GPIO2_IO05, WEAK_PULLDN),

	/* UART1 */
	IOMUX_PAD_CTRL(SD3_DAT7__UART1_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT6__UART1_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT0__GPIO7_IO04, WEAK_PULLUP),
#define GP_UART1_TX_EN		IMX_GPIO_NR(7, 5)
	IOMUX_PAD_CTRL(SD3_DAT1__GPIO7_IO05, WEAK_PULLDN_OUTPUT),

	/* UART2 */
	IOMUX_PAD_CTRL(EIM_D26__UART2_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__UART2_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CMD__UART2_CTS_B, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CLK__UART2_RTS_B, UART_PAD_CTRL),

	/* UART3 */
	IOMUX_PAD_CTRL(EIM_D24__UART3_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D25__UART3_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, WEAK_PULLUP),
#define GP_UART3_TX_EN		IMX_GPIO_NR(3, 31)
	IOMUX_PAD_CTRL(EIM_D31__GPIO3_IO31, WEAK_PULLDN_OUTPUT),

	/* UART4 */
	IOMUX_PAD_CTRL(CSI0_DAT12__UART4_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT13__UART4_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT17__GPIO6_IO03, WEAK_PULLUP),
#define GP_UART4_TX_EN		IMX_GPIO_NR(6, 2)
	IOMUX_PAD_CTRL(CSI0_DAT16__GPIO6_IO02, WEAK_PULLDN_OUTPUT),

	/* UART5 */
	IOMUX_PAD_CTRL(CSI0_DAT14__UART5_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT15__UART5_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT19__GPIO6_IO05, WEAK_PULLUP),
#define GP_UART5_TX_EN		IMX_GPIO_NR(6, 4)
	IOMUX_PAD_CTRL(CSI0_DAT18__GPIO6_IO04, WEAK_PULLDN_OUTPUT),

	/* USBH1 */
	IOMUX_PAD_CTRL(EIM_D30__USB_H1_OC, WEAK_PULLUP),
#define GP_USB_HUB_RESET	IMX_GPIO_NR(7, 12)
	IOMUX_PAD_CTRL(GPIO_17__GPIO7_IO12, WEAK_PULLDN),

	/* USBOTG */
	IOMUX_PAD_CTRL(ENET_RX_ER__USB_OTG_ID, WEAK_PULLUP),
//	IOMUX_PAD_CTRL(KEY_COL4__USB_OTG_OC, WEAK_PULLUP),
	IOMUX_PAD_CTRL(EIM_WAIT__GPIO5_IO00, WEAK_PULLUP),

	/* USDHC1  */
	IOMUX_PAD_CTRL(SD1_CLK__SD1_CLK, USDHC1_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_CMD__SD1_CMD, USDHC1_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT0__SD1_DATA0, USDHC1_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT1__SD1_DATA1, USDHC1_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT2__SD1_DATA2, USDHC1_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT3__SD1_DATA3, USDHC1_PAD_CTRL),
#define GP_USDHC1_CD		IMX_GPIO_NR(7, 0)
	IOMUX_PAD_CTRL(SD3_DAT5__GPIO7_IO00, WEAK_PULLUP),

	/* USDHC2 - silex/TiWi wl1271 */
	IOMUX_PAD_CTRL(SD2_CLK__SD2_CLK, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CMD__SD2_CMD, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT0__SD2_DATA0, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT1__SD2_DATA1, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT2__SD2_DATA2, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT3__SD2_DATA3, USDHC2_PAD_CTRL),

	/* USDHC4 - emmc */
	IOMUX_PAD_CTRL(SD4_CLK__SD4_CLK, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_CMD__SD4_CMD, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT0__SD4_DATA0, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT1__SD4_DATA1, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT2__SD4_DATA2, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT3__SD4_DATA3, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT4__SD4_DATA4, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT5__SD4_DATA5, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT6__SD4_DATA6, USDHC4_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT7__SD4_DATA7, USDHC4_PAD_CTRL),
#define GP_EMMC_RESET		IMX_GPIO_NR(2, 6)
	IOMUX_PAD_CTRL(NANDF_D6__GPIO2_IO06, WEAK_PULLUP),

	/* silex/wl1271 */
#define GPIRQ_WIFI		IMX_GPIO_NR(6, 11)
	IOMUX_PAD_CTRL(NANDF_CS0__GPIO6_IO11, WEAK_PULLDN),
#define GP_WIFI_QOW		IMX_GPIO_NR(2, 3)
	IOMUX_PAD_CTRL(NANDF_D3__GPIO2_IO03, WEAK_PULLUP),
#define GP_WIFI_CLK_REQ		IMX_GPIO_NR(2, 3)
	IOMUX_PAD_CTRL(NANDF_D4__GPIO2_IO04, WEAK_PULLUP),
	/* TiWi only */
#define GPIRQ_BT_HOST_WAKE	IMX_GPIO_NR(6, 10)
	IOMUX_PAD_CTRL(NANDF_RB0__GPIO6_IO10, WEAK_PULLDN),
};

static const struct i2c_pads_info i2c_pads[] = {
	/* I2C1, SGTL5000, RV4162 rtc */
	I2C_PADS_INFO_ENTRY(I2C1, EIM_D21, 3, 21, EIM_D28, 3, 28, I2C_PAD_CTRL),
	/* I2C2, J55 connector to hp board */
	I2C_PADS_INFO_ENTRY(I2C2, KEY_COL3, 4, 12, KEY_ROW3, 4, 13, I2C_PAD_CTRL),
	/* I2C3, PCA9540B(mux), I2C3a, J6 LVDS, I2C3b, PCIe connector */
	I2C_PADS_INFO_ENTRY(I2C3, GPIO_5, 1, 05, GPIO_16, 7, 11, I2C_PAD_CTRL),
};
#define I2C_BUS_CNT	3

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	if (port) {
		/* Reset USB hub */
		gpio_direction_output(GP_USB_HUB_RESET, 0);
		mdelay(2);
		gpio_set_value(GP_USB_HUB_RESET, 1);
	}
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	if (!on) {
		gpio_set_value(GP_REG_USBOTG, on);
		return 0;
	}
	if (otg_power_detect()) {
		gpio_set_value(GP_REG_USBOTG, 0);
		mdelay(100);
		if (otg_power_detect())
			return 0;
	}
	gpio_set_value(GP_REG_USBOTG, on);
	return 0;
}

#endif

#ifdef CONFIG_SYS_BOOT_BOARD_POWER_CHECK
void board_power_check()
{
	int i = 0;

	while (1) {
		if (!otg_power_detect())
			break;
		if (!i) {
			gpio_set_value(GP_REG_USBOTG, 0);
		} else {
			printf("Please disconnect otg cable\n");

		}
		i++;
		mdelay(1000);
	}
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg board_usdhc_cfg[] = {
	{.esdhc_base = USDHC1_BASE_ADDR, .bus_width = 4,
			.gp_cd = GP_USDHC1_CD},
	{.esdhc_base = USDHC4_BASE_ADDR, .bus_width = 8,
			.gp_reset = GP_EMMC_RESET},
};
#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? GP_ECSPI1_NOR_CS : -1;
}
#endif

#ifdef CONFIG_CMD_FBPANEL
void board_enable_lvds(const struct display_info_t *di, int enable)
{
	gpio_direction_output(GP_BACKLIGHT_LVDS, enable);
}

static const struct display_info_t displays[] = {
	VD_AFK1024600A02(LVDS, fbp_detect_i2c, (GP_I2C3A_J6_RESET << 8) | 2, 0x4a),

	/* ft5x06 */
	VD_HANNSTAR7(LVDS, fbp_detect_i2c, 2, 0x38),
	VD_AUO_B101EW05(LVDS, NULL, 2, 0x38),
	VD_LG1280_800(LVDS, NULL, 2, 0x38),
	VD_DT070BTFT(LVDS, NULL, 2, 0x38),
	VD_WSVGA(LVDS, NULL, 2, 0x38),
	VD_TM070JDHG30(LVDS, NULL, 2, 0x38),

	/* ili210x */
	VD_AMP1024_600(LVDS, fbp_detect_i2c, 2, 0x41),

	/* egalax_ts */
	VD_HANNSTAR(LVDS, fbp_detect_i2c, 2, 0x04),
	VD_LG9_7(LVDS, NULL, 2, 0x04),

	VD_SHARP_LQ101K1LY04(LVDS, NULL, 0, 0x00),
	VD_WXGA(LVDS, NULL, 0, 0x00),
	VD_LD070WSVGA(LVDS, NULL, 0, 0x00),
	VD_WVGA(LVDS, NULL, 0, 0x00),
	VD_AA065VE11(LVDS, NULL, 0, 0x00),
	VD_VGA(LVDS, NULL, 0, 0x00),
};
#define display_cnt	ARRAY_SIZE(displays)
#else
#define displays	NULL
#define display_cnt	0
#endif

static const unsigned short gpios_out_low[] = {
	GP_BT_RFKILL_RESET, 	/* disable bluetooth */
	GP_SPI_UART_RESET,
	GP_RGMII_PHY_RESET,
	GP_GPO_1,
	GP_GPO_2,
	GP_GPO_3,
	GP_GPO_4,
	GP_GPO_5,
	GP_GPO_6,
	GP_GPO_7,
	GP_GPO_8,
	GP_I2C3A_J6_RESET,
	GP_PCIE_RESET,
	GP_J55_PWM1,
	GP_BACKLIGHT_LVDS,
	GP_REG_USBOTG,		/* disable USB otg power */
	GP_REG_WLAN_EN,		/* disable wireless */
	GP_USB_HUB_RESET,	/* disable hub */
	GP_EMMC_RESET,		/* hold in reset */
	GP_UART1_TX_EN,
	GP_UART3_TX_EN,
	GP_UART4_TX_EN,
	GP_UART5_TX_EN,
};

static const unsigned short gpios_out_high[] = {
	GP_ECSPI1_NOR_CS,	/* SS1 of spi nor */
	GP_ECSPI2_SS0,
	GP_ECSPI2_SS1,
	GP_ECSPI3_UART,
	GP_FLEXCAN_STANDBY,
};

static const unsigned short gpios_in[] = {
	GPIRQ_SPI_UART,
	GPIRQ_ENET_PHY,
	GP_GPI_1,
	GP_GPI_2,
	GP_GPI_3,
	GP_GPI_4,
	GP_GPI_5,
	GP_GPI_6,
	GP_GPI_7,
	GP_GPI_8,
	GP_GPI_9,
	GP_GPI_10,
	GP_GPI_11,
	GP_GPI_12,
	GP_GPI_13,
	GP_GPI_14,
	GP_GPI_15,
	GP_GPI_16,
	GP_GPI_EN_N,
	GP_TP71,
	GP_TP72,
	GP_TP73,
	GP_TP74,
	GP_TP84,
	GP_TP88,
	GP_TP89,
	GP_TP95,
	GPIRQ_RTC_RV4162,
	GPIRQ_I2C3A_J6,
	GP_LVDS_CTRL,
	GPIRQ_LIGHT_SENSOR,
	GP_PCIE_DISABLE,
	GP_USDHC1_CD,
	GPIRQ_WIFI,
	GP_WIFI_QOW,
	GP_WIFI_CLK_REQ,
	GPIRQ_BT_HOST_WAKE
};

int board_early_init_f(void)
{
	set_gpios_in(gpios_in, ARRAY_SIZE(gpios_in));
	set_gpios(gpios_out_high, ARRAY_SIZE(gpios_out_high), 1);
	set_gpios(gpios_out_low, ARRAY_SIZE(gpios_out_low), 0);
	SETUP_IOMUX_PADS(init_pads);
	return 0;
}

int board_init(void)
{
	common_board_init(i2c_pads, I2C_BUS_CNT, IOMUXC_GPR1_OTG_ID_ENET_RX_ERR,
			displays, display_cnt, 0);
	return 0;
}

const struct button_key board_buttons[] = {
	{"tp71",	GP_TP71,	'T', 1},
	{NULL, 0, 0, 0},
};

#ifdef CONFIG_CMD_BMODE
const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},	/* 8-bit eMMC */
	{NULL,		0},
};
#endif
