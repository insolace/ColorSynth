;****************
;16f1827
;****************
	list p=16f1827

	#include <P16F1827.inc>
;	ERRORLEVEL	0,	-302	;suppress bank selection messages

	__CONFIG    _CONFIG1, _FCMEN_OFF & _IESO_OFF & _CLKOUTEN_OFF & _BOREN_OFF & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _PWRTE_ON & _WDTE_OFF & _FOSC_HS
	__CONFIG    _CONFIG2, _LVP_OFF & _STVREN_OFF & _PLLEN_OFF & _WRT_OFF & _BORV_19



	#include "../include/CS_variables.inc"			; cblock variable definitions
	#include "../include/CS_MPLAB.inc"				; MPLAB DEFINE, EQU, shortcuts etc

			ORG     0x0000							; processor reset vector
 			GOTO    Initial							; go to Initialization

			ORG     0x0004							; interrupt vector location

	#include "../include/CS_Interrupt.inc"			; Interrupt handler

;------------------------------------------------------------------------------
; MAIN PROGRAM
;------------------------------------------------------------------------------
Loop

		banksel	RCSTA								; banksel RCSTA
			btfsc	RCSTA,OERR						; test for MIDI RX overflow
			goto	InitBlink						; if set then blink and reset
  		banksel PIR1				; bank0
													; otherwise
	#include "../include/CS_Spectrum.inc"
	#include "../include/CS_OSC.inc"
	#include "../include/CS_Slide.inc"
	#include "../include/CS_ADC.inc"

			goto Loop

	#include "../include/CS_init.inc"				; initialization routine


;****************************************************************
;	SUBROUTINES
;****************************************************************

	#include "../include/CS_RxMIDI.inc"
	#include "../include/CS_XMIT.inc"
	#include "../include/CS_Delay.inc"
	#include "../include/CS_Tables.inc"

;****************************************************************
			

			end