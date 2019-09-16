; Context saving for ISR is no longer required as the 16f1827 does it automatically

ISR			ORG     0x0004				; interrupt vector location

			movlb	d'0'				; bank0
; Test RX
			btfss	PIR1,RCIF			; Test for UART/MIDI RX
			goto	NotURX				; if it's NOT UART RX skip ahead
			call 	RxMidi				; otherwise call MIDI RX
			RETFIE
NotURX		
; Test TX
	banksel PIE1
			btfss	PIE1,TXIE			; Test TX interrupt enable
			goto	NotUTX				; if clear move ahead
	movlb	d'0'		; bank 0
			btfss	PIR1,TXIF			; Test TX interrupt flag
			goto	NotUTX				; if clear skip ahead

			movf	TXBuff,w			; collect TXBuff
	banksel	TXREG
			movwf	TXREG				; transmit
	banksel	PIE1
			bcf		PIE1,TXIE			; disable TX interrupts
	movlb	d'0'		; bank 0
			clrf	TXState				; ready to transmit another packet
			RETFIE

NotUTX
	movlb	d'0'				; bank0
			btfss	PIR3,TMR6IF			; Test for Sliding timer
			goto	NotTMR6				; if it's not TMR6, skip ahead
			bcf		PIR3,TMR6IF			; clear interrupt flag			
			bsf		SlideFlag,0			; set slide flag
			retfie						; return

;CheckTmr0	btfss	INTCON,TMR0IF		; Check if this is timer0
;			goto	CheckTmr2			; if not skip ahead
;			bcf		INTCON,TMR0IF		; otherwise clear interrupt and continue
;
;CheckTmr2	btfss	PIR1,TMR2IF			; check if this is a TMR2 interrupt
;			goto	LeaveInt			; if not then skip

NotTMR6									; now servicing modstrobe and moving RGB to PWM	
			bcf		PIR1,TMR2IF			; clear and continue

BeginMS	   	movlw	d'0'
			xorwf	ModStrobe,w			; test modstrobe
			btfsc	STATUS,Z			; if it's 0	
			goto	SkipMS				; then don't bother

			decfsz	ModStrobeE,f		; dec Counter
			goto	SkipMS				; If 0 move on, otherwise skip

			movf	ModStrobe,w
			movwf	ModStrobeE			; reset the counter	

	movf	ModStrobe,w			; Collect ModStrobe
	xorwf	ModStrobeD,w		; Compare it to Counter
	btfss	STATUS,Z			; if they're the same
	goto	MSCmatch
					
	decfsz	ModStrobeD,f		; otherwise dec Counter
	goto	SkipMS				; Get out of here	
	
	movlw	d'127'				; if we somehow hit 0
	movwf	ModStrobeD			; reset the counter
	goto	SkipMS		; and get out of here

MSCmatch

	movlw	d'127'				; 
	xorwf	PWMM,f				; is our master fader at 127?
	btfss	STATUS,Z			; if it isn't 127
	goto	StrobeOn			; gosub
StrobeOff						; otherwise
	clrf	PWMM
	goto 	SkipMS

StrobeOn
	movwf	PWMM				; move 127 to master

; Apply Master Fader

SkipMS		movf	PWMM,w
			subwf	PWMR, w				; subtract PWMM from PWMR and store in W reg
			btfss	STATUS,C			; if result is negative
			movlw	d'0'				; set W to 0
			movwf	PWMR_CUR			; store result in current PWM value

			movf	PWMM,w
			subwf	PWMG, w				; subtract PWMM from PWMG and store in W reg
			btfss	STATUS,C			; if result is negative
			movlw	d'0'				; set W to 0
			movwf	PWMG_CUR			; store result in current PWM value

			movf	PWMM,w
			subwf	PWMB, w				; subtract PWMM from PWMB and store in W reg
			btfss	STATUS,C			; if result is negative
			movlw	d'0'				; set W to 0
			movwf	PWMB_CUR			; store result in current PWM value

; apply PWM values to LEDs

			movf	PWMR_CUR,w
			movlb	d'6'			; BANK
			movwf	CCPR3L
			movlb	d'0'			; BANK

			movf	PWMG_CUR,w
			movlb	d'6'			; BANK
			movwf	CCPR4L
			movlb	d'0'			; BANK

			movf	PWMB_CUR,w
			movlb	d'5'			; BANK
			movwf	CCPR1L
			movlb	d'0'			; BANK


;------------------------------------------------------------------------------
; USER Interrupt ENDS
;------------------------------------------------------------------------------

; Restoring context before returning from interrupt is no longer required as 16f1827 does it automatically

LeaveInt	movlb	d'0'
			incf	ADCCount,f			; ADC slowdown counter

			RETFIE						; return from interrupt
