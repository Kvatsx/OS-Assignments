bits	16
 
; Remember the memory map-- 0x500 through 0x7bff is unused above the BIOS data area.
; We are loaded at 0x500 (0x50:0)
 
org 0x500
 
jmp	main				; go to start
 
;*******************************************************
;	Preprocessor directives
;*******************************************************
 
;include "stdio.inc"			; basic i/o routines
;%include "Gdt.inc"			; Gdt routines
 
;*******************************************************
;	Data Section
;*******************************************************
 
LoadingMsg db "Preparing to load operating system...", 0x0D, 0x0A, 0x00
 
;*******************************************************
;	STAGE 2 ENTRY POINT
;
;		-Store BIOS information
;		-Load Kernel
;		-Install GDT; go into protected mode (pmode)
;		-Jump to Stage 3
;*******************************************************
 
main:
 
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
 
	cli				; clear interrupts
	xor	ax, ax			; null segments
	mov	ds, ax
	mov	es, ax
	mov	ax, 0x9000		; stack begins at 0x9000-0xffff
	mov	ss, ax
	mov	sp, 0xFFFF
	sti				; enable interrupts
 
	;-------------------------------;
	;   Print loading message	;
	;-------------------------------;
 
	mov	si, LoadingMsg
	call	Puts16
 
	;-------------------------------;
	;   Install our GDT		;
	;-------------------------------;
 
	call	InstallGDT		; install our GDT
 
	;-------------------------------;
	;   Go into pmode		;
	;-------------------------------;
 
	cli				; clear interrupts
	mov	eax, cr0		; set bit 0 in cr0--enter pmode
	or	eax, 1
	mov	cr0, eax
 
	jmp	08h:Stage3		; far jump to fix CS. Remember that the code selector is 0x8!
 
	; Note: Do NOT re-enable interrupts! Doing so will triple fault!
	; We will fix this in Stage 3.
 
Puts16:
		pusha				; save registers
.Loop1:
		lodsb				; load next byte from string from SI to AL
		or	al, al			; Does AL=0?
		jz	Puts16Done		; Yep, null terminator found-bail out
		mov	ah, 0eh			; Nope-Print the character
		int	10h			; invoke BIOS
		jmp	.Loop1			; Repeat until null terminator found
Puts16Done:
		popa				; restore registers
		ret				; we are done, so return
 
;*******************************************
; InstallGDT()
;	- Install our GDT
;*******************************************
 
InstallGDT:
 
	cli				; clear interrupts
	pusha				; save registers
	lgdt 	[toc]			; load GDT into GDTR
	sti				; enable interrupts
	popa				; restore registers
	ret				; All done!
 
;*******************************************
; Global Descriptor Table (GDT)
;*******************************************
 
gdt_data: 
	dd 0 				; null descriptor
	dd 0 
 
; gdt code:				; code descriptor
	dw 0FFFFh 			; limit low
	dw 0 				; base low
	db 0 				; base middle
	db 10011010b 			; access
	db 11001111b 			; granularity
	db 0 				; base high
 
; gdt data:				; data descriptor
	dw 0FFFFh 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 10010010b 			; access
	db 11001111b 			; granularity
	db 0				; base high
 
end_of_gdt:
toc: 
	dw end_of_gdt - gdt_data - 1 	; limit (Size of GDT)
	dd gdt_data 			; base of GDT
	

;******************************************************
;	ENTRY POINT FOR STAGE 3
;******************************************************

bits 32					; Welcome to the 32 bit world!
 
Stage3:
 
	;-------------------------------;
	;   Set registers		;
	;-------------------------------;
 
	mov		ax, 0x10		; set data segments to data selector (0x10)
	mov		ds, ax
	mov		ss, ax
	mov		es, ax
	mov		esp, 90000h		; stack begins from 90000h
 
;*******************************************************
;	Stop execution
;*******************************************************
 
STOP:
 
	cli
	hlt
