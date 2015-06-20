#include <MC9S08QG8.h> /* I/O map */
#define BIT(pos)	(1<<(pos))
#define ON			1
#define OFF			0
#define V1_MIN		(unsigned char)0x1F
#define V1_MAX		(unsigned char)0xB0
#define V2_MIN		(unsigned char)0x67
#define V2_MAX		(unsigned char)0xB0
#define V3_MIN		(unsigned char)0x6B
#define V3_MAX		(unsigned char)0xB0
#define COUNT		10

unsigned char ADCValue;
unsigned char val1;
unsigned char val2;
unsigned char val3;
int count;

void MCU_init(void)
{
	/* Conf. Generales */
	SOPT1 = 0x52;
	SOPT2 = 0x00;
	SPMSC1 = 0x1C;
	SPMSC2 = 0x00;
	SPMSC3 &= (unsigned char)~0x30;
	/* Conf. Internal Clock Source */
	ICSC1 = 0x04;
	ICSC2 = 0x40;
	/* Conf. Timer para desbordarse cada 2ms */
	MTIMCLK = 0x06;
	MTIMMOD = 0x7D;
	MTIMSC_TOIE = 1;
	MTIMSC_TRST = 1;
	MTIMSC_TSTP = 0;
	/* Inicializa puertos */
	PTASE |= (unsigned char)0x3F;
	PTBSE = 0xFF;
	PTADS = 0x00;
	PTBDD |= BIT(2)+BIT(3)+BIT(4)+BIT(5)+BIT(6)+BIT(7);
	//PTBDS |= BIT(2)+BIT(3)+BIT(4)+BIT(5)+BIT(6)+BIT(7);
	PTBD &= ~(BIT(2)+BIT(3)+BIT(4)+BIT(5)+BIT(6)+BIT(7));
	/* Conf. ADC */
	ADCSC2 = 0x00;
	ADCCV = 0x00;
	ADCCFG = 0x50;
	ADCSC1 = 0x60;
//	APCTL1 |= BIT(0)+BIT(1)+BIT(2);
	asm CLI; 
	/* Inicializacion variables */
	val1 = 0x00;
	val2 = 0x00;
	val3 = 0x00;
	count = COUNT;
}

void SCI_init(void)
{
	/* ### Configuración del módulo SCI*/
	/* SCIBD: SBR12=0,SBR11=0,SBR10=0,SBR9=0,SBR8=0,SBR7=0,SBR6=0,SBR5=0,SBR4=1,SBR3=1,SBR2=0,SBR1=1,SBR0=0 */
	/*Baud rate = 9600 bps*/
	SCIBDH = 0x00;
	SCIBDL = 0x18; /* Este baudrate se ajusta más a la frecuencia de nuestro micro*/
	/* SCIC1: LOOPS=0,SCISWAI=0,RSRC=0,M=0,WAKE=0,ILT=0,PE=0,PT=0 */
	SCIC1 = 0x00;
	/* SCIC2: TIE=0,TCIE=0,RIE=0,ILIE=0,TE=1,RE=1,RWU=0,SBK=0 */
	SCIC2 = 0x0C;
	/* SCIC3: R8=0,T8=0,TXDIR=1,TXINV=0,ORIE=0,NEIE=0,FEIE=0,PEIE=0 */
	SCIC3 = 0x20;
	/* SCIS2: BRK13=0,RAF=0 */
	SCIS2 = 0x00;
	/* ### */
}

__interrupt void Vadc_Isr(void)
{	
	switch (ADCSC1_ADCH){
		case 0x00:
			val1 = ADCRL;
			break;
		case 0x01:
			val2 = ADCRL;
			break;
		case 0x02:
			val3 = ADCRL;
			break;
		default:
			break;
	}
	if (ADCSC1_ADCH < 0x03) ADCSC1_ADCH += (unsigned char)0x01;
	else ADCSC1_ADCH = 0x00;
}

__interrupt void Vmtim_Isr(void)
{
	if (count == 0){
		count = COUNT;
		if (val1 < V1_MIN) {
			PTBD_PTBD2 = ON;
			PTBD_PTBD3 = OFF;
		} else if ((val1 > V1_MIN) && (val1 < V1_MAX)) {
			PTBD_PTBD2 = OFF;
			PTBD_PTBD3 = OFF;		
		} else if (val1 > V1_MAX) {
			PTBD_PTBD2 = OFF;
			PTBD_PTBD3 = ON;	
		}

		if (val2 < V2_MIN) {
			PTBD_PTBD4 = ON;
			PTBD_PTBD5 = OFF;
		} else if ((val2 > V2_MIN) && (val2 < V2_MAX)) {
			PTBD_PTBD4 = OFF;
			PTBD_PTBD5 = OFF;		
		} else if (val2 > V2_MAX) {
			PTBD_PTBD4 = OFF;
			PTBD_PTBD5 = ON;	
		}

		if (val3 < V3_MIN) {
			PTBD_PTBD6 = ON;
			PTBD_PTBD7 = OFF;
		} else if ((val3 > V3_MIN) && (val3 < V3_MAX)) {
			PTBD_PTBD6 = OFF;
			PTBD_PTBD7 = OFF;		
		} else if (val3 > V3_MAX) {
			PTBD_PTBD6 = OFF;
			PTBD_PTBD7 = ON;	
		}

		//Envio datos sensor 1
		while (SCIS1_TDRE == 0);
		SCID = val1;

		//Envio datos sensor 2
		while (SCIS1_TDRE == 0);
		SCID = val2;

		//Envio datos sensor 3
		while (SCIS1_TDRE == 0);
		SCID = val3;
	} else {
		count--;
	}
	
	MTIMMOD = 0x7D;
	
//	//Envio datos sensor 1
//	while (SCIS1_TDRE == 0);
//	SCID = (val1 << 2) + (unsigned char)0x01;
//	while (SCIS1_TDRE == 0);
//	SCID = (val1 >> 6) + (unsigned char)0x01;
//	while (SCIS1_TDRE == 0);
//	SCID = 0x0A;
//	
//	//Envio datos sensor 2
//	while (SCIS1_TDRE == 0);
//	SCID = (val2 << 2) + (unsigned char)0x02;
//	while (SCIS1_TDRE == 0);
//	SCID = (val2 >> 6) + (unsigned char)0x02;
//	while (SCIS1_TDRE == 0);
//	SCID = 0x0A;
//
//	//Envio datos sensor 3
//	while (SCIS1_TDRE == 0);
//	SCID = (val3 << 2) + (unsigned char)0x03;
//	while (SCIS1_TDRE == 0);
//	SCID = (val3 >> 6) + (unsigned char)0x03;
//	while (SCIS1_TDRE == 0);
//	SCID = 0x0A;
//	MTIMMOD = 0x7D;
}

extern void _Startup(void);

/* Interrupt vector table */
#define UNASSIGNED_ISR 0xFFFF          /* unassigned interrupt service routine */

void (* const _vect[])() @0xFFD0 = {   /* Interrupt vector table */
         UNASSIGNED_ISR,               /* Int.no.  0 Vrti (at FFD0)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  1 Reserved2 (at FFD2)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  2 Reserved3 (at FFD4)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  3 Vacmp (at FFD6)                 Unassigned */
         Vadc_Isr,                     /* Int.no.  4 Vadc (at FFD8)                  Used */
         UNASSIGNED_ISR,               /* Int.no.  5 Vkeyboard (at FFDA)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  6 Viic (at FFDC)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  7 Vscitx (at FFDE)                Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  8 Vscirx (at FFE0)                Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  9 Vscierr (at FFE2)               Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 10 Vspi (at FFE4)                  Unassigned */
         Vmtim_Isr,           	       /* Int.no. 11 Vmtim (at FFE6)                 Used */
         UNASSIGNED_ISR,               /* Int.no. 12 Reserved13 (at FFE8)            Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 13 Reserved14 (at FFEA)            Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 14 Reserved15 (at FFEC)            Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 15 Reserved16 (at FFEE)            Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 16 Vtpmovf (at FFF0)               Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 17 Vtpmch1 (at FFF2)               Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 18 Vtpmch0 (at FFF4)               Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 19 Reserved20 (at FFF6)            Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 20 Vlvd (at FFF8)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 21 Virq (at FFFA)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 22 Vswi (at FFFC)                  Unassigned */
 };
