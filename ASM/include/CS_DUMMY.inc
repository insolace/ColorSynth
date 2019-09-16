	nop
	nop
	nop
	nop
	nop

	movlw d'128'
	movwf DummyCount
	
DoDummy
	nop
	nop

	nop
	nop
	nop
	nop
	nop
	nop
	nop

	nop
	nop
	nop
	nop
	nop
	nop
	nop

	nop
	nop
	nop
	nop
	nop
	nop
	nop

	decfsz DummyCount,f
	goto DoDummy

;*****************
