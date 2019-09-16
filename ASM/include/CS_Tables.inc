;****************************************************************
; data tables 
;****************************************************************
			org 0x600
								
table128red	

			movwf 	TabWStore	; store our table address

								; ** remove last 23 **
			movlw   d'106'		; Subtract 106 
			subwf	TabWStore,W ; from our table address
			btfsc	STATUS,C	; if the result is positive	then
			retlw	d'127'		; (>=106) our table address would have been 127

								; **remove first 23**
			movlw	d'23'		; subtract 23
			subwf	TabWStore,f	; from our table address  
			btfss	STATUS,C	; if the result is negative	then
			retlw	d'127'		; (<23) our table address would have been 127

								; **ramp down?**
			movlw	d'20'		; subtract 20
			subwf	TabWStore,w ; from our current table address
			btfss	STATUS,C	; if the result is negative then
			goto	table128dwn ; (<43) this is a down ramp

								; **ramp up?**
			movlw	d'63'		; subtract 63
			subwf	TabWStore,f ; from our current table address
			btfsc	STATUS,C	; if the result is positive
			goto	table128up	; (=>86) this is an up ramp 

			retlw	d'0'		; otherwise our result should be 0


table128grn	movwf	TabWStore	; store table address

								; **Is it a zero?**
			movlw	d'44'		; subtract 44
			subwf	TabWStore,f	; from table address
			btfss	STATUS,C	; if the result is negative then
			retlw	d'0'		; (<44) our table value is 0

								; **Ramp up?**	
			movlw	d'20'		; subtract 20
			subwf	TabWStore,w	; from our table address
			btfss	STATUS,C	; if the result is negative
			goto	table128up	; (<=64) this is an up ramp

								; **Ramp down?**
			movlw	d'63'		; subtract (63)
			subwf	TabWStore,f	; from table address
			btfsc	STATUS,C	; if the result is positive
			goto	table128dwn	; (=<107) this is a down ramp

			retlw	d'127'		; otherwise the result should be 127

table128blu	movwf	TabWStore	; store table address

								; **Is it zero??!?!**
			movlw	d'85'		; subtract 85
			subwf	TabWStore,w	; from the table address
			btfsc	STATUS,C	; if the result is positive
			retlw	d'0'		; (>85) our table value is 0

								; **Ramp up?**
			movlw	d'22'		; subtract 22
			subwf	TabWStore,w	; from the table address
			btfss	STATUS,C	; if the result is negative
			goto	blu128up	; (<22) this is a ramp up
								; goto diff label as blu needs special handling

								; **Ramp down?**
			movlw	d'65'		; subtract 65
			subwf	TabWStore,f ; from table address
			btfsc	STATUS,C	; if the result is positive
			goto	table128dwn	; (>64) this is a down ramp

			retlw	d'127'		; otherwise the result should be 127

table128dwn	movf	TabWStore,w ; Collect modified table address
	
			brw
			nop
			;retlw d'127'		; 0 	 
			;retlw d'127'		; 1 	The first 23 (0-22) entries in the table 
			;retlw d'127'		; 2		are the same value, so we're saving
			;retlw d'127'		; 3		program space with the "remove first 23"
			;retlw d'127'		; 4		code above
			;retlw d'127'		; 5
			;retlw d'127'		; 6
			;retlw d'127'		; 7
			;retlw d'127'		; 8
			;retlw d'127'		; 9
			;retlw d'127'		; 10
			;retlw d'127'		; 11
			;retlw d'127'		; 12
			;retlw d'127'		; 13
			;retlw d'127'		; 14
			;retlw d'127'		; 15
			;retlw d'127'		; 16
			;retlw d'127'		; 17
			;retlw d'127'		; 18
			;retlw d'127'		; 19
			;retlw d'127'		; 20
			;retlw d'127'		; 21
			;retlw d'127'		; 22
			retlw d'121'		; 23   - 0
			retlw d'115'		; 24   - 1
			retlw d'109'		; 25   - 2
			retlw d'103'		; 26   - 3
			retlw d'97'			; 27   - 4
			retlw d'91'			; 28   - 5
			retlw d'85'			; 29   - 6
			retlw d'79'			; 30   - 7
			retlw d'73'		; 31   - 8
			retlw d'67'		; 32   - 9
			retlw d'60'		; 33   - 10
			retlw d'54'		; 34   - 11
			retlw d'48'		; 35   - 12
			retlw d'42'		; 36   - 13
			retlw d'36'		; 37   - 14
			retlw d'30'		; 38   - 15
			retlw d'24'		; 39   - 16
			retlw d'18'		; 40   - 17
			retlw d'12'		; 41   - 18
			retlw d'6'		; 42   - 19
			retlw d'0'		; 43   - 20 - Still necessary for green entry 17
			;retlw d'0'		; 44
			;retlw d'0'		; 45
			;retlw d'0'		; 46
			;retlw d'0'		; 47
			;retlw d'0'		; 48
			;retlw d'0'		; 49
			;retlw d'0'		; 50
			;retlw d'0'		; 51
			;retlw d'0'		; 52
			;retlw d'0'		; 53
			;retlw d'0'		; 54
			;retlw d'0'		; 55
			;retlw d'0'		; 56
			;retlw d'0'		; 57
			;retlw d'0'		; 58
			;retlw d'0'		; 59
			;retlw d'0'		; 60
			;retlw d'0'		; 61
			;retlw d'0'		; 62
			;retlw d'0'		; 63
			;retlw d'0'		; 64
			;retlw d'0'		; 65
			;retlw d'0'		; 66
			;retlw d'0'		; 67
			;retlw d'0'		; 68
			;retlw d'0'		; 69
			;retlw d'0'		; 70
			;retlw d'0'		; 71
			;retlw d'0'		; 72
			;retlw d'0'		; 73
			;retlw d'0'		; 74
			;retlw d'0'		; 75
			;retlw d'0'		; 76
			;retlw d'0'		; 77
			;retlw d'0'		; 78
			;retlw d'0'		; 79
			;retlw d'0'		; 80
			;retlw d'0'		; 81
			;retlw d'0'		; 82
			;retlw d'0'		; 83
			;retlw d'0'		; 84
			;retlw d'0'		; 85

								; A zero or one value for the blue index
								; throws things off, because the up ramp really
								; starts at index position 2.
								; so...
blu128up	movlw	d'2'		; subtract 2
			subwf 	TabWStore,f	; from our index value
			btfss	STATUS,C	; if we're now negative
			retlw	d'0'		; then return a 0, and all is well

table128up	movf	TabWStore,w ; Collect modified table address
			brw
			nop
			retlw d'6'		; 86   - 63
			retlw d'12'		; 87
			retlw d'18'		; 88
			retlw d'24'		; 89
			retlw d'30'		; 90
			retlw d'36'		; 91
			retlw d'42'		; 92
			retlw d'48'		; 93
			retlw d'54'		; 94
			retlw d'60'		; 95
			retlw d'67'		; 96
			retlw d'73'		; 97
			retlw d'79'		; 98
			retlw d'85'		; 99
			retlw d'91'		; 100
			retlw d'97'		; 101
			retlw d'103'		; 102 
			retlw d'109'		; 103
			retlw d'115'		; 104
			retlw d'121'		; 105
			;retlw d'127'		; 106
			;retlw d'127'		; 107
			;retlw d'127'		; 108
			;retlw d'127'		; 109
			;retlw d'127'		; 110
			;retlw d'127'		; 111
			;retlw d'127'		; 112
			;retlw d'127'		; 113
			;retlw d'127'		; 114
			;retlw d'127'		; 115
			;retlw d'127'		; 116
			;retlw d'127'		; 117
			;retlw d'127'		; 118
			;retlw d'127'		; 119
			;retlw d'127'		; 120
			;retlw d'127'		; 121
			;retlw d'127'		; 122
			;retlw d'127'		; 123
			;retlw d'127'		; 124
			;retlw d'127'		; 125
			;retlw d'127'		; 126
			;retlw d'127'		; 127
        	IF ((HIGH ($)) != (HIGH (table128red+1)))
         	   ERROR "string_table CROSSES PAGE BOUNDARY!"
        	ENDIF

; next table, for OSC timer
			org 0x700
TableOT
	brw
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'1'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'2'
	retlw 	d'3'
	retlw 	d'3'
	retlw 	d'3'
	retlw 	d'3'
	retlw 	d'3'
	retlw 	d'3'
	retlw 	d'4'
	retlw 	d'4'
	retlw 	d'4'
	retlw 	d'4'
	retlw 	d'5'
	retlw 	d'5'
	retlw 	d'6'
	retlw 	d'6'
	retlw 	d'7'
	retlw 	d'7'
	retlw 	d'8'
	retlw 	d'8'
	retlw 	d'9'
	retlw 	d'9'
	retlw 	d'10'
	retlw 	d'10'
	retlw 	d'11'
	retlw 	d'11'
	retlw 	d'12'
	retlw 	d'12'
	retlw 	d'13'
	retlw 	d'13'
	retlw 	d'14'
	retlw 	d'15'
	retlw 	d'16'
	retlw 	d'17'
	retlw 	d'18'
	retlw 	d'19'
	retlw 	d'20'
	retlw 	d'21'
	retlw 	d'22'
	retlw 	d'23'
	retlw 	d'24'
	retlw 	d'25'
	retlw 	d'26'
	retlw 	d'27'
	retlw 	d'28'
	retlw 	d'29'
	retlw 	d'30'
	retlw 	d'31'
	retlw 	d'32'
	retlw 	d'33'
	retlw 	d'34'
	retlw 	d'35'
	retlw 	d'37'
	retlw 	d'39'
	retlw 	d'41'
	retlw 	d'43'
	retlw 	d'45'
	retlw 	d'47'
	retlw 	d'49'
	retlw 	d'51'
	retlw 	d'53'
	retlw 	d'55'
	retlw 	d'57'
	retlw 	d'59'
	retlw 	d'61'
	retlw 	d'63'
	retlw 	d'65'
	retlw 	d'67'
	retlw 	d'69'
	retlw 	d'71'
	retlw 	d'73'
	retlw 	d'75'
	retlw 	d'77'
	retlw 	d'79'
	retlw 	d'81'
	retlw 	d'83'
	retlw 	d'85'
	retlw 	d'87'
	retlw 	d'89'
	retlw 	d'91'
	retlw 	d'93'
	retlw 	d'95'
	retlw 	d'97'
	retlw 	d'98'
	retlw 	d'99'
	retlw 	d'100'
	retlw 	d'101'
	retlw 	d'102'
	retlw 	d'103'
	retlw 	d'104'
	retlw 	d'105'
	retlw 	d'106'
	retlw 	d'107'
	retlw 	d'108'
	retlw 	d'109'
	retlw 	d'110'
	retlw 	d'111'
	retlw 	d'112'
	retlw 	d'113'
	retlw 	d'114'
	retlw 	d'115'
	retlw 	d'116'
	retlw 	d'117'
	retlw 	d'118'
	retlw 	d'119'
	retlw 	d'120'
	retlw 	d'121'
	retlw 	d'122'
	retlw 	d'123'
	retlw 	d'124'
	retlw 	d'125'
	retlw 	d'126'
	retlw 	d'127'
	retlw 	d'128'
