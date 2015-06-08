;**********************************************************************
;       jl16-5.asm - Demonstrate interface to an LCD display
;
;       By Allan Weber, USC Dept. of Electrical Engineering
;
;       This program will print a message on an LCD display
;       using the 4-bit wide interface method
;
;       Port A, bit 1 (0x02) - output to RS (Register Select) input of display
;               bit 2 (0x04) - output to R/W (Read/Write) input of display
;               bit 3 (0x08) - output to E (Enable) input of display
;       Port B, bits 0-7 - Outputs to DB0-DB7 inputs of display.
;
; Revision History
; Date     Author      Description

;**********************************************************************
;http://ee-classes.usc.edu/ee459/library/samples/HC08/jl16-5.asm

; Include derivative-specific definitions
            INCLUDE 'derivative.inc'

; export symbols
            XDEF _Startup, main
            ; we export both '_Startup' and 'main' as symbols. Either can
            ; be referenced in the linker .prm file or from C/C++ later on

            XREF __SEG_END_SSTACK   ; symbol defined by the linker for the end of the stack

; variable/data section
MY_ZEROPAGE: SECTION  SHORT         ; Insert here your data definition

delay2: ds.b    1               ; Delay counts
delay1: ds.b    1
delay0: ds.b    1
nibdat: ds.b    1
prtdat: ds.b    1

MyCode:     SECTION
main:
_Startup:
            LDHX   #__SEG_END_SSTACK ; initialize the stack pointer
            TXS

mainLoop:

start:  LDA     #$2           ;carga el acumulador con el valor de 2'h
        STA     SOPT1         ;desactiva watchdog
        clrh                  ; Clear the H register
        ;clrx                  ; Clear the X register
        
		LDA		#$F0
		STA		PTBDD
		LDA		#$0E
		STA		PTADD
		CLRA
        ;mov     #$ff,PTBDD       ; Set PTB bits 0-7 for output
        ;mov     #$07,PTADD       ; Set PTA bits 1-3 for output

        jsr     init            ; Initialize the LCD dispay

        ldhx    #str1           ; Print string on line 1
        lda     #0
        jsr     strout

        ldhx    #str2           ; Print string on line 2
        lda     #$40
        jsr     strout

stop:   bra     stop

;
; strout - Output a null-terminated string of characters
;       X = RAM address of the string
;       A = display address where string is to start
;
strout: ora     #$80            ; Make A contain a Set Display Address command
        jsr     cmdout
sloop:  lda     ,x              ; Get next character in string
        beq     sdone           ; If null byte, end of string
        jsr     datout          ; Write the character
        aix     #1              ; Increment the X register
        bra     sloop           ; Do it again
sdone:  rts

;
; datout - Output a data byte
; cmdout - Output a command byte
;       A = data or command to go to display
;
datout: mov     #2,PTAD          ; Set R/W, E for 0, RS for 1
        bra     dcout           ; share code with cmdout routine
cmdout: clr     PTAD             ; Set R/W, RS, E for 0
dcout:  jsr     nibout          ; Output upper four bits
        nsa                     ; swap nibbles in accumulator
        jsr     nibout          ; Output lower four bits
        jsr     busywt          ; Wait for busy flag to reset
        rts

;
; nibout - Puts bits 4-7 from the accumulator into bits 4-7 of Port B.
;          Toggle the E line low-high-low.
;          Bits 0-3 of Port B are unchanged and the accumulator is unchanged.
;
nibout: sta     nibdat          ; Save contents of accumulator
        lda     PTBD             ; Get Port B
        and     #$0f            ; Clear the top four bits
        sta     prtdat          ; Store it
        lda     nibdat          ; Restore accumulator
        and     #$f0            ; Clear bottom four bits
        ora     prtdat          ; Combine nibbles
        sta     PTBD             ; Put data in Port B
        bset    3,PTAD           ; Set E for 1
        nop
        bclr    3,PTAD           ; Set E for 0
        lda     nibdat          ; Restore accumulator
        rts

;
; init - Initialize the display
;
init:   mov     #$08,delay2     ; Delay at least 15ms

idel1:  jsr     del1m
        dec     delay2
        bne     idel1

		lda     #$30            ; Send a 0x30 (Function Set = 8-bits)
        jsr     iniout

        mov     #4,delay2       ; Delay at least 4msec
        
idel2:  jsr     del1m
        dec     delay2
        bne     idel2

        lda     #$30            ; Send a 0x30 (Function Set = 8-bits)
        jsr     iniout

        mov     #3,delay2       ; Delay at least 100usec
        
idel3:  jsr     del40u
        dec     delay2
        bne     idel3

        lda     #$30            ; Send a 0x30 (Function Set = 8-bits)
        jsr     iniout

        lda     #$20            ; Function Set; 4-bit interface
        jsr     iniout
        jsr     busywt          ; BUSY flag now working

        lda     #$28            ; Function Set: 4-bit interface, 2 lines
        jsr     cmdout

        lda     #$0f            ; Display and cursor on blinking
        jsr     cmdout
        rts

;
; iniout - Output a nibble to the display.  Only do the top bits since
;       at this stage the display thinks it's in 8-bit mode.
;       Don't wait for BUSY since flag not working during initialization.
;
iniout: clr     PTAD             ; Set R/W, RS, E for 0
        jsr     nibout          ; Output upper four bits
        rts

;
; busywt - Wait for BUSY flag to clear
;
busywt: clr     PTBDD            ; Set PTB 4-7 for input
        mov     #4,PTAD          ; E=0, R/W=1, RS=0
bloop:  bset    3,PTAD           ; Set E to 1
        lda     PTBD             ; Read upper bits of status register
        bclr    3,PTAD           ; Set E to 0
        nop
        bset    3,PTAD           ; Set E to 1
        nop                     ; Don't need to get lower bits
        bclr    3,PTAD           ; Set E to 0
        bmi     bloop           ; If Busy (PTB7=1), loop
        mov     #$f0,PTBDD       ; Set PTB 4-7 for output
        rts

;
; del40u - Delay 40 microseconds by looping
;       Total time is 4 + 13 * (4 + 3) + 4 = 99 cycles
;       A 9.8304MHz external clock gives an internal CPU clock of 2.4576MHz
;       (406ns/cycle).  Delay is then 40.2 usec.  The JSR instruction will
;       add a bit more.
;
del40u: mov     #7,delay0      ; 4 cycles
u1:     dec     delay0          ; 4 cycles
        bne     u1              ; 3 cycles
        rts                     ; 4 cycles

;
; del1m - Delay 1 millisecond by looping
;       Total time is 4 + 5 * (4 + 68 * (4 + 3) + 4 + 3) + 4 = 2443 cycles
;       A 9.8304MHz external clock gives an internal CPU clock of
;       2.4576MHz (407ns/cycle).  Delay is then .994 milliseconds.
;
del1m:  mov     #3,delay1       ; 4 cycles
m1:     mov     #34,delay0      ; 3 cycles
m0:     dec     delay0          ; 4 cycles
        bne     m0              ; 3 cycles
        dec     delay1          ; 4 cycles
        bne     m1              ; 3 cycles
        rts                     ; 4 cycles

str1:   dc.b    ">> Microcontroladores <<"
        dc.b    0
str2:   dc.b    ">> - 2015 - <<"
        dc.b    0
