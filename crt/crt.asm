.module crt0

.globl _init
.globl _main
.globl _printBreak
.globl _checkBreak
.globl _allocateMemory
.globl __allocateMemory
.globl _freeMemory
.globl __freeMemory
.globl __decI
.globl __hexI
.globl _mul
.globl _z80div
.globl __divu
.globl __stade
.globl _hlsta
.globl _g_strBuff
.globl __putStr

.globl l__INITIALIZER
.globl s__INITIALIZER
.globl s__INITIALIZED

;; Ordering of segments for the linker.
.area _HOME
.area _CODE
.area _INITIALIZER
.area _GSINIT
.area _GSFINAL

.area _DATA
.area _INITIALIZED
.area _BSEG
.area _BSS
.area _HEAP

.area _CODE

CALL _initProc       ;$1500
LD SP,#(__kmstack)   ;$1503
JP _main             ;$1506
JP _printBreak       ;$1509
JP _checkBreak       ;$150c
JP __allocateMemory  ;$150f
JP __freeMemory      ;$1512

_initProc:
 call gsinit
 call _init
 ret

_printBreak:
 rst 0x28
 .db 4
_checkBreak:
 rst 0x28
 .db 0x0d
 ret nz
 jp _main

__allocateMemory::
; Allocate memory with BC bytes
; Return the address of allocated memory as DE
PUSH BC
CALL _allocateMemory
LD D,H
LD E,L
POP BC
RET

__freeMemory::
; Free the last allocated memory starting with HL
PUSH HL
CALL _freeMemory
POP HL
RET

;
; puts (HL)
;
__putStr::
 ex de,hl
 rst 0x28
 .db 0x04
 jr _checkBreak

;
; HL=Hex(DE)
;
__hexI::
 xor a
 ld h,a
 ld l,a
hexilop:
 ld a,(de)
 call rdhex
 ret c
 add hl,hl	;*2
 add hl,hl	;*4
 add hl,hl	;*8
 add hl,hl	;*16
 ret c
 or l
 ld l,a
 inc de
 jr hexilop

rdhex:
 cp #'0'
 ret c
 cp #':'
 jr c,hex1
 cp #'A'
 ret c
 cp #'G'
 jr nc,rderr
 sub #0x37
 ret
rderr:
 scf
 ret
hex1:
 sub #0x30
 ret

;
; hl = decimal(DE)
;
__decI::
 ld bc,#0x303A
 xor a		;HL=0 A=(DE)
 ld h,a
 ld l,a
 ld a,(de)	
 cp b		;IF A <"0" RET
 ret c
 cp c 		;IF A>=":" RET
 jr nc,rderr
_dec0:		;DEC0 A=(DE)
 ld a,(de)
 cp b
 ret c		;IF A<"0" RET
 cp c		;IF A>=":" RET
 jr nc,rderr
 call __mul10
 push bc
 sub b
 ld b,#0	
 ld c,a
 add hl,bc
 pop bc
 inc de	
 jr _dec0

__mul10:
 push de
 ld d,h
 ld e,l
 add hl,hl	;*2
 add hl,hl	;*4
 add hl,de	;*5
 add hl,hl	;*10
 pop de
 ret

_com:
 ld a,h
 cpl
 ld h,a
 ld a,l
 cpl
 ld l,a
 inc hl
 ret
 
_z80div::
 ld b,#0
 ld a,d
 or a
 jr nz,_div0
 or e
 jr nz,_div1
 ret

_div0:
 jp p,_div1
 inc b
 ex de,hl
 call _com
 ex de,hl
_div1:
 ld a,h
 cp a,#0x80
 jr c,_div2
 dec b
 call _com
_div2:
 ld a,b
 or a,a
 jr z,__divu
 call __divu
 jr _com
;
; HL = HL div DE
;
__divu:
 ld a,h		;AC=HL HL=0
 ld c,l
 ld hl,#0
 ld b,#8
_divulop:	;DO A=8
 sla c 		; SLA(C) RL(A) HL=HL+.HL
 rla
 adc hl,hl
 inc c 		; C+ HL=HL-.DE
 sbc hl,de
 jr nc,_divu2	; IF CY THEN HL=HL+DE C-
 add hl,de
 dec c
_divu2:
 sla c		; SLA(C) RL(A) HL=HL+.HL
 rla
 adc hl,hl
 inc c		; C+ HL=HL-.DE
 sbc hl,de
 jr nc,_divu3	; IF CY THEN HL=HL+DE C-
 add hl,de
 dec c
_divu3:
 djnz _divulop	;UNTIL DEC(B)=0 DE=AC DE<>HL
 ld d,a
 ld e,c
 ex de,hl
 ret

;
; DE = DE div 10
;
__div10:
 ex de,hl
 ld de,#10
 call __divu
 ex de,hl
 ret

;
; HL = HL * DE
;
_mul::
 ld a,d
 or a
 push hl
 ld hl,#0
 ld b,#16
 jr nz,_mul0
 srl b
 ld a,e
_mul0:
 ld c,e
 dec b
 pop de
_mul1:
 sla c
 rla
 jr nc,_mul2
 add hl,de
_mul2:
 add hl,hl
 djnz _mul1
 or a
 ret p
 add hl,de
 ret

;
; PRINT DECIMAL
;  HL
;
_hlsta:	;HLSTA [DE DE<>HL !STADE ]DE RET
 push de
 ex de,hl
 call __stade
 pop de
 ret

;
; PRINT DECIMAL
;  DE : decimal
;  HL : g_strBuff +5
__stade::
 ld hl,#(_g_strBuff + 0x0005)
 ld (hl),#0x0d
_stade2:
 push hl
 call __div10
 ld a,l
 add a,#'0'
 pop hl
 dec hl
 ld (hl),a
 ld a,d	
 or e
 jr nz,_stade2
 ret

;
; stack area
;
	.ds 200
__kmstack::


	.area   _GSINIT
gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:

	.area   _GSFINAL
	ret
