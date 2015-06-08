;=================================================================
; ARCHIVO : main.asm
; PROPÓSITO : 
; NOTAS : Ninguna
;
; REFERENCIA : 
;
; LENGUAJE : IN-LINE ASSEMBLER
; ---------------------------------------------------------------------------
; HISTORIAL
; DD MM AA
; 10 03 15 Creado.
; 17 03 22 Modificado.
;=================================================================


; Include derivative-specific definitions
            INCLUDE 'derivative.inc'

; export symbols
            XDEF _Startup, main
            ; we export both '_Startup' and 'main' as symbols. Either can
            ; be referenced in the linker .prm file or from C/C++ later on
    
            XREF __SEG_END_SSTACK   ; symbol defined by the linker for the end of the stack

; variable/data section
MY_ZEROPAGE: SECTION  SHORT         ; Insert here your data definition

; code section
MyCode:     SECTION
main:
_Startup:
            LDHX   #__SEG_END_SSTACK ; initialize the stack pointer
            TXS
			CLI			; enable interrupts

			LDA     #$2           ;carga el acumulador con el valor de 2'h
            STA     SOPT1         ;desactiva watchdog
              
            ;LDA     PTAPE
            ;ORA     #$8
            ;STA     PTAPE        ;PTAPE PullUp Eneble para el pin [3], //0 Internal pullup device disabled //1 Internal pullup device enabled -> XXXX 1XXXX = 8'h
            
            LDA     PTAPE
            AND     #$F7
            STA     PTAPE         ;PTAPE PullUp Disable para el pin [3], //0 Internal pullup device disabled //1 Internal pullup device enabled -> XXXX 1XXXX = 8'h
            
            LDA     PTADS
            ORA     #$3
            STA     PTADS         ;PTADS Drive Strenght High para para los pines [0] y [1], que son los LED //0 Low //1 High -> XXXX XX11 = 3'h
            
            ;//0 Input //1 Output -> XXXX X111'b = 7'h
            LDA     PTADD         ;Carga el registro PTADD
            ORA     #$7           ;Configurar PTADD [0] como pin de salida 
                                  ;Configurar PTADD [1] como pin de salida
                                  ;Configurar PTADD [2] como pin de salida
            
            ;//0 Input //1 Output -> XXXX 0XXX'b = F7'h
            AND     #$F7          ;Configurar PTADD [3] como pin de entrada            
            STA     PTADD         ;Sobreescribe en registro PTADD

            ;BSET    7,PTBD
            BSET    7,PTBDD
            BRA		mainLoop
            
            
mainLoop:   LDA     PTAD          ;Se guarda en el Acumulador lo que hay en el registro del puerto A completo (6bits)
            AND     #$08           ;Se le realiza un AND al Acumulador con el numero binario 0000 1000
            CBEQA   #$08,LED_ON    ;Si se detecto´ que el interruptor activado por el IR esta encendido, enciende el LED_ON
            CBEQA   #$00,LED_OFF       ;Si no se detecto´ que el interruptor activado por el IR esta encendido, enciende el LED_OFF
            ;JSR     IR_PULSE      ;Se dirije a la subrutina IR_PULSE
            
LED_OFF:    LDA     PTAD          ;Lee el registro PTAD
            ORA     #$1           ;realiza una mascara OR para activar el bit[0]     --- Encender el LED_OFF
            AND     #$FD          ;realiza una mascara AND para desactivar el bit[1] --- Apagar el LED_ON
            STA     PTAD          ;Sobreescribe en registro PTAD
            BRA		IR_PULSE
            
LED_ON:     LDA     PTAD          ;Lee el registro PTAD
            ORA     #$2           ;realiza una mascara OR para activar el bit[1]     --- Encender el LED_ON
            AND     #$FE          ;realiza una mascara AND para desactivar el bit[0] --- Apagar el LED_OFF
            STA     PTAD          ;Aplica el resultado a los pines del puerto A
            BRA		IR_PULSE      ;

IR_PULSE:   BCLR    2,PTAD	      ;Apagar el IR_TX
			BCLR    7,PTBD        ;Apagar LED_INDICADOR
            LDX     #$09           ;			
tBajo:		JSR		DELAY20	      ;Espera un tiempo 9/10 del total del ciclo = 20ms
            DBNZX   tBajo         ;
            BSET    2,PTAD
            BSET    7,PTBD  
			LDX     #$01           ;
tAlto:		JSR		DELAY20	      ;Espera un tiempo 1/10 del total del ciclo = 180ms
            DBNZX   tAlto         ;
			          
            
            feed_watchdog
            BRA    mainLoop
            
DELAY20:	
			LDA		#$8F
RT1:		PSHA
			LDA		#$8F
RT2:		DBNZA	RT2	
			PULA
			DBNZA	RT1
			RTS
