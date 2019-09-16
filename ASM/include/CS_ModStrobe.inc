	movlw	d'0'
	xorwf	ModStrobe,w			; test modstrobe
	btfsc	STATUS,Z			; if it's 0	
	goto	SkipModStrobe		; then don't bother
								; otherwise

	btfss	ModStrobeC,0		; test flag from interrupt
	goto	SkipModStrobe		; skip if not flagged

	clrf	ModStrobeC			; otherwise clear flag

	movf	ModStrobe,w			; Collect ModStrobe
	xorwf	ModStrobeD,w		; Compare it to Counter
	btfss	STATUS,Z			; if they're the same
	goto	MSCmatch
					
	decfsz	ModStrobeD,f		; otherwise dec Counter
	goto	SkipModStrobe		; Get out of here	
	
	movlw	d'127'				; if we somehow hit 0
	movwf	ModStrobeD			; reset the counter
	goto	SkipModStrobe		; and get out of here

MSCmatch

	movlw	d'127'				; 
	xorwf	PWMM,f				; is our master fader at 127?
	btfss	STATUS,Z			; if it isn't 127
	goto	StrobeOn			; gosub
StrobeOff						; otherwise
	clrf	PWMM
	goto 	SkipModStrobe

StrobeOn
	movwf	PWMM				; move 127 to master
	
SkipModStrobe


