; define variables in GPR on BANK0
; 80 bytes total available
	cblock h'20'
	MidiCH		; MIDI CH indicated by PORTB
	MidiByte	; working register for current RX midi byte
	MidiStatus	; first midi byte (status)
	MidiData1	; second midi byte (note/cc number)
	MidiData2	; third midi byte (note velocity, cc value)
	ByteCount	; used to keep track of which midi byte we're working with
	CurrentByte	; working byte for MIDI interpretation
	StatusFlag	; flag to interperet status byte, bit 0 = note off, 1 = note on, 2 = cc, 3 = pitch bend, 4 = progch
	EvDropCH	; MIDI channel eavesdropping channel.  Bits 0-3 are the channel value, bit 7 set = enabled, clear = disabled
	CurPGCH		; Current program/patch value received from last Program Change
	PWMCount	; counter for PWM, compared to RGB intensity to turn banks on/off during cycle
	PWMR		; red intensity
	PWMG		; green intensity
	PWMB		; blue intensity
	PWMM		; Master intensity 
	PWMR_CUR	; Current R value in the PWM interrupt
	PWMG_CUR	; Current G value in the PWM interrupt
	PWMB_CUR	; Current B value in the PWM interrupt
	PWMM_CUR	; Current M value in the PWM interrupt
	count1		; delay counter
	counta		; delay counter a
	countb		; delay counter b
	TargetR		; For note slide routines
	TargetG		; ""
	TargetB		; ""
	RateR		; ""
	RateG		;  ""
	RateB		; ""
	CounterR	; ""
	CounterG	; ""
	CounterB	; "" 
	RateDiff	; ""
	RateCut		; ""
	SlideSlow	; counter for slowing down our sliding effect
	SlideFlag	; flag for SLIDE function
	OscCount0	; RGB Osc counters
	OscCount1	; ""
	OscTrigs	; Triggers from counters (Count0 = bit0 etc)
	OscDE		; Osc directions and enable, bits 0,1 = Red enable, direction (0 = up, 1 = down), bits 2,3 = Green,bits 4,5 = Blue
	OscCLDE		; Same as OscDE but for color oscillator
	OscCLR		; COLOR oscillator speed (0 = off)
	OscRed		; Red oscillator speed (0 = off)
	OscGreen	; Green oscillator speed (0 = off)
	OscBlue		; Blue oscillator speed (0 = off)
	ORCount		; Counter for Red Oscillator
	OGCount		; Counter for Green Oscillator
	OBCount		; Counter for Blue Oscillator
	OCCount		; Counter for COLOR Oscillator
	wint		; working intensity for notes to white/black
	ModStrobeE	; delay2 counter to slow down strobing
	ModStrobeD	; delay counter to slow down our strobing
	ModStrobeC	; counter for strobing with mod wheel
	ModStrobe	; value of MODSTROBE!
	TabWStore	; Temp byte to store W when calling a table
	RGBspeed	; Speed of RGB cycle
	RGBdelay	; slowdown counter for RGB cycle
	TXEchoWait	; bit 0 high = don't echo MIDI RX until the next status byte
	TXState		; bit 0 high = transmitting, 
	TXBuff		; byte to hold for transmission (3 byte packets only)
	CCNumXmit	; CC Number to transmit
	CCValXmit	; CC value to transmit
	CurADC		; Flags for current ADC (All Clear = pot1, bit0=pot2, bit1=pot3, bit7=ext button is pressed, bit6=mode3, bit5=mode2, bit5/6 clear=mode1)
	ADCCount	; Counter to slow down ADC reading
	CUR7BIT		; Current 7 bit result
	POT17BIT	; most recent unique result from ADC for POT 1
	POT17B2		; 2nd most recent unique result for POT 1
	POT27BIT	; most recent unique result from ADC for POT 2
	POT27B2		; 2nd most recent unique result for POT 2
	POT37BIT	; 7 bit result from ADC (128 values) for POT 3
	POT37B2		; 2nd most recent unique result for POT 3
	RXModeTrig	; Trigger to change modes via RX reception
	RestoreRED	; For blinking mode indicator, stores RED value
	RestoreGRN	; "" green
	RestoreBLU	; "" blue









; <- line 84, That's 80 bytes, all you get for first bank
	endc
