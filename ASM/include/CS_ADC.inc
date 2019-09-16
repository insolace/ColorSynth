;******************************************************************************
; ADC ROUTINES
; Read our 3 pots (AN6=1, AN5=2, and AN1=3) as well as the outside switch (RA0)
;******************************************************************************	
;
; Note - Pots operate in 3 modes/banks, pressing the outside switch cycles to the
; next mode.  Each mode changes the parameters that the pots adjust, giving us
; nine different adjustable parameters.

;	goto ExitADC

; Test for MODE change from MIDI RX
			btfsc	RXModeTrig,0		; Mode 1 received
			goto	ButtonM3
			btfsc	RXModeTrig,1		; Mode 2 received
			goto	ButtonM1
			btfsc	RXModeTrig,2		; Mode 3 received
			goto	ButtonM2


	banksel	ADCCount		
			btfss	ADCCount,7			; Is ADC Flagged from interrupt?
			goto	ExitADC				; nope, leave
			movlw	d'120'				; 0-127, higher = more often
			movwf	ADCCount			; Reset count, proceed

			btfsc	CurADC,1			; is our current ADC pot3?
			goto	ReadPot3
			btfsc	CurADC,0			; is our current ADC pot2?
			goto	ReadPot2			
										; our current ADC is pot1
ReadPot1
; read AN6 (RGB Spectrum Oscillator)
			call	ReadADC
; This section of code filters out duplicate values
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT17BIT,w			; Test it against our last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot1			; then discard and quit ADC

; (1) This more controversial section of code filters out repetitive single step patterns
; such as (40, 41, 40, 41).  This is because PWM dimming LEDs on the same circuit as 
; the PIC has proven to cause noise in the ADC that cannot be filtered with caps or a
; rolling average.  It is therefore entirely possible that controllerists who wish to
; turn their knob with this kind of precision will consider this solution heresy.  To
; them, I can only recomend that the following six lines be commented out, and the
; LED be dimmed by another microcontroller with a sufficiently isolated power supply.
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT17B2,w			; Test it against our 2nd to last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot1			; then discard and quit ADC
			movf	POT17BIT,w			; Move most recent value
			movwf	POT17B2				; to 2nd to most recent

; This section stores our most recent result for later comparison
			movf	CUR7BIT,w			; collect current value
			movwf	POT17BIT			; Store it as most recent
			movwf	CCValXmit			; store value for CC XMIT

			btfsc	CurADC,6			; Test for Mode3
			goto	Pot1M3
			btfsc	CurADC,5			; Test for Mode2
			goto	Pot1M2

Pot1M1		btfsc	PORTB,4				; test DS5
			goto	SkipP1M1			; if set then don't apply locally
			movf	POT17BIT,w			; collect current value
			sublw	d'128'				; subtract from 128 (invert it)
			movwf	OscCLR				; move value of CC to OSC speed
			bsf		OscCLDE,0			; enable osc
			movlw	d'128'
			xorwf 	OscCLR,w
			btfsc	STATUS,Z			; if Osc is 0
			bcf		OscCLDE,0			; disable osc
SkipP1M1	movlw	d'9'				; CC#9 (RGB Spectrum Oscillator)
			goto	EndPot1Mode

Pot1M2		btfsc	PORTB,4				; test DS5
			goto	SkipP1M2			; if set then don't apply locally
			movf	POT17BIT,w			; collect current value			
			sublw	d'128'				; subtract it from 128
			movwf	OscRed				; move value of CC to OSC speed
			bsf		OscDE,0				; enable osc
			movlw	d'128'
			xorwf 	OscRed,w
			btfsc	STATUS,Z			; if Osc is 0
			bcf		OscDE,0				; disable osc
SkipP1M2	movlw	d'20'				; CC#20 (Red OSC)
			goto	EndPot1Mode

Pot1M3		btfsc	PORTB,4				; test DS5
			goto	SkipP1M3			; if set then don't apply locally
			movf	POT17BIT,w			; collect current value
			movwf	PWMR				; apply to green PWM value
SkipP1M3	movlw	d'14'				; CC#14 (red)

EndPot1Mode	movwf	CCNumXmit			; CC to Xmit
			call 	TxMidiCC

SkipPot1
	movlb	d'0'
			bsf		CurADC,0			; next time ReadPot2
										; Now we need to setup the next ADC channel
										; by the time we return to this subroutine the
										; ADC channel will have settled.
										; If we didn't do this we'd have to have a delay
										; within the ReadADC subroutine, otherwise our ADC
										; channels will experience cross talk.
;					  CHANELgA
			movlw	b'00010101'		; AN5 (bits 2-6), ADC on (bit 0), don't start conversion (bit 1)
			goto	EndADC				; EndADC will write W to ADCON0, setting the next ADC Channel

ReadPot2
; read AN1 (ModStrobe)
			call	ReadADC

; This section of code filters out duplicate values
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT27BIT,w			; Test it against our last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot2			; then discard and quit ADC

; See note (1) above
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT27B2,w			; Test it against our 2nd to last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot2			; then discard and quit ADC
			movf	POT27BIT,w			; Move most recent value
			movwf	POT27B2				; to 2nd to most recent

; This section stores our most recent result for later comparison
			movf	CUR7BIT,w			; collect current value
			movwf	POT27BIT			; Store it as most recent
			movwf	CCValXmit			; store value for CC XMIT

			btfsc	CurADC,6			; Test for Mode3
			goto	Pot2M3
			btfsc	CurADC,5			; Test for Mode2
			goto	Pot2M2

Pot2M1		btfsc	PORTB,4				; test DS5
			goto	SkipP2M1			; if set then don't apply locally
			movf	POT27BIT,w			; collect current value
			movwf	ModStrobe			; store the result as our MODSTROBE value
			movlw 	d'0'				; ...if ModStrobe is 0...
			xorwf	ModStrobe,W			; test it 
			btfsc	STATUS,Z			; if it's 0
			clrf	PWMM				; reset master fader (0 = full)	
SkipP2M1	movlw	d'1'				; CC#1 (ModStrobe)
			goto	EndPot2Mode

Pot2M2		btfsc	PORTB,4				; test DS5
			goto	SkipP2M2			; if set then don't apply locally
			movf	POT27BIT,w			; collect current value			
			sublw	d'128'				; subtract it from 128
			movwf	OscGreen			; move value of CC to OSC speed
			bsf		OscDE,2				; enable osc
			movlw	d'128'
			xorwf 	OscGreen,w
			btfsc	STATUS,Z			; if Osc is 0
			bcf		OscDE,2				; disable osc
SkipP2M2	movlw	d'21'				; CC#21 (Green OSC)
			goto	EndPot2Mode

Pot2M3		btfsc	PORTB,4				; test DS5
			goto	SkipP2M3			; if set then don't apply locally
			movf	POT27BIT,w			; collect current value
			movwf	PWMG				; apply to green PWM value
SkipP2M3	movlw	d'15'				; CC#15 (green)

EndPot2Mode	movwf	CCNumXmit			; CC to Xmit
			call 	TxMidiCC

SkipPot2
			bsf		CurADC,1			; next time ReadPot3

;					  CHANELgA
			movlw	b'00000101'		; Pot1, AN1 (bits 2-6), ADC on (bit 0), don't start conversion (bit 1)
			goto	EndADC

ReadPot3
; read AN5 (PWM Master fader)
			call	ReadADC
; This section of code filters out duplicate values
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT37BIT,w			; Test it against our last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot3			; then discard and quit ADC

; See note (1) above
			movf	CUR7BIT,w			; Collect our current value
			xorwf	POT37B2,w			; Test it against our 2nd to last
			btfsc	STATUS,Z			; If they're equal
			goto	SkipPot3			; then discard and quit ADC
			movf	POT37BIT,w			; Move most recent value
			movwf	POT37B2				; to 2nd to most recent

; This section stores our most recent result for later comparison
			movf	CUR7BIT,w			; collect current value
			movwf	POT37BIT			; Store it as most recent
			movwf	CCValXmit			; store value for CC XMIT

			btfsc	CurADC,6			; Test for Mode3
			goto	Pot3M3
			btfsc	CurADC,5			; Test for Mode2
			goto	Pot3M2

Pot3M1		btfsc	PORTB,4				; test DS5
			goto	SkipP3M1			; if set then don't apply locally
			movf	POT37BIT,w			; collect current value
			movwf	PWMM				; apply to master fader
SkipP3M1	sublw	d'128'				; subtract it from 128
			movwf	CCValXmit			; store inverted value for CC XMIT	
			movlw	d'17'				; CC#17 (master fader)
			goto	EndPot3Mode

Pot3M2		btfsc	PORTB,4				; test DS5
			goto	SkipP3M2			; if set then don't apply locally
			movf	POT37BIT,w			; collect current value			
			sublw	d'128'				; subtract it from 128
			movwf	OscBlue				; move value of CC to OSC speed
			bsf		OscDE,4				; enable osc
			movlw	d'128'
			xorwf 	OscBlue,w
			btfsc	STATUS,Z			; if Osc is 0
			bcf		OscDE,4				; disable osc
SkipP3M2	movlw	d'22'				; CC#22 (Blue OSC)
			goto	EndPot3Mode

Pot3M3		btfsc	PORTB,4				; test DS5
			goto	SkipP3M3			; if set then don't apply locally
			movf	POT37BIT,w			; collect current value
			movwf	PWMB				; apply to blue PWM value
SkipP3M3	movlw	d'16'				; CC#16 (blue)

EndPot3Mode	movwf	CCNumXmit			; CC to Xmit
			call 	TxMidiCC
			goto	SkipPot3

SkipPot3
			bcf		CurADC,0			; next time ReadPot1
			bcf		CurADC,1

;					  CHANELgA
			movlw	b'00011001'			; AN6 (bits 2-6), ADC on (bit 0), don't start conversion (bit 1)

EndADC
	banksel ADCON0
    		movwf	ADCON0
	movlb	d'0'

TestSwitch	btfss	PORTA,0				; test external switch
			goto	ButtonPress			; if pressed then gosub
			bcf		CurADC,7			; button is depressed, clear flag								
			goto	ExitADC

ButtonPress	btfsc	CurADC,7			; bit 7 is flagged if we are waiting for ext button to be depressed
			goto	ExitADC				; if we're still waiting then exit until flag is cleared

			bsf		CurADC,7			; flag bit 7, wont re-enter sub until button is depressed

			btfsc	CurADC,6			; Test for Mode3
			goto	ButtonM3			
			btfsc	CurADC,5			; Test for Mode2
			goto	ButtonM2

ButtonM1	bsf		CurADC,5			; Cue Mode2
			movlw	d'30'				; CC30 = PotMode
			movwf	CCNumXmit
			movlw	d'2'				; Cue Mode2
			movwf	CCValXmit
			btfss	RXModeTrig,1		; skip xmit if we're coming from an RX flag (it would have been echoed)
			call	TxMidiCC
			clrf	RXModeTrig			; Clear mode flag from MIDI RX
			call	BlinkGreen
			goto	ExitADC				; Exit Subroutine

ButtonM2	bsf		CurADC,6			; Cue Mode3
			movlw	d'30'				; CC30 = PotMode
			movwf	CCNumXmit
			movlw	d'3'				; Cue Mode3
			movwf	CCValXmit
			btfss	RXModeTrig,2		; skip xmit if we're coming from an RX flag (it would have been echoed)
			call	TxMidiCC
			clrf	RXModeTrig			; Clear mode flag from MIDI RX
			call	BlinkBlue
			goto	ExitADC				; Exit Subroutine

ButtonM3	bcf		CurADC,5			; Cue Mode1
			bcf		CurADC,6			; ""
			movlw	d'30'				; CC30 = PotMode
			movwf	CCNumXmit
			movlw	d'1'				; Cue Mode1
			movwf	CCValXmit
			btfss	RXModeTrig,0		; skip xmit if we're coming from an RX flag (it would have been echoed)
			call	TxMidiCC
			clrf	RXModeTrig			; Clear mode flag from MIDI RX
			call	BlinkRed
			goto	ExitADC				; Exit Subroutine

;****************************************************************
; Read value from ADC
;****************************************************************

ReadADC 	
	banksel ADCON0
			bsf		ADCON0, 1			; begin conversion
    		btfsc   ADCON0, 1			; (GO_DONE is bit 2 of ADCON)
    		goto    $-1					; wait for ADC to finish

    		movf    ADRESH,W			

	banksel	CUR7BIT
	   		movwf   CUR7BIT				; store 8 bit result
			rrf		CUR7BIT,f			; rotate right 1 bit (11111110 = 01111111)
			bcf		CUR7BIT,7			; clear bit 7, we now have a 7 bit, MIDI friendly result

			RETURN

;*****************
ExitADC	
	movlb	d'0'			; prob redundant, better safe than sorry
