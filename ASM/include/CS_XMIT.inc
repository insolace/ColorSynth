;****************************************************************
; Subroutine to handle transmission of MIDI data
;
; This subroutine handles all MIDI data except SysEx, which can be any lenght of data/bytes.  Any subroutine 
; handling transmission of SysEx should set bit 0 of TXState high, which will defeat transmission from this 
; subroutine until it is cleared.
;
; MIDI data can be transmitted in 3, 2, or single byte packets.  To conform to the entire spec, we have to
; be able to transmit any of these packets, regardless of the size.  
;
; To transmit a single or two byte packet:
; -Test TXState, if it's set then discard the packet and move on
; -Test (PIR1,TXIF), if it's clear (TXREG is full) then discard the packet and move on
; -Test (TXSTA,TRMT), if it's clear (USART is transmitting) then discard the packet and move on
; -Write both bytes to TXREG in order.  The first will transmit immediately, the second is buffered in TXREG.
;
; How to setup a 3 byte packet for transmission:
; -Test (TXState,0), if it's high then discard the entire packet, otherwise continue
; -Test (PIR1,TXIF), if it's clear (TXREG is full) then discard the packet and move on
; -Test (TXSTA,TRMT), if it's clear (USART is transmitting) then discard the packet and move on
; -Set (TXState,0) high
; -Write the first two bytes to TXREG in order.
; -Write the third to TXBuff
; -Set (PIE1,TXIE) high to enable TX interrupt
;
; When TXREG is empty TXIF will flag high and trigger the interrupt routine, the interrupt will:
; -write TXBuff to TXREG
; -clear TXState and (PIE1,TXIE)
;
;****************************************************************

TxMidiCC
			btfsc	ByteCount,0		; are we in the middle of RX and consequently echo RX?
			RETURN					; yes, move along
			btfsc	TXState,0		; are we currently transmitting?
			RETURN					; yes, move along
			btfss	PIR1,TXIF		; is TXREG full?
			RETURN					; yes, move along
	banksel	TXSTA	
			btfsc	TXSTA,TRMT		; Is a byte still being transmitted?
			goto	XmitClear		; no, lets transmit!
									; otherwise
	movlb	d'0' 			; bank0
			RETURN					; leave

XmitClear	; still in correct bank for TXREG
			bsf		TXEchoWait,0	; Flag to wait for status byte when echoing MIDI RX
			movlw	d'191'			; CH16 CC
			movwf	TXREG			; transmited immediately
	movlb	d'0'			; bank0
			movf	CCNumXmit,w		; collect CC Number
	banksel	TXREG
			movwf	TXREG			; buffered in TXREG

	movlb	d'0' 			; bank0
			movf	CCValXmit,w		; collect CC value
			movwf	TXBuff			; buffer it for interrupt to transmit later
			bsf		TXState,0		; We are transmitting now, don't interrupt us
	banksel PIE1
			bsf		PIE1,TXIE		; enable TX interrupt
	movlb	d'0' 			; bank0

			RETURN

;****************************************************************