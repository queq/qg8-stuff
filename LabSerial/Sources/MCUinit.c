#include <MC9S08QG8.h> /* I/O map */

#define ON 	1
#define OFF 0
#define CW 1
#define CCW 0

unsigned char ReceivedByte;
int mStatus;			//mStatus global
int mDirection;			//mDirection global
unsigned char mSpeed;	//mSpeed global
//int Status;				//mStatus global
//int Direction;			//mDirection global
//unsigned char Speed;	//mSpeed global

void MCU_init(void)
{
	/* ### Configura la MCU */
	/* SOPT1: COPE=0,COPT=1,STOPE=0,BKGDPE=1,RSTPE=0 */
	SOPT1 = 0x52;
	/* SPMSC1: LVDF=0,LVDACK=0,LVDIE=0,LVDRE=1,LVDSE=1,LVDE=1,BGBE=0 */
	SPMSC1 = 0x1C; /* System Power Management Status and Control 1 Register */
	/* SPMSC2: PDF=0,PPDF=0,PPDACK=0,PDC=0,PPDC=0 */
	SPMSC2 = 0x00; /* System Power Management Status and Control 2 Register */
	/* SPMSC3: LVDV=0,LVWV=0 System Power Management Status and Control 1 Register */
	SPMSC3 &= (unsigned char)~0x30;
	/* ### */
	/* ### Configura el Modulo ICS en modo FEI*/
	/* ICSC1: CLKS=0,RDIV=0,IREFS=1,IRCLKEN=0,IREFSTEN=0 */
	ICSC1 = 0x04; /* ICS en Modo FEI*/
	/* ICSC2: BDIV=1,RANGE=0,HGO=0,LP=0,EREFS=0,ERCLKEN=0,EREFSTEN=0 */
	ICSC2 = 0x40; /* La frecuencia de salida dividida en 2*/
	ICSSC = 0x01; /* Ajuste en modo Fino */
	/* SOPT2: COPCLKS=0,IICPS=0,ACIC=0 */
	SOPT2 = 0x00; /*Se modifica una sola vez */
	/* ### */
	/* ### Inicializa puertos*/
	/* PTASE: PTASE5=1,PTASE4=1,PTASE3=1,PTASE2=1,PTASE1=1,PTASE0=1 */
	PTADD = 0x0F;
	PTBDD_PTBDD4 = 1;
	PTASE |= (unsigned char)0x3F; /*Port A Slew Rate Enable*/
	/* PTBSE: PTBSE7=1, PTBSE6=1, PTBSE5=1, PTBSE4=1, PTBSE3=1, PTBSE2=1, PTBSE1=1, PTBSE0=1 */
	PTBSE = 0xFF; /*Port B Slew Rate Enable*/
	/* PTADS: PTADS5=0,PTADS4=0,PTADS3=0,PTADS2=0,PTADS1=0,PTADS0=0 */
	PTADS = 0x00; /*Port A Drive Strength Select*/
	/* PTBDS: PTBDS7=0,PTBDS6=0,PTBDS5=0,PTBDS4=0,PTBDS3=0,PTBDS2=0,PTBDS1=0,PTBDS0=0 */
	PTBDS = 0x00; /*Port B Drive Strength Select*/
	/* ### */
	/*Conf. Timer*/
	MTIMCLK = 0x08;		//Reloj del timer / 256
	//MTIMMOD = 0x19;		// Modulo del timer, 25dec = 19hex (Para observar settear en FF)
	MTIMMOD = 0xF1;
							//FF (255)-> 7 Hz, 14 Hz en motor, 8.2 RPM
							//AA (170)-> 10Hz, 27 Hz en motor, 13 RPM
							//A0 (160)-> 11Hz, 14 RPM
							//23 (35)-> 50Hz, 61 RPM
							//19 (25)-> 68Hz, 82 RPM
							//16 (22)-> 76Hz, 94 RPM
							//11 (17)-> 98Hz, 120 RPM (Valor minimo que se le puede asignar para que el sMotor funcione correctamente)
	//FORMULA PARA SACAR LOS RPM
	// RPM = (Hz*4)/(200*60)
	// Hz = 1775 / MTIMMOD
	//
	// RPM = 1775*4 / (MTIMMOD*200*60)
	// RPM Max -> 120, MTIMMOD = 0x11
	// RPM Min -> 8.5 RPM, MTIMMOD = 0xFF

	MTIMSC_TRST = 1;	// Module reset
	MTIMSC_TSTP = 0;
	mStatus = ON;
	mSpeed = 0x13;
	mDirection = CW;
} /*MCU_init*/

void SCI_init(void)
{
	/* ### Configuración del módulo SCI*/
	/* SCIBD: SBR12=0,SBR11=0,SBR10=0,SBR9=0,SBR8=0,SBR7=0,SBR6=0,SBR5=0,SBR4=1,SBR3=1,SBR2=0,SBR1=1,SBR0=0 */
	/*Baud rate = 9600 bps*/
	SCIBDH = 0x00;
	SCIBDL = 0x18; /* Este baudrate se ajusta más a la frecuencia de nuestro micro*/
	/* SCIC1: LOOPS=0,SCISWAI=0,RSRC=0,M=0,WAKE=0,ILT=0,PE=0,PT=0 */
	SCIC1 = 0x00;
	/* SCIC2: TIE=0,TCIE=0,RIE=1,ILIE=0,TE=1,RE=1,RWU=0,SBK=0 */
	SCIC2 = 0x2C;
	/* SCIC3: R8=0,T8=0,TXDIR=1,TXINV=0,ORIE=0,NEIE=0,FEIE=0,PEIE=0 */
	SCIC3 = 0x20;
	/* SCIS2: BRK13=0,RAF=0 */
	SCIS2 = 0x00;
	/* ### */
	asm CLI; /* Habilita Interrupciones */
}

int getMotorStatus(void)
{
	int st;
	st = mStatus;
	return st;
}

int getMotorDirection(void)
{
	int dir;
	dir = mDirection;
	return dir;
}

unsigned char getMotorSpeed(void)
{
	unsigned char sp;
	sp = mSpeed;
	return sp;
}
__interrupt void VscirxISR(void)
{
	unsigned char ID;
	unsigned char Frame;
    unsigned char Temp;
    unsigned char tempo;
    PTBD_PTBD4 = 1;
    Temp = SCIS1;             // Acknowledge Receiver Full Flag
    ReceivedByte = SCID;      // Read the Data Buffer
    tempo = 0;
    ID = ReceivedByte & (unsigned char)0x03; 	//Acceder a los dos bits menos significativos
    Frame = ReceivedByte & (unsigned char)0xFC;	// Acceder a los bits 3-7
    switch (ID){

    case 0x00:
    	//PC solicita cambiar el estado del motor
    	tempo = Frame &(unsigned char)0xE0;        //Máscara bits 5-7, correspondientes a mStatus
    	if (tempo == 0xE0) mStatus = ON;     //motor ON, con trama = |1|1|1|X|X|X|0|0|
    	else if (tempo == 0xA0) mStatus = OFF;     //motor OFF, con trama = |1|0|1|X|X|X|0|0|
    	tempo = Frame &(unsigned char)0x1C;    //Máscara bits 2-4, correspondientes a mDirection
    	if (tempo == 0x1C) mDirection = CW;            //motor CCW, con trama = |X|X|X|1|1|1|0|0|
    	else if (tempo == 0x14) mDirection = CCW;   //motor CCW, con trama = |X|X|X|1|0|1|0|0|

    	break;

    case 0x01:
    	//PC solicita cambiar velocidad del motor
    	mSpeed = Frame / 4;        //Velocidad ocupa bits 2-7, al dividir trama entre cuatro se corre para que comience desde el bit 0 y así se pueda igualar al MTIMMOD.

    	break;

    case 0x02:
    	//PC solicita obtener estado de comunicaciones, enviando 0xAA
        //Si PC recibe el mismo 0xAA, comunicaciones OK
        //Si PC recibe 0x55, comunicaciones erróneas pero el micro entiende lo que le envían
        //Si recibe otra cosa, puede ser error de software en el micro o error de sincronización
    	if (Frame == 0xA8){
    		while (SCIS1_TDRE == 0);
    		SCID = 0xAA;
    		while (SCIS1_TDRE == 0);
    		SCID = 0x0A;
    	} else {
    		while (SCIS1_TDRE == 0);
    		SCID = 0x55;
    		while (SCIS1_TDRE == 0);
    		SCID = 0x0A;
    	}

    	break;

    case 0x03:
        //El case 0x01 recibe ordenes del PC, mientras que el 0x03 envía el status general hacia el PC.
    	//PC solicita obtener estado+velocidad del motor, enviando dos tramas:
        // Trama 1: settear en los bits 2-7 la velocidad en función del MTIMMOD, y los bits 0-1 fijos: b1=0 b0=1.

        //Trama 2: Posibles opciones
        //Motor ON & CW:   |1|1|1|1|1|1|0|1| = 0xFD = 253
        //Motor ON & CCW:  |1|1|1|1|0|1|0|1| = 0xF5 = 245
        //Motor OFF & CW:  |0|0|0|1|1|1|0|1| = 0x1D = 29
        //Motor OFF & CCW: |0|0|0|1|0|1|0|1| = 0x15 = 21

        //--------------------------------------------------------------------

        //Ahora, para cuando PC recibe status

        // Trama 1: bits 2-7 la velocidad en función del MTIMMOD, bits 0-1 fijos: b1=1 b0=1.

        //Trama 2: Posibles opciones
        //Motor ON & CW:   |1|1|1|1|1|1|1|1| = 0xFF = 255
        //Motor ON & CCW:  |1|1|1|1|0|1|1|1| = 0xF7 = 247
        //Motor OFF & CW:  |0|0|0|1|1|1|1|1| = 0x1F = 31
        //Motor OFF & CCW: |0|0|0|1|0|1|1|1| = 0x17 = 23

        tempo = 0;
    	if (mStatus == ON) tempo += 0xE0;
        if (mStatus == OFF) tempo += 0xA0;
    	if (mDirection == CW) tempo += 0x1C;
        if (mDirection == CCW) tempo += 0x14;
        tempo += (unsigned char)0x03;

        //Trama 1
		while (SCIS1_TDRE == 0);
    	SCID = tempo;
    	while (SCIS1_TDRE == 0);
    	SCID = 0x0A;

        //Trama 2
    	tempo = (mSpeed * 4) + 0x03;
    	while (SCIS1_TDRE == 0);
    	SCID = tempo;
    	while (SCIS1_TDRE == 0);
    	SCID = 0x0A;

    	break;
    default:
    	break;
    }
}

extern void _Startup(void);

/* Interrupt vector table */
#define UNASSIGNED_ISR 0xFFFF          /* unassigned interrupt service routine */

void (* const _vect[])() @0xFFD0 = {   /* Interrupt vector table */
         UNASSIGNED_ISR,               /* Int.no.  0 Vrti (at FFD0)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  1 Reserved2 (at FFD2)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  2 Reserved3 (at FFD4)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  3 Vacmp (at FFD6)                 Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  4 Vadc (at FFD8)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  5 Vkeyboard (at FFDA)             Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  6 Viic (at FFDC)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no.  7 Vscitx (at FFDE)                Unassigned */
         VscirxISR,                    /* Int.no.  8 Vscirx (at FFE0)                Used */
         UNASSIGNED_ISR,               /* Int.no.  9 Vscierr (at FFE2)               Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 10 Vspi (at FFE4)                  Unassigned */
         UNASSIGNED_ISR,               /* Int.no. 11 Vmtim (at FFE6)                 Unassigned */
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
         UNASSIGNED_ISR               /* Int.no. 22 Vswi (at FFFC)                  Unassigned */
 };
