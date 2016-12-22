
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

/* Delay for 100ns. */
#define PULSE_DELAY_TICKS 10
/* The goal is a pulse with 1050ns.
 * Because of the slow rising edges, set this to 1000ns.
 */
#define PULSE_WIDTH_TICKS 100


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

	/* Set GPIO0 to input. */
	ulValue  = 0U << HOSTSRT(gpio_cfg0_inv);
	ulValue |= 0U << HOSTSRT(gpio_cfg0_mode);
	ptGpioArea->aulGpio_cfg[0] = ulValue;

	/* Set GPIO1 to input. */
	ptGpioArea->aulGpio_cfg[1] = 0;

	/* Stop counter 0 and 1. */
	ptGpioArea->aulGpio_counter_ctrl[0] = 0;
	ptGpioArea->aulGpio_counter_ctrl[1] = 0;

	/* Set the value of counter 0 to 0 and the maximum to 0xffffffff. */
	ptGpioArea->aulGpio_counter_cnt[0] = 0;
	ptGpioArea->aulGpio_counter_max[0] = 0xffffffffU;

	/* Set the value of counter 1 to 0 and the maximum to the sum of the delay and the pulse. */
	ptGpioArea->aulGpio_counter_cnt[1] = 0;
	ptGpioArea->aulGpio_counter_max[1] = PULSE_DELAY_TICKS + PULSE_WIDTH_TICKS - 1U;

	/* Set the treshold of GPIO 1 to the delay. */
	ptGpioArea->aulGpio_tc[1] = PULSE_DELAY_TICKS;

	/* Setup counter 0 to count GPIO0 negative edges. */
	ulValue  = 0U << HOSTSRT(gpio_counter0_ctrl_gpio_ref);
	ulValue |= 1U << HOSTSRT(gpio_counter0_ctrl_event_act);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_once);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_sel_event);
	ulValue |= HOSTMSK(gpio_counter0_ctrl_run);
	ptGpioArea->aulGpio_counter_ctrl[0] = ulValue;

	/* Setup counter 1 to start with the GPIO0 event. */
	ulValue  = 0U << HOSTSRT(gpio_counter1_ctrl_gpio_ref);
	ulValue |= 3U << HOSTSRT(gpio_counter1_ctrl_event_act);
	ulValue |= HOSTMSK(gpio_counter1_ctrl_once);
	ulValue |= HOSTMSK(gpio_counter1_ctrl_sel_event);
	ulValue |= HOSTMSK(gpio_counter1_ctrl_run);
	ptGpioArea->aulGpio_counter_ctrl[1] = ulValue;

	/* Set GPIO1 to PWM mode with counter 1.
	 * Set to inverted mode to have a proper reset signal. */
	ulValue  = 1U << HOSTSRT(gpio_cfg1_count_ref);
	ulValue |= 1U << HOSTSRT(gpio_cfg0_inv);
	ulValue |= 7U << HOSTSRT(gpio_cfg1_mode);
	ptGpioArea->aulGpio_cfg[1] = ulValue;

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
