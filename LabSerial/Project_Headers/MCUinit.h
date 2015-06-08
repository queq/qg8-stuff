#ifndef MCUINIT_H_
#define MCUINIT_H_

/* Incluye modulos compartidos los cuales se utilizan en todo el proyecto */
extern void MCU_init(void);
extern void SCI_init(void);
extern int getMotorStatus(void);
extern int getMotorDirection(void);
extern unsigned char getMotorSpeed(void);
/*
** ===================================================================
** Method : MCU_init (bean MC9S08QG8_16)
**
** Description :
** Device initialization code for selected peripherals.
** ===================================================================
*/
__interrupt void VscirxISR(void);
/*
** ===================================================================
** Interrupt handler : VscirxISR
**
** Description :
** User interrupt service routine.
** Parameters : None
** Returns : Nothing
** ===================================================================
*/

#endif /* MCUINIT_H_ */

/*
** ###################################################################**
Cabecera.
** Project : SCI_QG9
** Processor : MC9S08QG8CPB
** Abstract :
** This module contains device initialization code
** for selected on-chip peripherals.
** Contents :
** Function "MCU_init" initializes selected peripherals
###################################################################
*/

