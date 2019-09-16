Initial		

;---------------------------------------------------------------------------------
; First we initialize some basic settings
;---------------------------------------------------------------------------------

; initialize ADC ports (comes before PORTA/PORTB and ADC settings)
	banksel	ANSELA
			movlw	b'00000010'		; RA1/AN1 = analog inputs
			movwf	ANSELA			; setup analog inputs for RA

			movlw	b'11000000'		; RB6/AN5, and RB7/AN6 are analog inputs
			movwf	ANSELB			; setup analog inputs for RB
			clrf	ANSELB

; initialize oscillator
	banksel	OSCCON	
			clrf	OSCCON			; use OSC from CONFIG1
			clrf	OSCTUNE			; factory OSC setting
	banksel	CLKRCON
			clrf	CLKRCON			; ref clock disabled
			
; set pullups
	banksel	WPUB
			clrf	WPUB
	banksel OPTION_REG
			bcf		OPTION_REG,7	; PORTB pullups set individually	

; set alternate pin configurations (16f1827)

	banksel APFCON0
			bsf		APFCON0,RXDTSEL		; RX to RB2

	banksel APFCON1
			bsf		APFCON1,TXCKSEL		; TXCKSEL (0=RB2, 1=RB5)

;---------------------------------------------------------------------------------
; initialize PORTA/PORTB
;---------------------------------------------------------------------------------
	
PORT_Init	
	banksel	PORTA
			clrf	PORTA
			clrf	PORTB
	banksel	LATA 					
			CLRF 	LATA 			; clear PORTA Data Latch
			CLRF 	LATB 			; clear PORTB Data Latch

	banksel	TRISA

; updated for 16F1827
;
; RA0 = digital input, J5 pin 1, outside switch for user control
; RA1 = ANALOG input, J5 pin 2, outside knob 3 (AN1)
; RA2 = digital input, DS1 (MIDI CH)
; RA3 = PWM output CCP3, RED, SET the pin to disable output driver for PWM
; RA4 = PWM output CCP4, GREEN, SET the pin to disable output driver for PWM
; RA5 = digital input, DS4 (MIDI CH)
; RA6 = output, oscillator out pin 1
; RA7 = output, oscillator in pin 2

;			16f1827
			movlw	b'00111111'
			movwf	TRISA

; RB0 = digital input, DS2 (MIDI CH)
; RB1 = digital input, DS3 (MIDI CH)
; RB2 = UART input, Midi RX
; RB3 = PWM output CCP1, BLUE, SET the pin to disable output driver for PWM
; RB4 = digital input, DS5 (Standalone on/off)
; RB5 = UART output, Midi TX
; RB6 = ANALOG input, J5 pin 3, outside knob 2 (AN5)
; RB7 = ANALOG input, J5 pin 4, outside knob 1 (AN6)

;			16f1827
			movlw 	b'11011111'
			movwf	TRISB

;---------------------------------------------------------------------------------
; initialize CCP/PWM 
;---------------------------------------------------------------------------------

	banksel	CCPTMRS
;					  ,,_______ C4TSEL 00 = Timer2, 01 = Timer4, 10 = Timer 6
;					  ||,,_____ C3TSEL 00 = Timer2, 01 = Timer4, 10 = Timer 6
;					  ||||,,___ C2TSEL 00 = Timer2, 01 = Timer4, 10 = Timer 6
;					  ||||||,,_ C1TSEL 00 = Timer2, 01 = Timer4, 10 = Timer 6
			movlw	b'01010101'
			movwf	CCPTMRS	

	banksel	PR4
			movlw	d'127'
			movwf	PR4

	banksel	CCP1CON
;					  ,,_______ PxM - CCPxM = 11xx then 00 = single output, PxA modulated
;					  ||,,_____ DCxB - PWM Duty Cycle least significant bits
;					  ||||,,,,_ CCPxM - Mode select - 1100 = PWM, bits 0/1 are enhanced mode
			movlw	b'00001100'
			movwf	CCP1CON
			clrf	CCP2CON
	banksel	CCP3CON
			movlw	b'00001100'
			movwf	CCP3CON
			movwf	CCP4CON

	banksel CCPR1L
			clrf	CCPR1L			; Set PWM off
	banksel CCPR3L
			clrf	CCPR3L			; Set PWM off
	banksel CCPR4L
			clrf	CCPR4L			; Set PWM off

;---------------------------------------------------------------------------------
; Initialize UART
;---------------------------------------------------------------------------------

UART_Init				

	banksel	TXSTA					; also RCSTA, BAUDCON, SPBRGx

			bsf		RCSTA,SPEN  	; doesn't matter, we're in asynch mode, but whatever
			bcf		TXSTA,SYNC		; asynchronus mode (bit 4)		
			bsf 	TXSTA,TXEN		; enable Transmit (bit 5)

			bsf		TXSTA,BRGH		; high speed baud rate (bit 2)
			bcf		BAUDCON,BRG16

			clrf	SPBRGH
			movlw	d'23'			; 12mhz osc w/ high brgh
			movwf	SPBRGL			; init BRG 31250 bauds

			bcf		RCSTA,CREN  	; clear CREN, reset RX
			bsf		RCSTA,CREN  	; continuous receive enabled, restart RX

;---------------------------------------------------------------------------------
; Init ADC
;---------------------------------------------------------------------------------

; Set ADCON1 - configure references, clock divide by 2 (y/n), and how the ADC result is formatted
    banksel ADCON1
;					  Jfoc -++
    		movlw   b'00100000' 	; left justified (8 bit), Fosc/16, vref- = AVSS, vref+ = AVDD
    		movwf   ADCON1
			

; Set ADCON0 - control register, use to turn on ADC, select analog channel, begin conversion and read status
	banksel ADCON0
;					  CHANELgA
			movlw	b'00011001'			; AN6 (bits 2-6), ADC on (bit 0), don't start conversion (bit 1)
    		movwf   ADCON0
			bsf		CurADC,0		; Current ADC = pot1


;---------------------------------------------------------------------------------
; Init interrupts
;---------------------------------------------------------------------------------

	banksel	OPTION_REG

; Set TIMER0 options
;			banksel	OPTION_REG
;					  ,________ PORTB Pull up (0 = enabled, 1 = disabled) 
;					  |,_______ Interrupt Edge select (0 = interrupt on falling edge RB0, 1 = rising)
;					  ||,______ TMR0 Clocl source select (0 = internal instruction, 1 = RA4)
;					  |||,_____ TMR0 Source edge select (0 = low to high, 1 = high to low)
;					  ||||,____ Prescale assignment (0 = Timer0, 1 = WDT)
;					  |||||,,,_ 3 bit prescale value (000 = 2, 111 = 256)
			movlw	b'00001000'
			movwf	OPTION_REG

;			goto	T2Init

	Banksel INTCON
; Set INTERRUPT options
;					  ,________ Global interrupt (0 = disable, 1 = enabled)
;					  |,_______ Peripheral interrupt (0 = disabled, 1 = enabled)
;					  ||,______ TMR0 overflow interrupt (0 = disabled, 1 = enabled)
;					  |||,_____ INTE external interrupt (0 = disabled, 1 = enabled)
;					  ||||,____ IOCIE Intterup on change interrupt (0 = disabled, 1 = enabled)
;					  |||||,___ TMR0 flag (0 = didn't overflow, 1 = has overflown, must be cleared)
;					  ||||||,__ INT flag (0 = ext interrupt didn't occur, 1 = occurred, must be cleared)
;					  |||||||,_ IOCIF flag (0 = non of RB7:RB4 pins changed, 1 = at least one changed)
			movlw	b'11000000'
			movwf	INTCON

	Banksel PIE1
; Set Peripheral Interrupt Enable (PIE) options
;					  ,________ TMR1GIE: Timer1 Gate (0 = disabled, 1 = enabled)
;					  |,_______ A/D Converter interrupt (0 = disabled, 1 = enabled)
;					  ||,______ UART RX interrupt (0 = disabled, 1 = enabled)
;					  |||,_____ UART TX interrupt (0 = disabled, 1 = enabled)
;					  ||||,____ SSP1IE interrupt (0 = disabled, 1 = enabled)
;					  |||||,___ CCP1 interrupt (0 = disabled, 1 = enabled)
;					  ||||||,__ TMR2IE interrupt (0 = disabled, 1 = enabled)
;					  |||||||,_ TMR1 overflow interrupt (0 = disabled, 1 = enabled)
			movlw	b'00100010'
			movwf	PIE1			

			clrf	PIE2		; wont need any of these (Comparators, eeprom, CCP, MSSP1 collision)

;					  ,,_______ unimplemented
;					  ||,______ CCP4IE
;					  |||,_____ CCP3IE
;					  ||||,____ TMR6IE
;					  |||||,___ unimplemented
;					  ||||||,__ TMR4IE
;					  |||||||,_ unimplemented
			movlw	b'00001000'

			movwf	PIE3		; CCP4/3, TMR6IE, TMR4IE
			clrf	PIE4		; BCL2IE, SSP2IE

	banksel	PIR3
			bcf		PIR3,TMR4IF		; clear TMR4IF on PIR3

;---------------------------------------------------------------------------------
; Init timers
;---------------------------------------------------------------------------------

T1Init  ; not implemented currently
;	banksel	T1CON
;			movlw	b'00000100'		; Clock=Fosc/4, 1:1 prescale, dedicated osc off, no sync ext clk, T1 off
;			movwf	T1CON			 
;			clrf	T1GCON			; Timer1 Gate Control register (disabled)

T2Init	; Modstrobe and rate of refreshing PWM values to hardware PWM
	banksel	T2CON		
			movlw	b'00111100'		; Post scale 1:8, TMR2 ON, pre scale 1
			movwf	T2CON	
	Banksel PR2
			movlw	d'255'			; Adjust to fine tune MODSTROBE/PWM refresh
			movwf	PR2

T4Init   ; LED hardware PWM timer
	banksel	T4CON					
			movlw	b'00000100'		; Post scale 1:1, TMR4 ON, pre scale 1:1
			movwf	T4CON	

	banksel	TRISA
			bcf		TRISA,3			; Set RB3 to output, turn on PWM RED
			bcf		TRISA,4			; Set RB3 to output, turn on PWM GREEN
			bcf		TRISB,3			; Set RB3 to output, turn on PWM BLUE

T6Init	; MIDI note color Sliding timer
	banksel	T6CON		
			movlw	b'00101101'		; Post scale 1:8, TMR6 ON, pre scale 16
			movwf	T6CON	
	Banksel PR6
			movlw	d'255'			; This is the magic value to adjust note slide times
			movwf	PR6

			movlb	d'0'			; bank0
		
;---------------------------------------------------------------------------------
; Let's clear all of our variables (it's fun!)
;---------------------------------------------------------------------------------


			call 	delay
		
			clrf 	ByteCount
			clrf 	RateR
			clrf 	RateB
			clrf 	RateG
			clrf 	TargetR
			clrf 	TargetB
			clrf 	TargetG
			clrf 	PWMR
			clrf 	PWMB
			clrf 	PWMG
			clrf	PWMM
			clrf	OscCLR
			clrf	OscCLDE
			clrf	OscDE
			clrf	OscTrigs
			clrf	ModStrobe
			clrf	EvDropCH
			clrf	CurPGCH
			clrf	CurADC
			clrf	ModStrobeC			
			clrf	TXState
			clrf	TXEchoWait
			clrf	RXModeTrig

			movlw	d'2'			
			movwf	OscCount1
			movlw	d'1'			
			movwf	SlideSlow
			movlw 	d'10'
			movwf 	PWMCount
			movlw 	d'127'
			movwf	ModStrobeD
			movlw 	d'255'
			movwf	ModStrobeE
			movwf	RGBdelay

	call	InitBlink

; reset MIDI RX (just in case) and goto main program loop
	
	banksel	RCSTA
			bcf		RCSTA,CREN  		; clear CREN, reset RX
			bsf		RCSTA,CREN  		; continuous receive enabled, restart RX
	movlb	d'0'		; bank 0
					
			goto	Loop

;---------------------------------------------------------------------------------
; Blink RED/GREEN/BLUE when we turn the light on
;---------------------------------------------------------------------------------

InitBlink	
			movlw	d'127'
			movwf	PWMR
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMR
			movlw	d'127'
			movwf	PWMG
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMG
			movlw	d'127'
			movwf	PWMB
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMB

			movlw	d'127'
			movwf	PWMR
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMR
			movlw	d'127'
			movwf	PWMG
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMG
			movlw	d'127'
			movwf	PWMB
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMB
			RETURN

BlinkRed	movf	PWMR,w
			movwf	RestoreRED
			movf	PWMG,w
			movwf	RestoreGRN
			movf	PWMB,w
			movwf	RestoreBLU
			clrf	PWMR
			clrf	PWMG
			clrf	PWMB

			call 	delay
			call 	delay
			call 	delay
			movlw	d'127'
			movwf	PWMR
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMR
			call 	delay
			call 	delay
			call 	delay

			movf	RestoreRED,w
			movwf	PWMR
			movf	RestoreGRN,w
			movwf	PWMG
			movf	RestoreBLU,w
			movwf	PWMB
			RETURN

BlinkGreen	movf	PWMR,w
			movwf	RestoreRED
			movf	PWMG,w
			movwf	RestoreGRN
			movf	PWMB,w
			movwf	RestoreBLU
			clrf	PWMR
			clrf	PWMG
			clrf	PWMB

			call 	delay
			call 	delay
			call 	delay
			movlw	d'127'
			movwf	PWMG
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMG
			call 	delay
			call 	delay
			call 	delay

			movf	RestoreRED,w
			movwf	PWMR
			movf	RestoreGRN,w
			movwf	PWMG
			movf	RestoreBLU,w
			movwf	PWMB
			RETURN

BlinkBlue	movf	PWMR,w
			movwf	RestoreRED
			movf	PWMG,w
			movwf	RestoreGRN
			movf	PWMB,w
			movwf	RestoreBLU
			clrf	PWMR
			clrf	PWMG
			clrf	PWMB

			call 	delay
			call 	delay
			call 	delay
			movlw	d'127'
			movwf	PWMB
			call 	delay
			call 	delay
			call 	delay
			clrf	PWMB
			call 	delay
			call 	delay
			call 	delay

			movf	RestoreRED,w
			movwf	PWMR
			movf	RestoreGRN,w
			movwf	PWMG
			movf	RestoreBLU,w
			movwf	PWMB
			RETURN
