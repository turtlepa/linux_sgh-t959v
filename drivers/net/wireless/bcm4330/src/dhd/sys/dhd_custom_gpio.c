/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2010, Broadcom Corporation
* 
*         Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the "GPL"),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the
* following added to such license:
* 
*      As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions of
* the license of that module.  An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* 
*      Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
* $Id: dhd_custom_gpio.c,v 1.2.42.1 2010/10/19 00:41:09 Exp $
*/

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#include <dngl_stats.h>
#include <dhd.h>

#include <wlioctl.h>
#include <wl_iw.h>

#ifdef P3_SPECIFIC
#include <mach/gpio.h>
#include <mach/hardware.h>
#include <mach/gpio-names.h>
#endif

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#define POWER_OFF	0
#define POWER_ON	1

#ifdef P3_SPECIFIC
extern void nvidia_wlan_poweroff(int off, int flag);
extern void nvidia_wlan_poweron(int on, int flag);
#endif /* P3_SPECIFIC */

#ifdef U1_SPECIFIC
extern void wlan_setup_power(int on, int flag);
#endif

#if defined(OOB_INTR_ONLY)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */

#ifdef CUSTOMER_HW3
#include <mach/gpio.h>
#endif

/* Customer specific Host GPIO defintion  */
#ifdef P3_SPECIFIC
static int dhd_oob_gpio_num = TEGRA_GPIO_PS0;
#elif defined(U1_SPECIFIC)
static int dhd_oob_gpio_num = IRQ_EINT(20);
#else
static int dhd_oob_gpio_num = -1; /* GG 19 */
#endif

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

/* that function will returns :
    1) return :  Host gpio interrupt number per customer platform
    2) irq_flags_ptr : Type of Host interrupt as Level or Edge

    NOTE :
    Customer should check his platform definitions
    and hist Host Interrupt  spec
    to figure out the proper setting for his platform.
    BRCM provides just reference settings as example.

*/
int dhd_customer_oob_irq_map(unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
			__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d), flag=0x%X\n",
	         __FUNCTION__, dhd_oob_gpio_num, (unsigned int)*irq_flags_ptr));

#ifdef P3_SPECIFIC
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
#elif defined(U1_SPECIFIC)
	host_oob_irq = dhd_oob_gpio_num;
#endif

	return (host_oob_irq);
}
#endif /* defined(OOB_INTR_ONLY) */

/* Customer function to control hw specific wlan gpios */
void
dhd_customer_gpio_wlan_ctrl(int onoff)
{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET\n",
				__FUNCTION__));
#ifdef P3_SPECIFIC
			nvidia_wlan_poweroff (POWER_OFF, 2);
#elif defined(U1_SPECIFIC)
			wlan_setup_power(POWER_OFF, 2);
#endif
			WL_ERROR(("=========== WLAN placed in RESET ========\n"));
		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET\n",
				__FUNCTION__));
#ifdef P3_SPECIFIC
			nvidia_wlan_poweron (POWER_ON,2);
#elif defined(U1_SPECIFIC)
			wlan_setup_power(POWER_ON, 2);
#endif
			WL_ERROR(("=========== WLAN going back to live  ========\n"));
		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_ON\n",
				__FUNCTION__));
#ifdef P3_SPECIFIC
			nvidia_wlan_poweroff (POWER_OFF, 1);
#elif defined(U1_SPECIFIC)
			wlan_setup_power(POWER_OFF, 1);
#endif
		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef P3_SPECIFIC
			nvidia_wlan_poweron (POWER_ON, 1);
#elif defined(U1_SPECIFIC)
			wlan_setup_power(POWER_ON, 1);
#endif
			/* Lets customer power to get stable */
			/* OSL_DELAY(200); */
		break;
	}
}
