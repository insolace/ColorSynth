;******************************************************************************
; RGB Spectrum routine
;******************************************************************************	

	btfss	OscCLDE,0		; Test CLR osc enable
	goto	SkipSpectrum	; if it's off then skip routine


	decfsz	RGBdelay,f			; Decrement our routine slowdown counter
	goto 	SkipSpectrum		; if it isn't 0 move ahead

	movf	OscCLR,w			; grab speed value
	call	TableOT				; lookup it's table value	
	movwf	RGBdelay			; set counter

	decfsz	RGBspeed,f			; Decrement our osc slowdown counter
	goto 	SkipSpectrum		; if it isn't 0 move ahead
	movf	OscCLR,w			; collect OSC speed
	movwf	RGBspeed			; apply to counter and proceed with code


; test if all values are > 0
Test3N0		decf	PWMR,w
			btfsc	STATUS,C		; if RED = 0
			goto	Black2Red		; move along
			decf	PWMG,w
			btfsc	STATUS,C		; if GREEN = 0
			goto	Black2Red		; move along
			decf	PWMB,w
			btfsc	STATUS,C		; if BLUE = 0
			goto	Black2Red		; move along
									; at this point we have 3 non zero values 
			decfsz	PWMB,f			; subtract blue, when it's 0 Test3N0 wont be call


; test flag to see if we're adding RED to black
Black2Red	btfsc	RGBequ,1
			goto	Grn2Yel

; test flag to see if we're subtracting GREEN from white
TestGrnSub	btfss	RGBequ,0
			goto	TestEqu
			decfsz	PWMG,f			; dec PWMG
			goto	SkipSpectrum	; if > 0 skip ahead
			clrf	RGBequ			; clear flag
			goto	SkipSpectrum	; skip ahead

; test to see if any RGB values are equal

TestEqu		movf	PWMG,w			; collect green
			subwf	PWMB,w			; compare to blue
			btfss	STATUS,Z		; if they're not equal		
			goto	GBNotEqual	

GBEqual		movf	PWMG,w			; collect green
			subwf	PWMR,w			; compare to red
			btfss	STATUS,Z		; if equal then
			goto	RGBEqual		; this is white or black
									; otherwise G=B, R<>G
			goto	RedORCyan		; 

RGBEqual	decf	PWMG,w			; dec Green
			btfss	STATUS,C		; if the result is positive (white)
			goto	RGBEqN0			; flag for green subtraction			
			bsf		RGBequ,1		; otherwise flag for red addition (black)
			goto	SkipSpectrum	; exit

RGBEqN0		bsf		RGBequ,0		; RGB are all equal, set flag
			goto	SkipSpectrum	; exit

RedORCyan	movf	PWMR,w			; collect red
			subwf	PWMG,w			; compare to green
			btfss	STATUS,C		; if red > green
			goto 	Red2Mag			; we're red
			goto	Cyan2Grn		; else we're cyan

;-

GBNotEqual	movf	PWMR,w			; collect red
			subwf	PWMB,w			; compare to green
			btfss	STATUS,Z		; if they're not equal		
			goto	RBNotEqual	

RBEqual		movf	PWMB,w			; collect blue
			subwf	PWMG,w			; compare to green
			btfss	STATUS,C		; if blue > green
			goto 	Mag2Blue		; we're Magenta
			goto	Grn2Yel			; else we're green
;-

RBNotEqual	movf	PWMR,w			; collect red
			subwf	PWMG,w			; compare to green
			btfss	STATUS,Z		; if they're not equal		
			goto	RGNotEqual	

RGEqual		movf	PWMB,w			; collect blue
			subwf	PWMG,w			; compare to green
			btfss	STATUS,C		; if blue > green
			goto 	Blue2Cyan		; we're Blue
			goto	Yel2Red			; else we're yellow

;--------------------------------
; Evaluate RGB LT and GT relationships

RGNotEqual	btfss	STATUS,C		; if negative then 
			goto	RedGTGreen		; red > green
									;
			btfss	STATUS,Z		; if not equal then 
			goto	RedLTGrn		; Red < green
									; else Red = Green
			movf	PWMB,w			; collect blue
			subwf	PWMG,w			; subtract from green	
			btfss	STATUS,C		; if negative then 
			goto	Blue2Cyan		; 
			goto	Yel2Red			; else

;--------------------------------
									; less than 
RedLTGrn	movf	PWMB,w			; collect blue
			subwf	PWMG,w			; subtract from green	
			btfss	STATUS,C		; if negative then 
			goto	Blue2Cyan		; blue > green
									; else
GrnGTrb		movf	PWMB,w			; collect blue
			subwf	PWMR,w			; subtract from red	
			btfss	STATUS,C		; if negative then 
			goto	Cyan2Grn		; blue > red
			goto	Grn2Yel			; else



;---------------------------------
RedGTGreen	movf	PWMB,w			; collect blue
			subwf	PWMG,w			; subtract from green	
			btfss	STATUS,C		; if negative then 
			goto	GrnLTrb			; green < red and blue
			goto	Yel2Red			; else

			
GrnLTrb		movf	PWMR,w			; collect red
			subwf	PWMB,w			; subtract from blue	
			btfss	STATUS,C		; if negative then 
			goto	Red2Mag			; Red > Blue > Green
			goto	Mag2Blue		; else

;---------------------------------			

Red2Mag		incf	PWMB,f			; inc blue, 
			movlw	d'128'
			subwf	PWMB,w			; subtract 128
			btfss	STATUS,C		; if negative
			goto	SkipSpectrum	; blue <= 127, move along no problem
			movlw	d'127'	
			movwf	PWMB			; set to 127
			goto	SkipSpectrum	; move along

Blue2Cyan	incf	PWMG,f			; inc green, 
			movlw	d'128'
			subwf	PWMG,w			; subtract 128
			btfss	STATUS,C		; if negative
			goto	SkipSpectrum	; green <= 127, move along no problem
			movlw	d'127'	
			movwf	PWMG			; set to 127
			goto	SkipSpectrum	; move along

Grn2Yel		incf	PWMR,f			; inc red
			movlw	d'128'
			subwf	PWMR,w			; subtract 128
			btfss	STATUS,C		; if negative
			goto	SkipSpectrum	; red <= 127, move along no problem
			movlw	d'127'	
			movwf	PWMR			; set to 127
			clrf	RGBequ			; (saves from having to duplicate Grn2Yel for adding red to black)
			goto	SkipSpectrum	; move along

;------------

Mag2Blue	decf	PWMR,f			; 
			goto	SkipSpectrum	; move along

Cyan2Grn	decf	PWMB,f
			goto	SkipSpectrum	; move along

Yel2Red		decf	PWMG
;			goto	SkipSpectrum	; move along

SkipSpectrum

