#ifndef MCUINIT_H_
#define MCUINIT_H_

/* Incluye modulos compartidos los cuales se utilizan en todo el proyecto */
extern void MCU_init(void);
extern void SCI_init(void);
/*
** ===================================================================
** Method : MCU_init (bean MC9S08QG8_16)
**
** Description :
** Device initialization code for selected peripherals.
** ===================================================================
*/
__interrupt void Vadc_Isr(void);
__interrupt void Vmtim_Isr(void);
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

