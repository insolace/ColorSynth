			decfsz	OscCount1,f		; Decrement our delay counter
			goto	EndOsc			; if it isn't 0, skip ahead
			movlw	d'190'			; otherwise reset counter
			movwf	OscCount1		; and proceed with code

OscTestR	btfss	OscDE,0			; Test Red osc enable
			goto	OscTestG		; if it's clear, skip
		
			btfss	OscTrigs,0		; test for our counter flag	
			goto 	OscTestG		; if it's 0 then skip ahead
									; otherwise:
			bcf		OscTrigs,0		; clear flag
			btfsc	OscDE,1			; test direction
			goto	OscRedDown		; 1 = decrement

OscRedUp	
			incf	PWMR,f
			movlw	d'127'	
			subwf	PWMR,w			; Is PWM 127 or greater?

			btfsc	STATUS,Z		; test by subtracting 127
			bsf		OscDE,1			; if zero, PWM = 127 so change direction

			btfss	STATUS,C		; test if 128 
			goto	ClrORCount		; if negative then PWM < 127, skip ahead
									; otherwise PWM=128
			bsf		OscDE,1			; set direction 
			movlw	d'126'			; we were at 127
			movwf	PWMR			; so lets goto 126
			goto	ClrORCount		; clear counter and get out		

OscRedDown	movlw	d'1'
			subwf	PWMR,f			; subtract 1 from PWM

			btfsc	STATUS,Z		; test
			bcf		OscDE,1			; z=1 means PWM is 0, change dir

			btfsc	STATUS,C		; test 
			goto 	ClrORCount		; c=1 means PWM >= 0, skip ahead
									; otherwise we have -1
			bcf		OscDE,1			; change direction
			movlw	d'1'			; we were at 0
			movwf	PWMR			; lets move it to 1


ClrORCount	clrf	ORCount

OscTestG	btfss	OscDE,2			; Test Green osc enable
			goto	OscTestB		; if it's clear, skip

			btfss	OscTrigs,1		; test for our counter flag	
			goto 	OscTestB		; if if it's 0 skip ahead
									; otherwise:
			bcf		OscTrigs,1		; clear flag
			btfsc	OscDE,3			; test direction
			goto	OscGrnDown		; 1 = decrement

OscGrnUp	
			incf	PWMG,f
			movlw	d'127'	
			subwf	PWMG,w			; Is PWM 127 or greater?
			btfsc	STATUS,Z		; test by subtracting 127
			bsf		OscDE,3			; if zero, PWM = 127 so change direction

			btfss	STATUS,C		; test if 128 
			goto	ClrOGCount		; if negative then PWM < 127, skip ahead
									; otherwise PWM=128
			bsf		OscDE,3			; set direction 
			movlw	d'126'			; we were at 127
			movwf	PWMG			; so lets goto 126
			goto	ClrOGCount		; clear counter and get out		

OscGrnDown	movlw	d'1'
			subwf	PWMG,f			; subtract 1 from PWM

			btfsc	STATUS,Z		; test
			bcf		OscDE,3			; z=1 means PWM is 0, change dir

			btfsc	STATUS,C		; test 
			goto 	ClrOGCount		; c=1 means PWM >= 0, skip ahead
									; otherwise we have -1
			bcf		OscDE,3			; change direction
			movlw	d'1'			; we were at 0
			movwf	PWMG			; lets move it to 1

ClrOGCount	clrf	OGCount

OscTestB	btfss	OscDE,4			; Test Blue osc enable
			goto 	OscClock		; if it's clear, skip

			btfss	OscTrigs,2		; test for our counter flag	
			goto 	OscClock		; if it's 0 skip ahead
									; otherwise:
			bcf		OscTrigs,2		; clear flag
			btfsc	OscDE,5			; test direction
			goto	OscBluDown		; 1 = decrement

OscBluUp	
			incf	PWMB,f
			movlw	d'127'	
			subwf	PWMB,w			; Is PWM 127 or greater?
			btfsc	STATUS,Z		; test by subtracting 127
			bsf		OscDE,5			; if zero, PWM = 127 so change direction

			btfss	STATUS,C		; test if 128 
			goto	ClrOBCount		; if negative then PWM < 127, skip ahead
									; otherwise PWM=128
			bsf		OscDE,5			; set direction 
			movlw	d'126'			; we were at 127
			movwf	PWMB			; so lets goto 126
			goto	ClrOBCount		; clear counter and get out		

OscBluDown	movlw	d'1'
			subwf	PWMB,f			; subtract 1 from PWM

			btfsc	STATUS,Z		; test
			bcf		OscDE,5			; z=1 means PWM is 0, change dir

			btfsc	STATUS,C		; test 
			goto 	ClrOBCount		; c=1 means PWM >= 0, skip ahead
									; otherwise we have -1
			bcf		OscDE,5			; change direction
			movlw	d'1'			; we were at 0
			movwf	PWMB			; lets move it to 1

ClrOBCount	clrf	OBCount

; This next routine is kind of funny.  It increments counters (OxCount) for each oscillator, tests them against the rate
; of oscillation (OscXXX) and if they match then a flag is tripped and the counter is reset.  The flag triggers
; the inc/dec code, so if it isn't tripped then we wait to actually affect PWMx.  
;
; Also confusing at first was the 6 reps of this routine, but this is just a speed control.  The counters are incremented
; at this rate to control the speed at which we modify PWMx.  
;
; Right now, we do these 6 cycles even if all the oscillators are not engaged.  This may be beneficial as it will keep
; the overall program loop the same length.  It may be prudent to test this by changing the code in OscTestB above to 
; skip past the OscClock routine.  

OscClock	movlw	d'6'
			movwf	OscCount0

OscClockL	incf	ORCount,f		; increment our counters for our osc colors
			incf	OGCount,f		; 
			incf	OBCount,f		; 

OscClockR	movf	OscRed,w
			xorwf	ORCount,w		; test OscX (speed) against OxCount
			btfss	STATUS,Z		; if they're not equal
			goto 	OscClockG		; skip ahead
			clrf	ORCount			; otherwise clear count
			bsf 	OscTrigs,0		; set flag

OscClockG	movf	OscGreen,w
			xorwf	OGCount,w		; test OscX (speed) against OxCount
			btfss	STATUS,Z		; if they're not equal
			goto 	OscClockB		; skip ahead
			clrf	OGCount			; otherwise clear count
			bsf 	OscTrigs,1		; set flag

OscClockB   movf	OscBlue,w
			xorwf	OBCount,w		; test OscX (speed) against OxCount
			btfss	STATUS,Z		; if they're not equal
			goto 	OscClock2		; skip ahead
			clrf	OBCount			; otherwise clear count
			bsf 	OscTrigs,2		; set flag

OscClock2	decfsz	OscCount0,f
			goto 	OscClockL		

EndOsc			