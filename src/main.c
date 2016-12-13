
#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"
#include "uart_standalone.h"
#include "uprintf.h"
#include "version.h"

/*-------------------------------------------------------------------------*/


UART_STANDALONE_DEFINE_GLOBALS


#define MMIO_INDEX_INPUT 0
#define MMIO_INDEX_OUTPUT 1


void test_main(void);
void test_main(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	HOSTDEF(ptGpioArea);
	unsigned long ulValue;
	unsigned long ulNewValue;


	systime_init();
	uart_standalone_initialize();

	uprintf("\f. *** Pulse generator by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch all LEDs off. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Setup the MMIOs. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[MMIO_INDEX_INPUT] = MMIO_CFG_gpio0;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[MMIO_INDEX_OUTPUT] = MMIO_CFG_gpio1;

	/* Set GPIO0 to inverted input. */
	ulValue  = HOSTMSK(gpio_cfg0_inv);
	ulValue |= 0U << HOSTMSK(gpio_cfg0_mode);
	ptGpioArea->aulGpio_cfg[0] = ulValue;

	/* Stop counter0. */
	ptGpioArea->aulGpio_counter_ctrl[0] = 0;

	/* Set the counter value to 0 and the maximum to 0xffffffff. */
	ptGpioArea->aulGpio_counter_cnt[0] = 0;
	ptGpioArea->aulGpio_counter_max[0] = 0xffffffffU;

	/* Setup counter0 to count GPIO0 negative edges. */
	ulValue  = 0U << HOSTSRT(gpio_counter0_ctrl_gpio_ref);
	ulValue |= 1U << HOSTSRT(gpio_counter0_ctrl_event_act);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_once);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_sel_event);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_run);
	ptGpioArea->aulGpio_counter_ctrl[0] = ulValue;

	ulValue = ptGpioArea->aulGpio_counter_cnt[0];
	while(1)
	{
		ulNewValue = ptGpioArea->aulGpio_counter_cnt[0];
		if( ulValue!=ulNewValue )
		{
			uprintf("Counter: 0x%08x\n", ulNewValue);
			ulValue = ulNewValue;
		}
	}
}
