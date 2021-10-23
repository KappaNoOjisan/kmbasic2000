.module crt0

.globl _init
.globl _main
.globl _printBreak
.globl _checkBreak
.globl _allocateMemory
.globl __allocateMemory
.globl _freeMemory
.globl __freeMemory
.globl _strncmp
.globl __decI
.globl __hexI
.globl _mul
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
LD SP,#0x1420	     ;$1503
JP _main             ;$1506
JP _printBreak       ;$1509
JP _checkBreak       ;$150c
JP __allocateMemory  ;$150f
JP __freeMemory      ;$1512
JP _strncmp          ;$1515

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
 xor a		;HL=0 A=(DE)
 ld h,a
 ld l,a
 ld a,(de)	
 cp #0x030	;IF A <"0" RET
 ret c
 cp #0x03a 	;IF A>=":" RET
 jr nc,rderr
_dec0:		;DEC0 A=(DE)
 ld a,(de)
 cp #0x030
 ret c		;IF A<"0" RET
 cp #0x03a	;IF A>=":" RET
 jr nc,rderr
 call __mul10
 ld b,#0	
 sub #0x30
 ld c,a
 add hl,bc
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

;
; HL = HL div DE
;
__divu:
 ld b,h		;BC=HL HL=0
 ld c,l
 ld hl,#0
 ld a,#8
_divulop:	;DO A=8
 sla c 		; SLA(C) RL(B) HL=HL+.HL
 rl b
 adc hl,hl
 inc c 		; C+ HL=HL-.DE
 sbc hl,de
 jr nc,_divu2	; IF CY THEN HL=HL+DE C-
 add hl,de
 dec c
_divu2:
 sla c		; SLA(C) RL(B) HL=HL+.HL
 rl b
 adc hl,hl
 inc c		; C+ HL=HL-.DE
 sbc hl,de
 jr nc,_divu3	; IF CY THEN HL=HL+DE C-
 add hl,de
 dec c
_divu3:
 dec a
 jr nz,_divulop	;UNTIL DEC(A)=0 DE=BC DE<>HL
 ld d,b
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
