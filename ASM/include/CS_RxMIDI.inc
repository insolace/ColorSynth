 ;*****************************************************
RxMidi	; can't have a banksel on the same line as a label
			banksel RCREG				; 16f1827 = bank 3 for RC/TXREG
				movf	RCREG,W			; collect our byte
			movlb	d'0'				; bank0
				movwf	CurrentByte		; store it for now

; Test if we should echo the byte or not
				btfsc	TXState,0		; are we currently transmitting?
				goto	SkipRXEcho		; if yes, skip
				btfss	PIR1,TXIF		; is TXREG full?
				goto	SkipRXEcho		; if yes, skip
				btfss	TXEchoWait,0	; Did we just finish transmitting data from pots? 
				goto	RXEcho			; if not then echo
				btfss	CurrentByte,7	; otherwise test if this is a status byte
				goto	SkipRXEcho		; if not then skip
				clrf	TXEchoWait		; if it is a status byte then clear flag and echo
			
RXEcho			movf	CurrentByte,w	; collect current byte
			banksel TXREG				
				movwf	TXREG			; echo to MIDI OUT/THRU
			movlb	d'0'				; bank0
			

SkipRXEcho		btfss	CurrentByte,7	; Status or Data byte?
				goto	RxDataByte

; This is a Status Byte
				
				btfss	CurrentByte,6	; 0 = Note On, Off, Poly Pres, CC
				goto	RxNoPolyCC
		
				btfss	CurrentByte,5	; 0 = ProgCh, ChanPres (only 1 data byte)
				goto	RxPrgChPres
	
				btfss	CurrentByte,4	; 0 = Pitch Bend
				goto	RxPitchBend
	
;-------- System Messages
	
				btfsc	CurrentByte,3	; 1 = 248-255 (one byte system msgs to ignore)
				return
				
				btfsc	CurrentByte,2	; 1 = 244-247 (one byte system msgs to ignore)
				return
	
				clrf	StatusFlag		; 240-243 break running status
				return
	
;-------------------------------------------------------------------------------------------

;=========== 10xx-CCCC
RxNoPolyCC		btfss	CurrentByte,5	; 0 = note on/off
				goto	RxNote
				
				btfss	CurrentByte,4	; 0 = PolyPresure
				goto	RxPolyP
	
RxCC			clrf	StatusFlag
				bsf		StatusFlag,2	; bit 2 high = CC					
				goto	TwoDataB
			
	
RxNote			clrf	StatusFlag
				btfss	CurrentByte,4	; 0 = NoteOn
				bsf		StatusFlag,0	; bit 0 high = NoteOff
				btfsc	CurrentByte,4	; 1 = NoteOn
				bsf		StatusFlag,1	; bit 1 high = NoteOn

TwoDataB		movlw	b'00000011'			
				movwf	ByteCount		; two data bytes incomming

				movf	CurrentByte,w
				movwf	MidiStatus		; Update status for CH later

				return

RxPolyP			clrf	StatusFlag
				Return
;-------------------------------------------------------------------------------------------


;=========== 110x-CCCC (only 1 data byte follows)
RxPrgChPres		btfsc	CurrentByte,4	; 1 = ChanPressure
				goto	RxChanP
	
RxProgCh		clrf	StatusFlag ; ProgCh sub goes here (1 data byte follows)
				return

RxChanP			clrf	StatusFlag
				Return
;-------------------------------------------------------------------------------------------

RxPitchBend 	clrf	StatusFlag ; PitchBend sub goes here
				return

;===========================================================================================
RxDataByte		btfss	ByteCount,0		; 0 = unexpected data byte (running status?)
				goto 	RxRunStat
				btfss	ByteCount,1		; 0 = This is our last byte
				goto	ApplyMidi
				
RxOneMore		movf	CurrentByte,w
				movwf	MidiData1		; Store data byte

				bcf		ByteCount,1		; ByteCount = b'00000001' = We have one byte left to collect 
				return

RxRunStat		bsf		ByteCount,0
				goto	RxOneMore

ApplyMidi		movf	CurrentByte,w
				movwf	MidiData2		; Store data byte
				
				clrf	ByteCount		; clear bytecount


; Temporary for breadboarding, remove when code is live and all dip switches are connected
;		goto CHMatches


; This code filters our MIDI data against our channel set via various dipswitches

				clrf 	MidiCH

				btfsc	DS4PORT,DS4		; bit 0
				bsf		MidiCH,0

				btfsc	DS3PORT,DS3		; bit 1
				bsf		MidiCH,1

				btfsc	DS2PORT,DS2		; bit 2
				bsf		MidiCH,2

				btfsc	DS1PORT,DS1		; bit 3
				bsf		MidiCH,3


CheckCH			; Check our dip switches against our MIDI status byte
				
				movf	MidiStatus,w		
				xorwf	MidiCH,f		; xor and store into MidiCH
				btfsc	MidiCH,0		; bit 0 high means 
				goto    SwNoMatch		; Our dipswitches don't match our Status Byte
				btfsc	MidiCH,1		; bit 1 high means 
				goto    SwNoMatch		; Our dipswitches don't match our Status Byte
				btfsc	MidiCH,2		; bit 2 high means 
				goto    SwNoMatch		; Our dipswitches don't match our Status Byte
				btfsc	MidiCH,3		; bit 3 high means 
				goto    SwNoMatch		; Our dipswitches don't match our Status Byte

SwDoesMatch		btfss	EvDropCH,7		; Our switches match, is eavesdropping enabled?
				goto 	CHMatches		; dip switches match data, e/d is disabled, so use the data

										; (***) At this point switches match, e/d is enabled, is our MIDI data it a CC25? (***)

										; test MidiStatus bits 4,5 an 6 - is this a control change?
				btfss	MidiStatus,4	; we want set
				goto 	CheckEDvsSB		; it's not, check if our status byte CH matches our e/d value
				btfss	MidiStatus,5	; we want set
				goto 	CheckEDvsSB		; it's not, check if our status byte CH matches our e/d value
				btfsc	MidiStatus,6	; we want clear
				goto 	CheckEDvsSB		; it's not, check if our status byte CH matches our e/d value

				movlw 	d'25'			; Yes this is a control change, is this a CC 25 (Eavesdropping)?
				xorwf	MidiData1,W		; test it 
				btfss	STATUS,Z		; if not equal, then
				goto 	CheckEDvsSB		; check if our status byte CH matches our e/d value

				goto 	Eavesdrop2		; This is a CC25 and it matches our dip switches, goto eavesdropping subroutine

SwNoMatch		btfss	EvDropCH,7		; Our switches don't match, is eavesdropping enabled?
				return					; switches don't match, e/d is disabled, so ignore this data
										
										; (***) At this point our switches don't match, but e/d is enabled (***)

CheckEDvsSB								; e/d is enabled, check if our status byte CH matches our e/d value
				movf	EvDropCH,w		; move eavesdropping value to w 
				movwf	MidiCH			; store it in MidiCH
				movf	MidiStatus,w	; move MIDI status byte to w
				xorwf	MidiCH,f		; xor and store into MidiCH
				btfsc	MidiCH,0		; bit 0 high means 
				return					; Our e/d value doesn't match our Status Byte
				btfsc	MidiCH,1		; bit 1 high means 
				return					; Our e/d value doesn't match our Status Byte
				btfsc	MidiCH,2		; bit 2 high means 
				return					; Our e/d value doesn't match our Status Byte
				btfsc	MidiCH,3		; bit 3 high means 
				return					; Our e/d value doesn't match our Status Byte
										; otherwise we have a match, let's use our MIDI data!

;--------------------------------------------------------------------------------------------------------------
; At this point our data matches our MIDI channel, either from dip switches or from MIDI channel eavesdropping.
; So lets figure out what kind of BYTE we have and act accordingly

CHMatches		btfsc	StatusFlag,0	; bit 0 high = NoteOff
				goto	ApplyNoteOff

				btfsc	StatusFlag,1	; bit 1 high = NoteOn
				goto	ApplyNoteOn

				btfsc	StatusFlag,2	; bit 2 high = NoteCC
				goto	ApplyCC

				btfsc	StatusFlag,3	; bit 3 high = PitchBend
				goto	ApplyPitchBend

				btfsc	StatusFlag,4	; bit 4 high = ProgramChange
				goto	ApplyProgCh	
				
				return					; StatusFlag = 0 means don't process this data

ApplyPitchBend	return					; not implemented

;-Apply program changes to Color Palette!
ApplyProgCh		clrf	CurPGCH			; clear the Current Program Change byte
				btfsc	MidiData2,0		; if bit 0 of our PGCH is set
				bsf		CurPGCH,0		; set bit 0 of CurPGCH
				btfsc	MidiData2,1		; etc
				bsf		CurPGCH,1		; ...
										; (Since we only recognize four program changes,
										; we'll just make them repeat)
				return

Byte2Fini	return


;----------------------------------------------------------

ApplyCC

;-Color Palette-----
ColorPal	movlw	d'23'			; CC 23
			xorwf	MidiData1,W		; test it 
			btfsc	STATUS,Z		; if equal then
			goto 	ApplyProgCh     ; Change Pallette


;-RED FADER---------
RedSlider	movlw 	d'14'			; CC 14
			xorwf	MidiData1,W		; test it 
			btfsc	STATUS,Z		; if equal then
			goto 	B3Red
;-BLU FADER---------
			movlw 	d'15'			; CC 15
			xorwf	MidiData1,W		; test it 
			btfsc	STATUS,Z		; if equal then
			goto 	B3Green
;-GRN FADER---------
			movlw 	d'16'			; CC 16
			xorwf	MidiData1,W		; test it 
			btfsc	STATUS,Z		; if equal then
			goto 	B3Blue

;-ColorSlider128---
			movlw 	d'3'			; CC 3 (color slider)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal then
			goto 	TestMod 		; skip ahead
									; otherwise
			movf	MidiData2,w		; Collect CC value
			call	table128red		; pull that record from red table
			movwf	PWMR			; Apply it to RED intensity
			movlb	d'0'

			movf	MidiData2,w		; collect cc val
			call	table128grn		; pull record from green table
			movwf	PWMG			; Apply it to GREEN intensity
			movlb	d'0'

			movf	MidiData2,w		; collect cc val
			call	table128blu		; pull record from blue table
			movwf	PWMB			; Apply it to BLUE intensity
			movlb	d'0'

			return

;-MOD STROBE!-----
TestMod		movlw 	d'1'			; CC 1 (mod wheel)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal then
			goto 	Eavesdrop		; skip ahead
									; otherwise
			movf	MidiData2,W		; move our CC value to W
			movwf	ModStrobe		; store the result as our MODSTROBE value

			movlw 	d'0'			; ...if ModStrobe is 0...
			xorwf	ModStrobe,W		; test it 
			btfsc	STATUS,Z		; if it's 0
			clrf	PWMM			; reset master fader (0 = full)	
			return

;-MIDI channel eavesdropping -----------
Eavesdrop	movlw 	d'25'			; CC 25 (Eavesdropping)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal then
			goto 	PotMode			; skip ahead
									; otherwise
Eavesdrop2	movf	MidiData2,W		; move our CC value to W
			movwf	EvDropCH		; store it as our Eavesdropping value

			movlw 	d'0'			; Is CC25 0?  (0 = turn eavesdropping off)
			xorwf	EvDropCH,W		; test it 
			btfsc	STATUS,Z		; if equal to zero then
			goto 	PotMode			; our flag is clear, eavesdropping is off, move along
									; if it's anything other than 0.....
EvDropOn	decf	EvDropCH,f		; subtract by one
			bsf		EvDropCH,7		; set our flag, which we will check when we receive MIDI status bytes
			return

;-POT Modes!------------------------------
PotMode		movlw 	d'30'			; CC 6 (PotMode)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal then
			goto 	InvertMidi2		; skip ahead
									; otherwise	
			movlw	d'1'
			xorwf	MidiData2,W		; test it 
			btfsc	STATUS,Z		; if equal then
			bsf		RXModeTrig,0	; Set Mode1

			movlw	d'2'
			xorwf	MidiData2,W		; test it 
			btfsc	STATUS,Z		; if equal then
			bsf		RXModeTrig,1	; Set Mode2

			movlw	d'3'
			xorwf	MidiData2,W		; test it 
			btfsc	STATUS,Z		; if equal then
			bsf		RXModeTrig,2	; Set Mode3


; from this point on we're inverting Byte 2, keep all checks/subs that need it below here
InvertMidi2	movf	MidiData2,w		; move Byte2 to W
			sublw	d'128'			; subtract it from 128
			movwf	MidiData2		; store inverted byte

;-MST FADER-------
TestMast	movlw 	d'17'			; CC 17
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal then
			goto 	ClrOsc			; skip ahead
									; otherwise

			movf	MidiData2,W		; move our CC value to W
			movwf	PWMM			; store the result as our master fader value
			decf	PWMM,f			; Adjust for hardware PWM fun
			return

;-COLOR OSC---------
ClrOsc		movlw 	d'9'			; CC 9 (osc Color)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not 9
			goto 	RxOsc			; skip
			
			movf	MidiData2,w		; otherwise setup osc
			movwf	OscCLR			; move value of CC to OSC speed
			bsf		OscCLDE,0		; enable osc
			movlw	d'128'
			xorwf 	OscCLR,w
			btfsc	STATUS,Z		; if Velocity/Osc is 0
			bcf		OscCLDE,0		; disable osc
			return

;-RED OSC---------
RxOsc		movlw 	d'20'			; CC 20 (osc Red)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not 20
			goto 	TestCC21		; skip

			movf	MidiData2,w		; otherwise setup osc
			movwf	OscRed			; move value of CC to OSC speed
			bsf		OscDE,0			; enable osc
			movlw	d'128'
			xorwf 	OscRed,w
			btfsc	STATUS,Z		; if Velocity/Osc is 0
			bcf		OscDE,0			; disable osc
			return

;-GRN OSC---------		
TestCC21	movlw 	d'21'			; CC 21 (osc Green)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not 20
			goto 	TestCC22		; skip

			movf	MidiData2,w		; otherwise setup osc
			movwf	OscGreen		; move value of CC to OSC speed
			bsf		OscDE,2			; enable osc
			movlw	d'128'
			xorwf 	OscGreen,w
			btfsc	STATUS,Z		; if Velocity/Osc is 0
			bcf		OscDE,2			; disable osc
			return

;-BLU OSC---------
TestCC22	movlw 	d'22'			; CC 22 (osc Blue)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not 20
			goto 	TestBO			; skip

			movf	MidiData2,w		; otherwise setup osc
			movwf	OscBlue			; move value of CC to OSC speed
			bsf		OscDE,4			; enable osc
			movlw	d'128'
			xorwf 	OscBlue,w
			btfsc	STATUS,Z		; if Velocity/Osc is 0
			bcf		OscDE,4			; disable osc
			return

;-Blackout

TestBO		movlw 	d'19'			; CC 19 (blackout)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal
			goto 	TestResetAB		; skip

			clrf	PWMR			; blackout
			clrf	PWMG
			clrf	PWMB
			return

;-Reset And Blackout------------
TestResetAB	movlw 	d'27'			; CC 27 (reset and blackout)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal
			goto 	TestReset		; skip

			clrf	PWMR			; blackout
			clrf	PWMG
			clrf	PWMB
			goto	ResetOSRM		; reset Osc Strobe Rate and Master

;-Reset------------
TestReset	movlw 	d'26'			; CC 26 (reset)
			xorwf	MidiData1,W		; test it 
			btfss	STATUS,Z		; if not equal
			goto 	TestCCFini		; skip

ResetOSRM	clrf	ModStrobe		; reset Osc Strobe Rate and Master
			clrf	ModStrobeC
			clrf	RateR
			clrf	RateG
			clrf	RateB
			clrf	OscDE
			clrf	OscRed
			clrf	OscGreen
			clrf	OscBlue
			clrf	OscTrigs
			clrf	ORCount
			clrf	OGCount
			clrf	OBCount
			clrf	PWMM
			clrf	OscCLDE
			clrf	OscCLR
			return


;----------
TestCCFini	clrf 	ByteCount		; probably not necessary
			return					; this byte is not for us

;----------------------------------------------------------

B3Red
			movf 	MidiData2, W	; move our second data byte


			movwf	PWMR			; to red intensity register
			return
B3Green
			movf 	MidiData2, W	; move our second data byte
			movwf	PWMG			; to green intensity register
			return
B3Blue
			movf 	MidiData2, W	; move our second data byte
			movwf	PWMB			; to blue intensity register
			return

;----------------------------------------------------------
;----------------------------------------------------------
; Abandon all hope yee who enter the bloated section of
; code that deals with NoteOn events
;----------------------------------------------------------
;----------------------------------------------------------

ApplyNoteOff
		clrf	MidiData2

ApplyNoteOn
		movlw	d'0'
		xorwf	MidiData2,w		; 
		btfsc	STATUS,Z		; If velocity = 0 (noteoff)
		return					; then ignore

		movf	MidiData2,w		; Sample our velocity, move to W
		movwf	RateR			; move to our Rate		
		movwf	RateG
		movwf	RateB


ChPal	movlw	d'0'			; PCHG 0 = 24+8 color palette
		xorwf 	CurPGCH,w		; 
		btfsc	STATUS,Z		; if equal then
		goto	Pal248			; Go get our color

;		pageselw table128red	; (setup page for tables)

		movlw	d'1'			; PCHG 1 = 128 color palette
		xorwf 	CurPGCH,w		; 
		btfsc	STATUS,Z		; if equal then
		goto	Pal128			; Go get our color


Pal64	bcf		MidiData1,6		; clear bit 6 and 0-63 is the same as 64-127
		rlf		MidiData1,f		; rotate left (multiply x 2) to compensate
		bcf		MidiData1,0		; The bit carries, get rid of it!
		
		movlw	d'2'			; PCHG 2 = 64 color palette
		xorwf 	CurPGCH,w		; 
		btfsc	STATUS,Z		; if they're equal then
		goto	Pal128			; Go get our color

Pal32   bcf		MidiData1,6		; clear bit 6 (bit 5 rotated left) and you limit to 32 values
		rlf		MidiData1,f		; roate left (multiply x 2 again) to compensate
		bcf		MidiData1,0		; The bit carries, get rid of it!


Pal128	movf	MidiData1,w		; Collect note value

		call	table128red		; pull that record from red table
		movwf	TargetR			; Apply it to RED intensity

		movf	MidiData1,w		; collect note value
		call	table128grn		; pull record from green table
		movwf	TargetG			; Apply it to GREEN intensity

		movf	MidiData1,w		; collect note value
		call	table128blu		; pull record from blue table
		movwf	TargetB			; Apply it to BLUE intensity
		return

;---------------------
; Use a 24+8 palette
;---------------------
Pal248	bcf		MidiData1,7	;
		bcf		MidiData1,6	;
		bcf		MidiData1,5	; convert byte to one of 32 possibilities
		btfsc	MidiData1,4	; test RMBC or GYWK
		goto 	m3gywk		; if set it's green, yellow, or black/white
							; otherwise it's red, magenta, blue, or cyan

m3rmbc	btfsc	MidiData1,3	; test if RM or BC, 
		goto	m3bc		; set = Blue/Cyan
							; clear = Red/Magenta

m3rm	bcf		MidiData1,3	; we wont need bit 3 anymore
		clrf	TargetG		; Red/Magenta have no green
		movlw	d'127'		; next PWM set will be 127
		btfsc	MidiData1,2	; test if red or magenta
		goto	m3mag		; set = magenta
							; clear = red

;*[RED*]********************************************************	
m3red	movwf	TargetR		; it's red, set red full
		clrf	TargetB		; set blue to 0
		movlw	d'0'
		xorwf	MidiData1,w	; 
		btfsc	STATUS,Z	; if MidiData1 is zero then
		return	; done
	 	movlw	d'32'		; otherwise add 32
m3redL	addwf	TargetB,f	; to PWMB
		decfsz	MidiData1,f	; loop until MidiData1 = 0
		goto	m3redL

		goto 	Byte2Fini	; all banks set, done	

;*[MAGENTA]*****************************************************
m3mag	movwf	TargetB		; Magenta has full blue and some red
		bcf		MidiData1,2	; clear bit 2, making MidiData1 binary 0-3 
		incf	MidiData1,f	; offset for upcomming dec (MidiData1 is now binary 1-4)
		movlw	d'159'		; 
		movwf	TargetR		; 127+32=159
		movlw	d'32'
m3magL	subwf	TargetR,f	
		decfsz	MidiData1,f	; loop until MidiData1=0
		goto 	m3magL		; MidiData1 1=127, 2=96, 3=64, 4=32	
		goto 	Byte2Fini


;****************************************************************
;****************************************************************
m3bc	bcf		MidiData1,3	; if we're blue or cyan we must clear bit 3

		clrf	TargetR		; blue/cyan have no red
		movlw	d'127'		; next PWM set will be 127
		btfsc	MidiData1,2	; test if blue or cyan
		goto	m3cyn		; if set it's cyan
	
;*[BLUE*]********************************************************
m3blu	movwf	TargetB		; other wise it's blue, set blue full
		clrf	TargetG		; set green to 0
		movlw	d'0'
		xorwf	MidiData1,w	; 
		btfsc	STATUS,Z	; if MidiData1 is zero then	
		return	; done
	 	movlw	d'32'		; otherwise add 32
m3bluL	addwf	TargetG,f	; to PWMG
		decfsz	MidiData1,f	; loop until MidiData1 = 0
		goto	m3bluL
		goto 	Byte2Fini	; all banks set, done	

;*[CYAN]*********************************************************
m3cyn	movwf	TargetG		; cyan has full green and some blue
		bcf		MidiData1,2	; clear bit 2, making MidiData1 binary 0-3 
		incf	MidiData1,f	; offset for upcomming dec (MidiData1 is now binary 1-4)
		movlw	d'159'		; 
		movwf	TargetB		; 127+32=159
		movlw	d'32'
m3cynL	subwf	TargetB,f	
		decfsz	MidiData1,f	; loop until MidiData1=0
		goto 	m3cynL		; MidiData1 1=127, 2=96, 3=64, 4=32	
		goto 	Byte2Fini
;****************************************************************
;****************************************************************

m3gywk	bcf		MidiData1,4	; if its GYWK we need to clear bit 4
		btfsc	MidiData1,3	; test if GY or WK
		goto	m3wk

m3gy	clrf	TargetB		; clear blue as GY doesn't have any
		movlw	d'127'		; next PWM set will be 127
		btfsc	MidiData1,2	; test if yellow or green
		goto	m3yel		; if set it's yellow


;*[GREEN]********************************************************
m3grn	movwf	TargetG		; other wise it's green, set green full
		clrf	TargetR		; set red to 0
		movlw	d'0'
		xorwf	MidiData1,w	;
		btfsc	STATUS,Z	; if MidiData1 is zero then
		return	; done
	 	movlw	d'32'		; otherwise add 32
m3grnL	addwf	TargetR,f		; to PWMR
		decfsz	MidiData1,f	; loop until MidiData1 = 0
		goto	m3grnL
		goto 	Byte2Fini	; all banks set, done	

;*[YELLOW]*******************************************************
	
m3yel	movwf	TargetR		; yellow has full red and some green
		bcf		MidiData1,2	; clear bit 2, making MidiData1 binary 0-3 
		incf	MidiData1,f	; offset for upcomming dec (MidiData1 is now binary 1-4)
		movlw	d'159'		; 
		movwf	TargetG		; 127+32=159
		movlw	d'32'
m3yelL	subwf	TargetG,f	
		decfsz	MidiData1,f	; loop until MidiData1=0
		goto 	m3yelL		; MidiData1 1=127, 2=96, 3=64, 4=32	
		goto 	Byte2Fini

;*[BLACK/WHITE***************************************************
m3wk	bcf		MidiData1,3	; reduce to first 3 bits to get binary 0-7
		incf	MidiData1,f	; offset the coming dec
		movlw	d'144'		; 126+18 = 144
		movwf	wint		; set our working intensity
		movlw	d'18'		; 127/7 ~= 18
m3wkL	subwf	wint,f		; otherwise subtract 18	from intensity
		decfsz	MidiData1,f	; loop until 
		goto 	m3wkL		; MidiData1 is 0
		movf	wint,w
		movwf	TargetR		; set all LEDs to wint
		movwf 	TargetG		;
		movwf 	TargetB		;
		return	; done 
