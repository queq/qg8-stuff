#include <hidef.h> /* for EnableInterrupts macro */
#include "MC9S08QG8.h"
#include "derivative.h" /* include peripheral declarations */
#include "MCUinit.h"

#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define ON 1
#define OFF 0
#define CW 1
#define CCW 0
#define COUNT 2

int stepState;
int motorDirection;
int motorStatus;
int *pStatus = &motorStatus;
unsigned char tSpeed;
unsigned char motorSpeed;

unsigned char count;

void stepMotor(void);

void main(void) {
	ICSTRM = *(unsigned char*far)0xFFBE;    /* Copy Factory Trim value */
	MCU_init();
	SCI_init();
	motorStatus = ON;
	motorSpeed = 0x11;
	motorDirection = CCW;
	stepState = S0;
	count = COUNT;

  for(;;) {
	  __RESET_WATCHDOG();
	  motorStatus = getMotorStatus();
	  motorDirection = getMotorDirection();
	  motorSpeed = getMotorSpeed();
	  stepMotor();
  }
}

void stepMotor(void)
{
	if (!(motorSpeed == tSpeed)){
		MTIMMOD = motorSpeed;
		tSpeed = motorSpeed;
	}
	switch (stepState){
		case S0:
			/*Output: 1100*/
			if (motorStatus) PTAD = 0x08;
			else PTAD = 0x00;
			if (MTIMSC_TOF){
				MTIMSC_TRST = 1;
				count--;
			}
			if (count == 0){
				stepState = (motorDirection) ? S3 : S1;
				count = COUNT;
			}
			break;
		case S1:
			/*Output: 0110*/
			if (motorStatus) PTAD = 0x04;
			else PTAD = 0x00;
			if (MTIMSC_TOF){
				MTIMSC_TRST = 1;
				count--;
			}
			if (count == 0){
				stepState = (motorDirection) ? S0 : S2;
				count = COUNT;
			}
			break;
		case S2:
			/*Output: 0011*/
			if (motorStatus) PTAD = 0x02;
			else PTAD = 0x00;
			if (MTIMSC_TOF){
				MTIMSC_TRST = 1;
				count--;
			}
			if (count == 0){
				stepState = (motorDirection) ? S1 : S3;
				count = COUNT;
			}
			break;
		case S3:
			/*Output: 1001*/
			if (motorStatus) PTAD = 0x01;
			else PTAD = 0x00;
			if (MTIMSC_TOF){
				MTIMSC_TRST = 1;
				count--;
			}
			if (count == 0){
				stepState = (motorDirection) ? S2 : S0;
				count = COUNT;
			}
			break;
		default:
			stepState = S0;
			break;
	}
}
