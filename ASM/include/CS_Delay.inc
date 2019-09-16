delay	movlw	d'250'			;delay 83.3 ms (12 MHz clock)
		movwf	count1
d1		movlw	0xC7			;delay .3mS
		movwf	counta
		movlw	0x01
		movwf	countb


Delay_0
		decfsz	counta, f
		goto	$+2
		decfsz	countb, f
		goto	Delay_0
		decfsz	count1	,f
		goto	d1
    	retlw	0x00