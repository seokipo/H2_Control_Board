	.file "d:\\Work\\H2_Control_Board\\02_Firmware\\rs422.c"
	.section	.text,code
	.align	2
	.global	_RS422_Initialize	; export
	.type	_RS422_Initialize,@function
_RS422_Initialize:
	.set ___PA___,1
	clr	w0
	.set ___PA___,0
	mov	#0x55,w1
	mov	w1,_NVMKEY
	mov	#0xAA,w1
	mov	w1,_NVMKEY
	mov	w0,_RPCON
	.set ___PA___,1
	mov	#-32513,w0
	and	_RPOR30bits
	mov	#2048,w0
	.set ___PA___,0
	mov	#0x55,w1
	mov	w1,_NVMKEY
	mov	#0xAA,w1
	mov	w1,_NVMKEY
	mov	w0,_RPCON
	.set ___PA___,1
	bset.b	_LATFbits+1,#5
	bclr.b	_TRISFbits+1,#5
	bset.b	_TRISEbits+1,#2
	return	
	.set ___PA___,0
	.size	_RS422_Initialize, .-_RS422_Initialize
	.align	2
	.global	_RS422_WriteByte	; export
	.type	_RS422_WriteByte,@function
_RS422_WriteByte:
	.set ___PA___,1
	mov	w8,[w15++]
	mov.b	w0,w8
	bclr.b	_LATFbits+1,#5
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	and	w8,#1,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	ze	w8,w8
	bfext	#1,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bfext	#2,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bfext	#3,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bfext	#4,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bfext	#5,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bfext	#6,#1,w8,w0
	sl	w0,#13,w0
	mov	_LATFbits,w1
	bclr	w1,#13
	ior	w0,w1,w1
	mov	w1,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	lsr	w8,#7,w8
	sl	w8,#13,w8
	mov	_LATFbits,w0
	bclr	w0,#13
	ior	w8,w0,w0
	mov	w0,_LATFbits
	mov	#200,w0
	mov	#0,w1
	rcall	___delay32
	bset.b	_LATFbits+1,#5
	mov	#208,w0
	mov	#0,w1
	rcall	___delay32
	mov	[--w15],w8
	return	
	.set ___PA___,0
	.size	_RS422_WriteByte, .-_RS422_WriteByte
	.align	2
	.global	_RS422_WriteBuffer	; export
	.type	_RS422_WriteBuffer,@function
_RS422_WriteBuffer:
	.set ___PA___,1
	mov.d	w8,[w15++]
	mov	w10,[w15++]
	mov	w1,w10
	cp0	w0
	.set ___BP___,10
	bra	z,.L3
	cp0	w10
	.set ___BP___,39
	bra	z,.L3
	mov	w0,w9
	clr	w8
.L5:
	mov.b	[w9++],w0
	rcall	_RS422_WriteByte
	inc	w8,w8
	sub	w10,w8,[w15]
	.set ___BP___,91
	bra	gtu,.L5
.L3:
	mov	[--w15],w10
	mov.d	[--w15],w8
	return	
	.set ___PA___,0
	.size	_RS422_WriteBuffer, .-_RS422_WriteBuffer
	.align	2
	.global	_RS422_IsRxReady	; export
	.type	_RS422_IsRxReady,@function
_RS422_IsRxReady:
	.set ___PA___,1
	mov	#1024,w0
	and	_PORTEbits,WREG
	dec	w0,w0
	lsr	w0,#15,w0
	return	
	.set ___PA___,0
	.size	_RS422_IsRxReady, .-_RS422_IsRxReady
	.align	2
	.global	_RS422_ReadByte	; export
	.type	_RS422_ReadByte,@function
_RS422_ReadByte:
	.set ___PA___,1
	mov.d	w8,[w15++]
	clr.b	w8
	btst.b	_PORTEbits+10/8,#10%8
	.set ___BP___,61
	bra	nz,.L9
	mov	#90,w0
	mov	#0,w1
	rcall	___delay32
	btst.b	_PORTEbits+10/8,#10%8
	.set ___BP___,61
	bra	nz,.L9
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	mov	#1024,w9
	mov	_PORTEbits,w0
	and	w9,w0,w8
	neg	w8,w8
	lsr	w8,#15,w8
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	mov	_PORTEbits,w0
	and	w9,w0,w9
	.set ___BP___,50
	bra	z,.L10
	bset.b	w8,#1
.L10:
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#2
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#3
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#4
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#5
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#6
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
	btsc	_PORTEbits,#10
	bset.b	w8,#7
	mov	#195,w0
	mov	#0,w1
	rcall	___delay32
.L9:
	mov.b	w8,w0
	mov.d	[--w15],w8
	return	
	.set ___PA___,0
	.size	_RS422_ReadByte, .-_RS422_ReadByte



	.section __c30_info, info, bss
__large_data_scalar:

	.section __c30_signature, info, data
	.word 0x0001
	.word 0x0007
	.word 0x0000

; MCHP configuration words

	.set ___PA___,0
	.end
