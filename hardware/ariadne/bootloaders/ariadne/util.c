/* Name: util.c
 * Author: Stelios Tsampas
 * Copyright: Arduino and optiboot
 * License: GPL http://www.gnu.org/licenses/gpl-2.0.html
 * Project: tftpboot
 * Function: Utility routines for various stuff
 * Version: 0.2 support for USB flashing
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "util.h"
#include "tftp.h"
#include "spi.h"
#include "tftp.h"
#include "debug.h"
#include "debug_util.h"

static uint16_t last_timer_1;
static uint16_t tick = 0;
static uint8_t resetRequried = FALSE;

void updateLed(void)
{
	uint16_t next_timer_1 = TCNT1;

	if(next_timer_1 & 0x1000) LED_PORT ^= _BV(LED); // Led pin high
	else LED_PORT &= ~_BV(LED); // Led pin low

	if(next_timer_1 < last_timer_1) {
		tick++;
		DBG_UTIL(
		    tracePGMlnUtil(mDebugUtil_TICK);
		    tracenum(tick);
		    tracePGMlnUtil(mDebugUtil_NEXT);
		    tracenum(next_timer_1);
		    tracePGMlnUtil(mDebugUtil_LAST);
		    tracenum(last_timer_1);
		)
	}

	last_timer_1 = next_timer_1;
}

void resetTick(void)
{
	TCNT1 = 0;
	tick = 0;
	wdt_reset(); //Added so can use wdt
}

uint8_t timedOut(void)
{
	// Never timeout if there is no code in Flash
	if (pgm_read_word(0x0000) == 0xFFFF) return(0);
	
	if(tick > TIMEOUT) return(1);
	else return(0);
}

// Helper method for initiated socket reset.
// main loop will reset the socket when needed.
void resetSocket(void)
{
	resetRequried = TRUE;
}

//
// Determine the conditions for reseting server OR reseting socket:
//
uint8_t socketResetRequried(void)
{
	// 1. Explicity requested, by resetSocket method.
	if (resetRequried) {
		resetRequried = FALSE;
		return(1);
	}

	//	2. tftp is active too long without calling "resetTick" (otherwise we whoudn't be here).
	//	3. TODO: add ethernet initialize error, why not try again.. after some time passed.
	if (tick > TIMEOUT) {
		if (tftpFlashing == TRUE) return(1);
	}

	return(0);
}
