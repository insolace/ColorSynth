	btfss	SlideFlag,0			; test slide flag
	goto 	SkipSlide			; if clear then skip ahead
	bcf		SlideFlag,0			; otherwise clear slide flag and continue

	movlw 	d'0'				; 
	xorwf	RateR,w				; 
	btfss	STATUS,Z			; if our rate is anything other than 0	
	call 	SlidingRed			; then call sub, otherwise move on

	movlw 	d'0'				; 
	xorwf	RateG,w				; 
	btfss	STATUS,Z			; if our rate is anything other than 0	
	call 	SlidingGrn			; then call sub, otherwise move on

	movlw 	d'0'				;
	xorwf	RateB,w				; 
	btfss	STATUS,Z			; if our rate is anything other than 0	
	call 	SlidingBlu			; then call sub, otherwise move on

	goto	SkipSlide

;*****************************************************
; subroutine to handle "sliding" function for red bank

; RateR is the waiting period at which we increment or decrement PWMR to match TargetR

; First we need to test if we're at our target, if we are then we set RateR to 0 and leavesub
; Then we need to decrement our counter
; If we hit 0 then  
; based on if we're below or above our target 
; we increment or decrement PWMR
; then we reset the counter and leavesub

SlidingRed	movf	PWMR,w
			xorwf	TargetR,w		; test if PWMR=TargetR
			btfss	STATUS,Z		; if they're different
			goto	SlidingRed2		; then continue
			clrf	RateR			; otherwise clear RateR and CounterR and leave
			clrf	CounterR		
			return
SlidingRed2
			movlw	d'0'			; 
			xorwf	CounterR,w		; test Counter
			btfsc	STATUS,Z		; if Counter is 0 
			goto 	SlidingRed3		; skip ahead, modify stuff, then reset counter
			decf	CounterR,f		; otherwise dec the counter
			return					; and leavesub

; convert rate to counter 0-7
SlidingRed3 btfss	RateR,4			; if bit 4 is clear
			bsf		CounterR,0		; set bit 0 in Counter
			btfss	RateR,5			; if bit 5 is clear
			bsf		CounterR,1		; set bit 1 in Counter
			btfss	RateR,6			; if bit 6 is clear
			bsf		CounterR,2		; set bit 2 in Counter

			movlw	d'0'
			xorwf	CounterR,w		; test new Counter
			btfss   STATUS,Z		; if it's anything other than 0
			goto	SlidingRed4		; skip ahead
			movf	TargetR,w		; otherwise
			movwf	PWMR			; jump to our target		
			clrf	RateR			; clear our Rate
			return					; and leave

; test bits, determine how much we're going to inc/dec
SlidingRed4	btfsc	CounterR,2		; test bit 2, 0 is 0-3, 
			goto	SlidingRC47		; 1 is 4-7, skip ahead
; counter is 0-3
			btfss	CounterR,0		; test bit 0 (we know it's not 00)
			goto	SlidingRC2		; it's 10 (2)
			btfss	CounterR,1		; must be 11 (3) or 01 (1)
			goto	SlidingRC1		; it's 01 (1)
SlidingRC3	movlw	d'8'			; it's 11 (3)
			goto	SlidingRCD
SlidingRC2	movlw	d'16'			; 
			goto	SlidingRCD
SlidingRC1	movlw	d'32'			; 
			goto	SlidingRCD

; counter is 4-7
SlidingRC47	btfss	CounterR,0		; test bit 0 
			goto	SlidingRC46		; must be 100 (4) or 110 (6)
			btfss	CounterR,1		; or it's 101 (5) or 111 (7)
			goto	SlidingRC5		; it's 101 (5)
SlidingRC7	movlw	d'1'			; it's 111 (7)		
			goto 	SlidingRCD
SlidingRC46	btfss	CounterR,1		; 
			goto	SlidingRC4		; it's 100 (4)
SlidingRC6	movlw	d'2'			; it's 110 (6)
			goto	SlidingRCD
SlidingRC5	movlw	d'3'			; 
			goto	SlidingRCD
SlidingRC4	movlw	d'4'			; 
			goto	SlidingRCD

SlidingRCD  movwf	RateCut			; store the amount we're cutting/adding
; determine if we're above or below and cut/dec accordingly
			movf	TargetR,w
			subwf	PWMR,w			; subtract TargetR from PWMR
			movwf	RateDiff		; Store the difference between
			btfss	STATUS,C		; C=0 means result negative
			goto 	SlidingRIX		; so we need to increment PWMR to reach or target
									; otherwise we need to decrement
SlidingRDX	movf	RateCut,w		; grab RateCut
			subwf	RateDiff,f		; sub from RateDiff
			btfss	STATUS,C		; test result
			goto	SlidingRD1		; c=0 means neg, only dec 1
			subwf	PWMR,f			; c=1 means pos, sub W
			return					; and leave
SlidingRD1	decf	PWMR,f			; dec once
			return					; and leave

; inc (negative difference)
SlidingRIX	movf	RateCut,w		; grab RateCut
			addwf	RateDiff,f		; add it to RateDiff
			btfsc	STATUS,C		; test result
			goto	SlidingRI1		; c=1 means we passed 255, only inc 1
			addwf	PWMR,f			; c=0 means we're under 255, add W
			return					; and leave
SlidingRI1	incf	PWMR,f			; dec once
			return					; and leave

				
;*****************************************************
									; subroutine to handle "sliding" function for Green bank
SlidingGrn	movf	PWMG,w
			xorwf	TargetG,w		; test if PWM=Target
			btfss	STATUS,Z		; if they're different
			goto	SlidingGrn2		; then continue
			clrf	RateG			; otherwise clear RateR and CounterR and leave
			clrf	CounterG		
			return
SlidingGrn2
			movlw	d'0'			; 
			xorwf	CounterG,w		; test Counter
			btfsc	STATUS,Z		; if Counter is 0 
			goto 	SlidingGrn3		; skip ahead, modify stuff, then reset counter
			decf	CounterG,f		; otherwise dec the counter
			return					; and leavesub

; convert rate to counter 0-7
SlidingGrn3 btfss	RateG,4			; if bit 4 is clear
			bsf		CounterG,0		; set bit 0 in Counter
			btfss	RateG,5			; if bit 5 is clear
			bsf		CounterG,1		; set bit 1 in Counter
			btfss	RateG,6			; if bit 6 is clear
			bsf		CounterG,2		; set bit 2 in Counter

			movlw	d'0'
			xorwf	CounterG,w		; test new Counter
			btfss   STATUS,Z		; if it's anything other than 0
			goto	SlidingGrn4		; skip ahead
			movf	TargetG,w		; otherwise
			movwf	PWMG			; jump to our target		
			clrf	RateG			; clear our Rate
			return					; and leave

; test bits, determine how much we're going to inc/dec
SlidingGrn4	btfsc	CounterG,2		; test bit 2, 0 is 0-3, 
			goto	SlidingGC47		; 1 is 4-7, skip ahead
; counter is 0-3
			btfss	CounterG,0		; test bit 0 (we know it's not 00)
			goto	SlidingGC2		; it's 10 (2)
			btfss	CounterG,1		; must be 11 (3) or 01 (1)
			goto	SlidingGC1		; it's 01 (1)
SlidingGC3	movlw	d'8'			; it's 11 (3)
			goto	SlidingGCD
SlidingGC2	movlw	d'16'			; 
			goto	SlidingGCD
SlidingGC1	movlw	d'32'			; 
			goto	SlidingGCD

; counter is 4-7
SlidingGC47	btfss	CounterG,0		; test bit 0 
			goto	SlidingGC46		; must be 100 (4) or 110 (6)
			btfss	CounterG,1		; or it's 101 (5) or 111 (7)
			goto	SlidingGC5		; it's 101 (5)
SlidingGC7	movlw	d'1'			; it's 111 (7)		
			goto 	SlidingGCD
SlidingGC46	btfss	CounterG,1		; 
			goto	SlidingGC4		; it's 100 (4)
SlidingGC6	movlw	d'2'			; it's 110 (6)
			goto	SlidingGCD
SlidingGC5	movlw	d'3'			; 
			goto	SlidingGCD
SlidingGC4	movlw	d'4'			; 
			goto	SlidingGCD

SlidingGCD  movwf	RateCut			; store the amount we're cutting/adding
; determine if we're above or below and cut/dec accordingly
			movf	TargetG,w
			subwf	PWMG,w			; subtract TargetR from PWMR
			movwf	RateDiff		; Store the difference between
			btfss	STATUS,C		; C=0 means result negative
			goto 	SlidingGIX		; so we need to increment PWMR to reach or target
									; otherwise we need to decrement
SlidingGDX	movf	RateCut,w		; grab RateCut
			subwf	RateDiff,f		; sub from RateDiff
			btfss	STATUS,C		; test result
			goto	SlidingGD1		; c=0 means neg, only dec 1
			subwf	PWMG,f			; c=1 means pos, sub W
			return					; and leave
SlidingGD1	decf	PWMG,f			; dec once
			return					; and leave

; inc (negative difference)
SlidingGIX	movf	RateCut,w		; grab RateCut
			addwf	RateDiff,f		; add it to RateDiff
			btfsc	STATUS,C		; test result
			goto	SlidingGI1		; c=1 means we passed 255, only inc 1
			addwf	PWMG,f			; c=0 means we're under 255, add W
			return					; and leave
SlidingGI1	incf	PWMG,f			; dec once
			return					; and leave

;*****************************************************
									; subroutine to handle "sliding" function for Blue bank
SlidingBlu	movf	PWMB,w
			xorwf	TargetB,w		; test if PWM=Target
			btfss	STATUS,Z		; if they're different
			goto	SlidingBlu2		; then continue
			clrf	RateB			; otherwise clear RateR and CounterR and leave
			clrf	CounterB		
			return
SlidingBlu2
			movlw	d'0'			; 
			xorwf	CounterB,w		; test Counter
			btfsc	STATUS,Z		; if Counter is 0 
			goto 	SlidingBlu3		; skip ahead, modify stuff, then reset counter
			decf	CounterB,f		; otherwise dec the counter
			return					; and leavesub

; convert rate to counter 0-7
SlidingBlu3 btfss	RateB,4			; if bit 4 is clear
			bsf		CounterB,0		; set bit 0 in Counter
			btfss	RateB,5			; if bit 5 is clear
			bsf		CounterB,1		; set bit 1 in Counter
			btfss	RateB,6			; if bit 6 is clear
			bsf		CounterB,2		; set bit 2 in Counter

			movlw	d'0'
			xorwf	CounterB,w		; test new Counter
			btfss   STATUS,Z		; if it's anything other than 0
			goto	SlidingBlu4		; skip ahead
			movf	TargetB,w		; otherwise
			movwf	PWMB			; jump to our target		
			clrf	RateB			; clear our Rate
			return					; and leave

; test bits, determine how much we're going to inc/dec
SlidingBlu4	btfsc	CounterB,2		; test bit 2, 0 is 0-3, 
			goto	SlidingBC47		; 1 is 4-7, skip ahead
; counter is 0-3
			btfss	CounterB,0		; test bit 0 (we know it's not 00)
			goto	SlidingBC2		; it's 10 (2)
			btfss	CounterB,1		; must be 11 (3) or 01 (1)
			goto	SlidingBC1		; it's 01 (1)
SlidingBC3	movlw	d'8'			; it's 11 (3)
			goto	SlidingBCD
SlidingBC2	movlw	d'16'			; 
			goto	SlidingBCD
SlidingBC1	movlw	d'32'			; 
			goto	SlidingBCD

; counter is 4-7
SlidingBC47	btfss	CounterB,0		; test bit 0 
			goto	SlidingBC46		; must be 100 (4) or 110 (6)
			btfss	CounterB,1		; or it's 101 (5) or 111 (7)
			goto	SlidingBC5		; it's 101 (5)
SlidingBC7	movlw	d'1'			; it's 111 (7)		
			goto 	SlidingBCD
SlidingBC46	btfss	CounterB,1		; 
			goto	SlidingBC4		; it's 100 (4)
SlidingBC6	movlw	d'2'			; it's 110 (6)
			goto	SlidingBCD
SlidingBC5	movlw	d'3'			; 
			goto	SlidingBCD
SlidingBC4	movlw	d'4'			; 
			goto	SlidingBCD

SlidingBCD  movwf	RateCut			; store the amount we're cutting/adding
; determine if we're above or below and cut/dec accordingly
			movf	TargetB,w
			subwf	PWMB,w			; subtract TargetR from PWMR
			movwf	RateDiff		; Store the difference between
			btfss	STATUS,C		; C=0 means result negative
			goto 	SlidingBIX		; so we need to increment PWMR to reach or target
									; otherwise we need to decrement
SlidingBDX	movf	RateCut,w		; grab RateCut
			subwf	RateDiff,f		; sub from RateDiff
			btfss	STATUS,C		; test result
			goto	SlidingBD1		; c=0 means neg, only dec 1
			subwf	PWMB,f			; c=1 means pos, sub W
			return					; and leave
SlidingBD1	decf	PWMB,f			; dec once
			return					; and leave

; inc (negative difference)
SlidingBIX	movf	RateCut,w		; grab RateCut
			addwf	RateDiff,f		; add it to RateDiff
			btfsc	STATUS,C		; test result
			goto	SlidingBI1		; c=1 means we passed 255, only inc 1
			addwf	PWMB,f			; c=0 means we're under 255, add W
			return					; and leave
SlidingBI1	incf	PWMB,f			; dec once
			return					; and leave



SkipSlide