/*
 * Copyright (C) 2017, Boundary Devices <info@boundarydevices.com>
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

/* Special MXCFB sync flags are here. */
#include "../drivers/video/mxcfb.h"

DECLARE_GLOBAL_DATA_PTR;

#define AUD_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define RGB_PAD_CTRL	PAD_CTL_DSE_120ohm

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t init_pads[] = {
	/* Audmux 3 */
	IOMUX_PAD_CTRL(CSI0_DAT7__AUD3_RXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT4__AUD3_TXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT5__AUD3_TXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT6__AUD3_TXFS, AUD_PAD_CTRL),

	/* Audmux 4 */
	IOMUX_PAD_CTRL(DISP0_DAT20__AUD4_TXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT21__AUD4_TXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT22__AUD4_TXFS, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT23__AUD4_RXD, AUD_PAD_CTRL),

	/* bt_rfkill */
#define GP_BT_RFKILL_RESET	IMX_GPIO_NR(6, 16)
	IOMUX_PAD_CTRL(NANDF_CS3__GPIO6_IO16, WEAK_PULLDN),

	/* ECSPI1 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI1_NOR_CS	IMX_GPIO_NR(3, 19)
	IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, SPI_PAD_CTRL),

	/* ECSPI2 */
	IOMUX_PAD_CTRL(CSI0_DAT10__ECSPI2_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT9__ECSPI2_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT8__ECSPI2_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI2_CS		IMX_GPIO_NR(5, 29)
	IOMUX_PAD_CTRL(CSI0_DAT11__GPIO5_IO29, SPI_PAD_CTRL),

#define GP_GPIOKEY_POWER	IMX_GPIO_NR(3, 1)
	IOMUX_PAD_CTRL(EIM_DA1__GPIO3_IO01, WEAK_PULLUP),
	/* Rev 3 */
#define GP_GPIOKEY_RESET	IMX_GPIO_NR(6, 18)
	IOMUX_PAD_CTRL(SD3_DAT6__GPIO6_IO18, WEAK_PULLUP),
	/* Rev 1, not on Rev 0*/
	/* Goes low when USB_OTG_VBUS goes high */
#define GP_GPIOKEY_VBUS_STATUS	IMX_GPIO_NR(3, 3)
	IOMUX_PAD_CTRL(EIM_DA3__GPIO3_IO03, WEAK_PULLUP),

	/* GPS */
#define GP_GPS_HEARTBEAT	IMX_GPIO_NR(7, 1)
	IOMUX_PAD_CTRL(SD3_DAT4__GPIO7_IO01, WEAK_PULLUP),
#define GP_GPS_INT		IMX_GPIO_NR(7, 13)
	IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, WEAK_PULLUP),
#define GP_GPS_RESET		IMX_GPIO_NR(4, 5)
	IOMUX_PAD_CTRL(GPIO_19__GPIO4_IO05, WEAK_PULLUP),
	/* Modem pins */
#define GP_SLEEP_STAT		IMX_GPIO_NR(4, 22)
	IOMUX_PAD_CTRL(DISP0_DAT1__GPIO4_IO22, WEAK_PULLUP),
#define GP_MODEM_RESET		IMX_GPIO_NR(4, 23)
	IOMUX_PAD_CTRL(DISP0_DAT2__GPIO4_IO23, WEAK_PULLUP),

#ifdef CONFIG_REV_OLD
	/* Polarity has changed between revs */
#define LED_ACTIVE_BLUE		0
#define LED_ACTIVE_GREEN	0
#define LED_ACTIVE_RED		0
#define PULL_BLUE		WEAK_PULLUP
#define PULL_GREEN		WEAK_PULLUP
#define PULL_RED		WEAK_PULLUP
#else
#define LED_ACTIVE_BLUE		1		/* Blue - high active */
#ifdef CONFIG_GREEN_HIGH
#define LED_ACTIVE_GREEN	1		/* Green - cid, high active */
#else
#define LED_ACTIVE_GREEN	0		/* Green - cid2, low active */
#endif
#define LED_ACTIVE_RED		1		/* Red - high active */
#define PULL_BLUE		WEAK_PULLDN
#define PULL_GREEN		WEAK_PULLUP
#define PULL_RED		WEAK_PULLDN
#endif

#define GP_LED_BLUE		IMX_GPIO_NR(4, 26)
	IOMUX_PAD_CTRL(DISP0_DAT5__GPIO4_IO26, PULL_BLUE),
	/* Green led on means USB powered, not charging */
#define GP_LED_GREEN		IMX_GPIO_NR(4, 25)
	IOMUX_PAD_CTRL(DISP0_DAT4__GPIO4_IO25, PULL_GREEN),
#define GP_LED_CTRL_GREEN	IMX_GPIO_NR(3, 7)
	IOMUX_PAD_CTRL(EIM_DA7__GPIO3_IO07, OUTPUT_40OHM),
	/* Red led on means USB powered, charging */
#define GP_LED_RED		IMX_GPIO_NR(4, 27)
	IOMUX_PAD_CTRL(DISP0_DAT6__GPIO4_IO27, PULL_RED),
#define GP_LED_CTRL_RED		IMX_GPIO_NR(2, 30)
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, OUTPUT_40OHM),
#define GP_MODEM_ON_OFF		IMX_GPIO_NR(4, 29)
	IOMUX_PAD_CTRL(DISP0_DAT8__GPIO4_IO29, WEAK_PULLUP),
	/* SIM */
#define GP_SIM_DETECT		IMX_GPIO_NR(2, 19)
	IOMUX_PAD_CTRL(EIM_A19__GPIO2_IO19, WEAK_PULLUP),
	/* Regulators */
#define GP_3P7_BYPASS_EN	IMX_GPIO_NR(2, 18)
	IOMUX_PAD_CTRL(EIM_A20__GPIO2_IO18, WEAK_PULLUP),
#define GP_3P7_EN		IMX_GPIO_NR(5, 11)
	IOMUX_PAD_CTRL(DISP0_DAT17__GPIO5_IO11, WEAK_PULLUP),
#define GP_REG_2P8V_EN		IMX_GPIO_NR(3, 9)
	IOMUX_PAD_CTRL(EIM_DA9__GPIO3_IO09, WEAK_PULLDN),
	/* Vibrator */
#define GP_VIBRATOR_EN		IMX_GPIO_NR(5, 8)
	IOMUX_PAD_CTRL(DISP0_DAT14__GPIO5_IO08, WEAK_PULLDN),
	/* TODO Wireless control pins */
	IOMUX_PAD_CTRL(NANDF_ALE__GPIO6_IO08, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NANDF_CLE__GPIO6_IO07, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NANDF_D3__GPIO2_IO03, WEAK_PULLUP),
	/* main power on/ use pmic_on_req instead */
	IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, WEAK_PULLUP),

	/* New rev,  not shorted with TAMPER */
#define GPIRQ_TAMPER		IMX_GPIO_NR(6, 17)
	IOMUX_PAD_CTRL(SD3_DAT7__GPIO6_IO17, WEAK_PULLUP),
	/* Old rev, shorted with TAMPER */
#define GPIRQ_TAMPER_OLD	IMX_GPIO_NR(4, 18)
	IOMUX_PAD_CTRL(DI0_PIN2__GPIO4_IO18, WEAK_PULLDN),

	/* TP68 */
	IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP),
	/* TP71 */
	IOMUX_PAD_CTRL(ENET_TXD0__GPIO1_IO30, WEAK_PULLUP),
	/* TP72 */
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, WEAK_PULLUP),
	/* TP74 */
#define GP_TP74			IMX_GPIO_NR(2, 7)
	IOMUX_PAD_CTRL(NANDF_D7__GPIO2_IO07, WEAK_PULLDN),
	/* TP84 */
	IOMUX_PAD_CTRL(KEY_ROW2__GPIO4_IO11, WEAK_PULLUP),
	/* TP85 */
	IOMUX_PAD_CTRL(GPIO_9__GPIO1_IO09, WEAK_PULLUP),
	/* TP86 */
	IOMUX_PAD_CTRL(DISP0_DAT10__GPIO4_IO31, WEAK_PULLUP),
	/* TP87 */
	IOMUX_PAD_CTRL(DISP0_DAT19__GPIO5_IO13, WEAK_PULLUP),
	/* TP91 */
	IOMUX_PAD_CTRL(SD1_DAT0__GPIO1_IO16, WEAK_PULLUP),
	/* TP92 */
	IOMUX_PAD_CTRL(SD1_DAT1__GPIO1_IO17, WEAK_PULLUP),
	/* TP93 */
	IOMUX_PAD_CTRL(EIM_A21__GPIO2_IO17, WEAK_PULLUP),
	/* TP94 */
	IOMUX_PAD_CTRL(EIM_A22__GPIO2_IO16, WEAK_PULLUP),
	/* TP95 */
	IOMUX_PAD_CTRL(EIM_DA0__GPIO3_IO00, WEAK_PULLUP),
	/* TP96 */
	IOMUX_PAD_CTRL(DISP0_DAT18__GPIO5_IO12, WEAK_PULLUP),
	/* TP97 */
	IOMUX_PAD_CTRL(EIM_DA5__GPIO3_IO05, WEAK_PULLUP),
	/* TP105 */
	IOMUX_PAD_CTRL(EIM_A24__GPIO5_IO04, WEAK_PULLUP),
	/* TP106 */
	IOMUX_PAD_CTRL(EIM_DA7__GPIO3_IO07, WEAK_PULLUP),
	/* TP110 */
	IOMUX_PAD_CTRL(KEY_COL4__GPIO4_IO14, WEAK_PULLUP),
	/* Let max77818 control this directly by turning on/off CHGIN */
	/* reg_usbotg_vbus */
#define GP_REG_USBOTG		IMX_GPIO_NR(3, 22)
	/*
	 * TP111 on rev 1,
	 * power enable on rev 0 that should not be used, so keep pull down
	 */
	IOMUX_PAD_CTRL(EIM_D22__GPIO3_IO22, WEAK_PULLDN),
	/* TP131 */
	IOMUX_PAD_CTRL(EIM_A25__GPIO5_IO02, WEAK_PULLUP),

	/* I2C1 - mux */
#define GP_I2C1MUX_A0		IMX_GPIO_NR(3, 15)
	IOMUX_PAD_CTRL(EIM_DA15__GPIO3_IO15, WEAK_PULLDN),
#define GP_I2C1MUX_A1		IMX_GPIO_NR(3, 14)
	IOMUX_PAD_CTRL(EIM_DA14__GPIO3_IO14, WEAK_PULLDN),
#define GP_I2C1MUX_RESET	IMX_GPIO_NR(3, 13)
	IOMUX_PAD_CTRL(EIM_DA13__GPIO3_IO13, WEAK_PULLDN),

	/* I2C1(mux 0) - wm8960 */
	IOMUX_PAD_CTRL(GPIO_0__CCM_CLKO1, WEAK_PULLDN),
#define GP_WM8960_MIC_DET		IMX_GPIO_NR(7, 8)
	IOMUX_PAD_CTRL(SD3_RST__GPIO7_IO08, WEAK_PULLUP),
#define GP_WM8960_HP_DET		IMX_GPIO_NR(4, 10)
	IOMUX_PAD_CTRL(KEY_COL2__GPIO4_IO10, WEAK_PULLUP),

	/* I2C1(mux 1) - accelerometer */
#define GPIRQ_MPU9250_INT	IMX_GPIO_NR(6, 11)
	IOMUX_PAD_CTRL(NANDF_CS0__GPIO6_IO11, WEAK_PULLUP),

	/* I2C1(mux 2) - finger sensor */
	IOMUX_PAD_CTRL(CSI0_DAT12__IPU1_CSI0_DATA12, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT13__IPU1_CSI0_DATA13, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT14__IPU1_CSI0_DATA14, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT15__IPU1_CSI0_DATA15, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT16__IPU1_CSI0_DATA16, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT17__IPU1_CSI0_DATA17, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT18__IPU1_CSI0_DATA18, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DAT19__IPU1_CSI0_DATA19, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_DATA_EN__IPU1_CSI0_DATA_EN, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_PIXCLK__IPU1_CSI0_PIXCLK, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_MCLK__IPU1_CSI0_HSYNC, WEAK_PULLUP),
	IOMUX_PAD_CTRL(CSI0_VSYNC__IPU1_CSI0_VSYNC, WEAK_PULLUP),
#define GP_FP_CSI0_RESET_N	IMX_GPIO_NR(2, 27)
	IOMUX_PAD_CTRL(EIM_LBA__GPIO2_IO27, WEAK_PULLUP),
#define GP_FP_SENSOR_STAT	IMX_GPIO_NR(2, 26)
	IOMUX_PAD_CTRL(EIM_RW__GPIO2_IO26, WEAK_PULLUP),
#define GP_FP_LE_EN		IMX_GPIO_NR(2, 25)
	IOMUX_PAD_CTRL(EIM_OE__GPIO2_IO25, WEAK_PULLUP),
#define GP_FP_LE_5V_EN		IMX_GPIO_NR(2, 24)
	IOMUX_PAD_CTRL(EIM_CS1__GPIO2_IO24, WEAK_PULLUP),
#define GP_FP_CSI_3P3V_EN	IMX_GPIO_NR(2, 23)
	IOMUX_PAD_CTRL(EIM_CS0__GPIO2_IO23, WEAK_PULLUP),

	/* I2C1(mux 3) - ov5640 mipi */
#define GP_OV5640_MIPI_POWER_DOWN	IMX_GPIO_NR(6, 10)
	IOMUX_PAD_CTRL(NANDF_RB0__GPIO6_IO10, WEAK_PULLUP),
#define GP_OV5640_MIPI_RESET		IMX_GPIO_NR(2, 5)
	IOMUX_PAD_CTRL(NANDF_D5__GPIO2_IO05, WEAK_PULLDN),
	IOMUX_PAD_CTRL(GPIO_3__CCM_CLKO2, OUTPUT_40OHM),	/* mclk */

	/* I2C2 - EEPROM */
#define GPIRQ_EEPROM_INTR	IMX_GPIO_NR(2, 28)
	IOMUX_PAD_CTRL(EIM_EB0__GPIO2_IO28, WEAK_PULLUP),
#define GP_EEPROM_ADDRESS	IMX_GPIO_NR(2, 29)
	IOMUX_PAD_CTRL(EIM_EB1__GPIO2_IO29, WEAK_PULLDN),

	/* I2C2 - RTC */
#define GPIRQ_RTC	IMX_GPIO_NR(1, 4)
	IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, WEAK_PULLUP),

	/* I2C3 - Touch */
#define GPIRQ_TOUCH	IMX_GPIO_NR(4, 15)
	IOMUX_PAD_CTRL(KEY_ROW4__GPIO4_IO15, WEAK_PULLUP),
#define GP_TOUCH_RESET	IMX_GPIO_NR(1, 7)
	IOMUX_PAD_CTRL(GPIO_7__GPIO1_IO07, WEAK_PULLUP),

	/* I2C3 - LM3643 */
#define GP_FLASH_STROBE		IMX_GPIO_NR(6, 6)
	IOMUX_PAD_CTRL(EIM_A23__GPIO6_IO06, WEAK_PULLUP),
#define GP_FLASH_HW_EN		IMX_GPIO_NR(5, 6)
	IOMUX_PAD_CTRL(DISP0_DAT12__GPIO5_IO06, WEAK_PULLUP),
#define GP_TORCH_EN		IMX_GPIO_NR(5, 5)
	IOMUX_PAD_CTRL(DISP0_DAT11__GPIO5_IO05, WEAK_PULLUP),
#define GP_FLASH_TX		IMX_GPIO_NR(4, 30)
	IOMUX_PAD_CTRL(DISP0_DAT9__GPIO4_IO30, WEAK_PULLUP),

	/* I2C3 - max77818 */
#define GPIRQ_MAX77818_WCHG_VALID_INT	IMX_GPIO_NR(2, 22)
	IOMUX_PAD_CTRL(EIM_A16__GPIO2_IO22, WEAK_PULLUP),
#define GPIRQ_MAX77818_WCHG_VALID	IMX_GPIO_NR(2, 21)
	IOMUX_PAD_CTRL(EIM_A17__GPIO2_IO21, WEAK_PULLUP),
#define GPIRQ_MAX77818_CHG_INT		IMX_GPIO_NR(2, 20)
	IOMUX_PAD_CTRL(EIM_A18__GPIO2_IO20, WEAK_PULLUP),

	/* MIPI display */
#define GP_MIPI_DSI_RESET	IMX_GPIO_NR(2, 2)
	IOMUX_PAD_CTRL(NANDF_D2__GPIO2_IO02, WEAK_PULLDN),
#define GP_MIPI_TE		IMX_GPIO_NR(6, 9)
	IOMUX_PAD_CTRL(NANDF_WP_B__GPIO6_IO09, WEAK_PULLDN),
#define GP_MIPI_ID		IMX_GPIO_NR(2, 4)
	IOMUX_PAD_CTRL(NANDF_D4__GPIO2_IO04, WEAK_PULLDN),

	/* PWM1, backlight (MIPI display) */
#define GP_PWM1			IMX_GPIO_NR(1, 21)
	IOMUX_PAD_CTRL(SD1_DAT3__GPIO1_IO21, WEAK_PULLUP),
#define GP_MIPI_BACKLIGHT_EN	IMX_GPIO_NR(1, 2)
	IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, WEAK_PULLDN),

	/* PWM2, FP_MCLK */
#define GP_PWM2			IMX_GPIO_NR(1, 19)
	IOMUX_PAD_CTRL(SD1_DAT2__GPIO1_IO19, WEAK_PULLUP),

	/* reg_wlan_en */
#define GP_REG_WLAN_EN		IMX_GPIO_NR(6, 15)
	IOMUX_PAD_CTRL(NANDF_CS2__GPIO6_IO15, WEAK_PULLDN),

	/* Rev 1 only, not on rev 0 */
#define GP_USB_NFC_PWR_EN	IMX_GPIO_NR(4, 21)
	IOMUX_PAD_CTRL(DISP0_DAT0__GPIO4_IO21, WEAK_PULLDN),

	/* UART2 */
#ifndef CONFIG_SILENT_UART
	IOMUX_PAD_CTRL(EIM_D26__UART2_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__UART2_RX_DATA, UART_PAD_CTRL),
#else
	IOMUX_PAD_CTRL(EIM_D26__GPIO3_IO26, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__GPIO3_IO27, UART_PAD_CTRL),
#endif

	/* UART3 for BT */
	IOMUX_PAD_CTRL(EIM_D24__UART3_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D25__UART3_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D23__UART3_CTS_B, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D31__UART3_RTS_B, UART_PAD_CTRL),

	/* UART4 */
	IOMUX_PAD_CTRL(KEY_COL0__UART4_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW0__UART4_RX_DATA, UART_PAD_CTRL),

	/* UART5 */
	IOMUX_PAD_CTRL(KEY_COL1__UART5_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW1__UART5_RX_DATA, UART_PAD_CTRL),

#define GP_USBH1_HUB_RESET	IMX_GPIO_NR(7, 12)
	IOMUX_PAD_CTRL(GPIO_17__GPIO7_IO12, WEAK_PULLDN),

	/* USBOTG */
	IOMUX_PAD_CTRL(GPIO_1__USB_OTG_ID, WEAK_PULLUP),

	/* USDHC2 - Wifi */
	IOMUX_PAD_CTRL(SD2_CLK__SD2_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CMD__SD2_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT0__SD2_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT1__SD2_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT2__SD2_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT3__SD2_DATA3, USDHC_PAD_CTRL),
#define GPIRQ_WIFI		IMX_GPIO_NR(6, 14)
	IOMUX_PAD_CTRL(NANDF_CS1__GPIO6_IO14, WEAK_PULLDN),
#define GP_WIFI_WAKE		IMX_GPIO_NR(2, 1)
	IOMUX_PAD_CTRL(NANDF_D1__GPIO2_IO01, WEAK_PULLUP),

//	IOMUX_PAD_CTRL(SD1_CLK__OSC32K_32K_OUT, OUTPUT_40OHM),	/* slow clock */

	/* USDHC3 - SD card */
	IOMUX_PAD_CTRL(SD3_CLK__SD3_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CMD__SD3_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT0__SD3_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT1__SD3_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT2__SD3_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT3__SD3_DATA3, USDHC_PAD_CTRL),
#define GP_USDHC3_CD		IMX_GPIO_NR(7, 0)
	IOMUX_PAD_CTRL(SD3_DAT5__GPIO7_IO00, WEAK_PULLUP),

	/* USDHC4 - emmc */
	IOMUX_PAD_CTRL(SD4_CLK__SD4_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_CMD__SD4_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT0__SD4_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT1__SD4_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT2__SD4_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT3__SD4_DATA3, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT4__SD4_DATA4, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT5__SD4_DATA5, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT6__SD4_DATA6, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT7__SD4_DATA7, USDHC_PAD_CTRL),
#define GP_EMMC_RESET		IMX_GPIO_NR(2, 6)
	IOMUX_PAD_CTRL(NANDF_D6__GPIO2_IO06, WEAK_PULLUP),
};

static const struct i2c_pads_info i2c_pads[] = {
	I2C_PADS_INFO_ENTRY(I2C1, EIM_D21, 3, 21, EIM_D28, 3, 28, I2C_PAD_CTRL),
	I2C_PADS_INFO_ENTRY(I2C2, KEY_COL3, 4, 12, KEY_ROW3, 4, 13, I2C_PAD_CTRL),
	I2C_PADS_INFO_ENTRY(I2C3, GPIO_5, 1, 05, GPIO_16, 7, 11, I2C_PAD_CTRL),
};
#define I2C_BUS_CNT	3

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	if (port) {
		gpio_set_value(GP_USB_NFC_PWR_EN, 1);

		/* Reset USB hub */
		gpio_set_value(GP_USBH1_HUB_RESET, 0);
		mdelay(2);
		gpio_set_value(GP_USBH1_HUB_RESET, 1);
	}
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	max77823_otg_power(on);
	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg board_usdhc_cfg[] = {
	{.esdhc_base = USDHC3_BASE_ADDR, .bus_width = 4,
			.gp_cd = GP_USDHC3_CD},
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

static const unsigned short gpios_out_low[] = {
	GP_BT_RFKILL_RESET,
	GP_EEPROM_ADDRESS,
	GP_FLASH_HW_EN,
	GP_FP_CSI0_RESET_N,
	GP_FP_LE_EN,
	GP_FP_LE_5V_EN,
	GP_FP_CSI_3P3V_EN,
	GP_GPS_RESET,
	GP_I2C1MUX_A0,
	GP_I2C1MUX_A1,
	GP_I2C1MUX_RESET,
	GP_MIPI_DSI_RESET,
	GP_MODEM_RESET,
	GP_MODEM_ON_OFF,
#if LED_ACTIVE_BLUE != 0
	GP_LED_BLUE,
#endif
#if LED_ACTIVE_GREEN != 0
	GP_LED_GREEN,
#endif
#if LED_ACTIVE_RED != 0
	GP_LED_RED,
#endif
	GP_OV5640_MIPI_RESET,
	GP_PWM1,
	GP_MIPI_BACKLIGHT_EN,
	GP_PWM2,
	GP_REG_USBOTG,
	GP_REG_WLAN_EN,
	GP_USBH1_HUB_RESET,
	GP_USB_NFC_PWR_EN,
	GP_REG_2P8V_EN,
	GP_VIBRATOR_EN,
	GP_TORCH_EN,
	GP_TOUCH_RESET,
};
static const unsigned short gpios_out_high[] = {
	GP_ECSPI1_NOR_CS,
	GP_ECSPI2_CS,
#if LED_ACTIVE_BLUE == 0
	GP_LED_BLUE,
#endif
#if LED_ACTIVE_GREEN == 0
	GP_LED_GREEN,
#endif
#if LED_ACTIVE_RED == 0
	GP_LED_RED,
#endif
	GP_LED_CTRL_GREEN,
	GP_LED_CTRL_RED,
	GP_OV5640_MIPI_POWER_DOWN,
};

static const unsigned short gpios_in[] = {
	GP_GPIOKEY_POWER,
	GP_GPIOKEY_RESET,
	GP_GPIOKEY_VBUS_STATUS,
	GP_GPS_INT,
	GP_SIM_DETECT,
	GP_USDHC3_CD,
	GP_WM8960_HP_DET,
	GP_WM8960_MIC_DET,
	GP_MIPI_TE,
	GPIRQ_EEPROM_INTR,
	GPIRQ_MAX77818_WCHG_VALID_INT,
	GPIRQ_MAX77818_WCHG_VALID,
	GPIRQ_MAX77818_CHG_INT,
	GPIRQ_MPU9250_INT,
	GPIRQ_RTC,
	GPIRQ_TAMPER,
	GPIRQ_TAMPER_OLD,
	GPIRQ_TOUCH,
	GPIRQ_WIFI,
	GP_TP74,
};

int board_early_init_f(void)
{
	set_gpios_in(gpios_in, ARRAY_SIZE(gpios_in));
	set_gpios(gpios_out_high, ARRAY_SIZE(gpios_out_high), 1);
	set_gpios(gpios_out_low, ARRAY_SIZE(gpios_out_low), 0);
	SETUP_IOMUX_PADS(init_pads);
	return 0;
}

void flash_red_led(void)
{
	int led_red = LED_ACTIVE_RED;
	int i;

	gpio_set_value(GP_LED_BLUE, LED_ACTIVE_BLUE ^ 1);
	mdelay(500);

	for (i = 0; i < 4; i++) {
		gpio_set_value(GP_LED_RED, led_red);
		mdelay(500);
		led_red ^= 1;
	}
}

static void __board_poweroff(void)
{
	struct snvs_regs *snvs = (struct snvs_regs *)(SNVS_BASE_ADDR);

	writel(0x60, &snvs->lpcr);
	mdelay(500);
}

void board_poweroff(void)
{
	flash_red_led();
	__board_poweroff();
}

void board_power_check(void)
{
	gpio_set_value(GP_VIBRATOR_EN, 0);
	max77834_power_check();
}

ulong start_time;

void hw_watchdog_reset(void)
{
	ulong elapsed;
	if (start_time) {
		elapsed = get_timer(start_time);
		if (elapsed >= CONFIG_SYS_HZ/2) {
			gpio_set_value(GP_VIBRATOR_EN, 0);
			start_time = 0;
		}
	}
}

int board_init(void)
{
	int i;
	int inactive = 0;
	int active = 0;
	common_board_init(i2c_pads, I2C_BUS_CNT, IOMUXC_GPR1_OTG_ID_GPIO1,
			NULL, 0, 0);
	gpio_set_value(GP_LED_BLUE, LED_ACTIVE_BLUE);

	if ((get_imx_reset_cause() & 0xef) == 0x1) {
		/*
		 * Power-on reset
		 * Check that power button is on for 1/2 second more
		 */
		for (i = 0; i < 500; i++) {
			if (gpio_get_value(GP_GPIOKEY_POWER)) {
				inactive++;
				if (inactive >= 100) {
					printf("power button not pressed long enough\n");
					__board_poweroff();
				}
			} else {
				active++;
				if (active >= 400)
					break;
			}
			udelay(1000);
		}
		gpio_set_value(GP_VIBRATOR_EN, 1);
		start_time = get_timer(0);
	}
	return 0;
}

const struct button_key board_buttons[] = {
	{"power",	GP_GPIOKEY_POWER, 'p', 1},
	{"reset",	GP_GPIOKEY_RESET, 'r', 1},
#ifdef CONFIG_REV_OLD
	{"tamper",	TAMPER_CHECK,	't', 1, 1},
#else
	{"tamper",	GPIRQ_TAMPER,	't', 1, 1},
#endif
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

static int _do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	board_poweroff();
	return 0;
}

U_BOOT_CMD(
	poweroff, 70, 0, _do_poweroff,
	"power down board",
	""
);
