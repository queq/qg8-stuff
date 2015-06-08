;*******************************************************************
;* This stationery serves as the framework for a user application. *
;* For a more comprehensive program that demonstrates the more     *
;* advanced functionality of this processor, please see the        *
;* demonstration applications, located in the examples             *
;* subdirectory of the "Freescale CodeWarrior for HC08" program    *
;* directory.                                                      *
;*******************************************************************
 
; Include derivative-specific definitions
		INCLUDE 'derivative.inc' 
 
; export symbols
		XDEF _Startup, main, SERV_KBI
		; we export both '_Startup' and 'main' as symbols. Either can
		; be referenced in the linker .prm file or from C/C++ later on
 
 
 
		XREF __SEG_END_SSTACK			; symbol defined by the linker for the end of the stack

; constant section
count:       equ     $64
delta_t:        equ     $65 
 
; variable/data section
MY_ZEROPAGE: SECTION  SHORT				; Insert here your data definition
 
; code section
MyCode:     SECTION
 
SERV_KBI:
		bset    KBISC_KBACK,KBISC		; Pone el ACK en 1, para que el módulo sepa que se reconoció la interrupción
		brset   0,PTAD,BTN_DISP			; Si la interrupción fue causada por SW_Disp (pin 0), hacer la tarea correspondiente
		brset   1,PTAD,BTN_L			; Si la interrupción fue causada por SW_L (pin 1), hacer la tarea correspondiente
		brset   2,PTAD,BTN_R			; Si la interrupción fue causada por SW_R (pin 2), hacer la tarea correspondiente
		bra     END_INT					; Si la interrupción no fue causada por los switches, terminar serv. de interrupción
 
BTN_DISP:
		lda     MTIMCNT
		sta     count
		bra     END_INT
 
BTN_L:
		dec     delta_t
		lda     #$09
		cbeq    delta_t,SET_LOW_LIMIT
		bra     END_INT
 
SET_LOW_LIMIT:
		lda     #$0A
		sta     delta_t
		bra     END_INT
 
BTN_R:
		inc     delta_t
		lda     #$15
		cbeq    delta_t,SET_HIGH_LIMIT
		bra     END_INT
 
SET_HIGH_LIMIT:
		lda     #$14
		sta     delta_t
		bra     END_INT
 
END_INT:
		rti
 
main:
_Startup:
		ldhx   #__SEG_END_SSTACK ; initialize the stack pointer
		txs
		cli      ; enable interrupts
 
		; Conf. watchdog
		lda     #$02
		sta     SOPT1
		; Conf. puertos
		lda     #%00011111				; Pines 0, 1, 2, 3 y 4 de PTB como salidas
		sta     PTBDD
		lda		#%00010000				; Drive strength para puerto 4 (LED)
		sta     PTBDS
		; Conf. KBI
		bclr	KBISC_KBIE,KBISC		; Limpiar bit de enable del KBI
		bclr	KBISC_KBIMOD,KBISC		; Edge detection
		lda     #%00000111				; Rising edge para los puertos 0, 1 y 2 de KBI (switches)				
		sta     KBIES
		lda     #%00000111				
		sta     PTAPE					; Pullup enable para pines 0, 1 y 2 de PTA (switches) 
		sta     KBIPE					; Pin enable para pines 0, 1 y 2 de KBI/PTA (switches) 
		bclr	KBISC_KBACK,KBISC		; Limpiar ACK del KBI
		bset	KBISC_KBIE,KBISC		; Set bit KBIE (interrupt enable)
		; Conf. timer
		lda		#%00000110				; División de la frecuencia de reloj en 64
		sta		MTIMCLK
		lda     #$09					; Valor máximo de conteo = 9
		sta     MTIMMOD
		bset	MTIMSC_TRST,MTIMSC		; Reset al módulo timer
		bclr	MTIMSC_TSTP,MTIMSC		; Limpiar bit de stop del timer, para que inicie a contar
		; Conf. disp
		clra
		sta     count					; Disp. 7 segmentos inicia en 0
		; Conf. delay
		lda     #$0A
		sta     delta_t					; Delay inicia en 10 ciclos de retardo50 (50ms * 10 = 500ms)
		bra		mainLoop
 
mainLoop:
		bra     prnt_disp
 
prnt_disp:
		lda     count
		ora     PTBD					; Máscara para que los bits en 1 del contador se graben en el puerto B
		sta     PTBD
		lda		count
		add     #$F0
		and     PTBD					; Máscara para que los bits en 0 del contador se graben en el puerto B
		sta     PTBD
		bra     retardo
 
retardo:
inf:	lda     #%00010000				; Encendido LED
		ora     PTBD
		sta     PTBD
		ldx     delta_t
loop1:	jsr     retardo50
		dbnzx   loop1
		lda     #%11101111				; Apagado LED
		and     PTBD
		sta     PTBD
		ldx     $65
loop2:	jsr     retardo50
		dbnzx   loop2
		feed_watchdog
		bra    mainLoop

retardo50:
		lda     #$EC
rt1:	psha
		lda     #$70					; Antes #$44
rt2:	dbnza   rt2
		pula
		dbnza   rt1
		rts
