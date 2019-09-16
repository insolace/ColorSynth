;******************************************************************************
; RGB Spectrum routine
;******************************************************************************	
	btfss	OscCLDE,0		; Test CLR osc enable
	goto	SkipSpectrum	; if it's off then skip routine

; This section sets the speed of the RGB spectrum color changing
	decfsz	RGBdelay,f			; Decrement our routine slowdown counter
	goto 	SkipSpectrum		; if it isn't 0 move ahead

	movf	OscCLR,w			; grab speed value
	call	TableOT				; lookup it's table value	
	movwf	RGBdelay			; set counter

	decfsz	RGBspeed,f			; Decrement our osc slowdown counter
	goto 	SkipSpectrum		; if it isn't 0 move ahead
	movf	OscCLR,w			; collect OSC speed
	movwf	RGBspeed			; apply to counter and proceed with routine

;******************************************************************************
; This next section detects our current RGB values and moves us forward in the RGB spectrum
; based on those values.  Our RGB spectrum is Red->Magenta->Blue->Cyan->Green->Yellow->Red.
;
; If our RGB values are all zero, we inc red.  Now we have one non-zero value.
;
; If we have one non-zero value, we inc that value until it is 127.  Then we inc the appropriate
; primary color to begin our next color in the spectrum (Red add blue, Blue add green, Green add Red).
; Now we have two non-zero values.
;
; If we have two non-zero values then we must always stay true to the sequence of our RGB spectrum,
; And we must always add before we subtrack.  So when Green and Red are both non-zero (yellow), we
; must inc red until it's 127, and when it's 127 we must dec green.  When green is zero then we only
; have one non-zero RGB value (red), and our code will soon add the next color in our spectrum (blue).
; This same logic applies to magenta (inc blue until 127, then dec red) and cyan (inc green until 127,
; then dec blue).
;
; If our RGB values are three non zero values (white), we must decrement one of those values
; until it is zero (I've chosen blue).  Then we will have two non zero values.
;******************************************************************************

; Note - There is a known issue when using both Blue and Green oscillators simultaneously with the RGB 
; spectrum oscillator, where the color locks to Green.  This has to do with how the all white colors are dealt
; with, and was the best comprimise.  All other oscillator combinations work (red+grn, Blue+red, red, grn, blu).
;
; Note - Code is tested with color sliding from notes, there are a few workarounds below to make it work.

; Start by testing which values are 0 or non 0 (n0)
TestZeros	decf	PWMR,w
			btfsc	WREG,7			; 
			goto	Red0			; Red=0
			decf	PWMG,w			; Red=n0
			btfsc	WREG,7			; 
			goto	Rn0Grn0			; Red=n0 Grn=0
			decf	PWMB,w			; Red=n0 Grn=n0
			btfsc	WREG,7			; 
			goto	RGBisYEL		; Red=n0 Grn=n0 Blu=0 (YELLOW)
									; at this point we have 3 non zero values (WHITE)
;******************************************************************************
		RGBisWhite	movlw	d'1'
					xorwf	PWMR,W			; Test red vs 1
					btfsc	STATUS,Z		; if equal then
					clrf	TargetR			; Set Red target to 0 (avoids complications with sliding notes)
					movlw	d'1'
					xorwf	PWMG,W			; Test green vs 1
					btfsc	STATUS,Z		; if equal then
					clrf	TargetG			; Set Green target to 0 (avoids complications with sliding notes)
 					clrf	RateB			; Clear rate for blue, this is another workaround for sliding notes

					btfsc	OscGreen,7		; if OscGreen=0 (128 = 0)
					goto	WhiteDECBlu		; proceed as normal
					decf	PWMR,f			; otherwise dec red, when it's 0 we'll have two non zero values		
					goto	SkipSpectrum    ; leave
		WhiteDECBlu	decf	PWMB,f			; dec blue, when it's 0 then we'll have two non zero values
					goto	SkipSpectrum    ; leave
;******************************************************************************
Red0		decf	PWMG,w			; 
			btfsc	WREG,7			; 
			goto	Red0Grn0		; Red=0 Grn=0
			decf	PWMB,w			; Red=0 Grn=n0
			btfsc	WREG,7			; 
			goto	RGBisGREEN		; Red=0 Grn=n0 Blu=0
									; at this point Red=0, Grn=n0, Blu=n0 (CYAN)
;******************************************************************************
		RGBisCYAN	movlw	d'127'			; 
					xorwf	PWMG,W			; test GREEN against 127 
					btfsc	STATUS,Z		; if equal then
					goto	CyanDECBlue		; dec Blue and Leave
					incf	PWMG,f			; otherwise inc Green and leave
					goto	SkipSpectrum
		CyanDECBlue	decf	PWMB,f			; dec Blue
					goto	SkipSpectrum	; leave
;******************************************************************************
Rn0Grn0		decf	PWMB,w			; 
			btfsc	WREG,7			; 
			goto	RGBisRED		; Red=n0 Grn=0 Blu=0 (RED)
									; at this point Red=n0, Grn=0, Blu=n0 (MAGENTA)
;******************************************************************************
		RGBisMAG	movlw	d'127'			; 
					xorwf	PWMB,W			; test BLUE against 127 
					btfsc	STATUS,Z		; if equal then
					goto	MagDECRed		; dec Red and Leave
					incf	PWMB,f			; otherwise inc Blue and leave
					goto	SkipSpectrum
		MagDECRed	decf	PWMR,f			; dec Red
					goto	SkipSpectrum	; leave
;------------------------------------------------------------------------------		
		RGBisYEL	movlw	d'127'			; 
					xorwf	PWMR,W			; test RED against 127 
					btfsc	STATUS,Z		; if equal then
					goto	YelDECGrn		; dec Green and Leave
					incf	PWMR,f			; otherwise inc Red and leave
					goto	SkipSpectrum
		YelDECGrn	decf	PWMG,f			; dec Green
					clrf	TargetG			; Change target for sliding if it's enabled
					goto	SkipSpectrum	; leave	
;******************************************************************************
Red0Grn0	decf	PWMB,w			; 
			btfsc	WREG,7			; 
			goto	RGBisBLACK		; Red=0 Grn=0 Blu=0 (black)
									; At this point Red=0, Grn=0, Blu=n0 (BLUE)
;******************************************************************************
		;Below routines handle when RGB only has one non-zero value (Red, Green, or Blue)
		RGBisBLUE	movlw	d'127'			; 
					xorwf	PWMB,W			; test BLUE against 127 
					btfsc	STATUS,Z		; if equal then
					goto	BlueADDGrn		; inc Green and leave, we now have two n0 values (CYAN)
					incf	PWMB,f			; otherwise inc Blue and leave
					clrf	RateB			; clear rate (avoid confusion on sliding notes feature)
					goto	SkipSpectrum	; leave
		BlueADDGrn	incf	PWMG,f			; inc Green
					goto	SkipSpectrum	; leave, we now have two n0 values (CYAN)
;------------------------------------------------------------------------------		
		RGBisGREEN	movlw	d'127'			; 
					xorwf	PWMG,W			; test GREEN against 127 
					btfsc	STATUS,Z		; if equal then
					goto	GrnADDRed		; inc Red and leave, we now have two n0 values (YELLOW)
					incf	PWMG,f			; otherwise inc Green
					clrf	RateG			; clear rate (avoid confusion on sliding notes feature)
					goto	SkipSpectrum	; leave
		GrnADDRed	incf	PWMR,f			; inc Red
					goto	SkipSpectrum	; leave, we now have two n0 values (YELLOW)
;------------------------------------------------------------------------------	
		RGBisRED	movlw	d'127'			; 
					xorwf	PWMR,W			; test RED against 127 
					btfsc	STATUS,Z		; if equal then
					goto	RedADDBlue		; inc Blue and leave, we now have two n0 values (MAGENTA)
					incf	PWMR,f			; otherwise inc Red
					clrf	RateR			; clear rate (avoid confusion on sliding notes feature)
					goto	SkipSpectrum	; leave
		RedADDBlue	incf	PWMB,f			; inc Blue
					goto	SkipSpectrum	; leave, we now have two n0 values (MAGENTA)
;******************************************************************************
		; RGB has no non-zero values (it's black)
		RGBisBLACK 	incf	PWMR,f			; inc Red, now we have one n0 value
					goto	SkipSpectrum    ; leave
;******************************************************************************
SkipSpectrum		; end of RGB Spectrum routine