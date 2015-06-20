#include <hidef.h> /* for EnableInterrupts macro */
#include "MC9S08QG8.h"
#include "derivative.h" /* include peripheral declarations */
#include "MCUinit.h"

void main(void) {
	ICSTRM = *(unsigned char*far)0xFFBE;    /* Copy Factory Trim value */
	MCU_init();
	SCI_init();
	EnableInterrupts;
	
	for(;;) {
		__RESET_WATCHDOG();
	}
}
