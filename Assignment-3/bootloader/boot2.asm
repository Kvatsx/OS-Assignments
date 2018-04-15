bits 16
org 0x7c00							; Output at this location 0x7c00.

HelloWorld: db "Hello-World!   ", 0

main:
	mov ax, 0x2401
	int 0x15						; Enables A20 bit
	mov ax, 0x3
	int 0x10 						; Set VGA Mode
	cli

	lgdt [pointer]					; Loads GDT Table
	mov eax, cr0
	or eax,0x1 						; protected mode bit setting.
	mov cr0, eax
	jmp CODE_SEG:main2				; Jumps to next code segment.

gdt_start:
	dq 0x0
gdt_code:
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0
gdt_data:
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0
gdt_end:
pointer:
	dw gdt_end - gdt_start
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

Message: db "Hello-World!   0x14DC090000", 0
cr: db "0x14DC090000", 0


bits 32

main2:
	mov ax, DATA_SEG 					; Initialize all segments.
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esi, Message
	mov ebx, 0xb8000
	call string
	mov esi, cr3
	mov ebx, 0xb8000
	call string

	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

string:		
		lodsb
		or al, al
		jz stop
		or eax,0x0100
		mov word [ebx], ax
		add ebx,2
		jmp string

stop:
	cli
	hlt
	call printNL

printNL:
		mov al, 0
		stosb
		mov ah, 0x0E
		mov al, 0x0D
		int 0x10
		mov al, 0x0A
		int 0x10
		ret


bits 64

boot64:
		cli
		mov ax, gdt_data
		mov gs, ax
		mov ss, ax
		mov fs, ax
		mov ds, ax
		mov es, ax
		mov rax, 0x1F201F201F201F20
		mov edi, 0xB8000
		mov ecx, 500
		rep stosq
		mov ebx, HelloWorld
		call print_func
;		mov ebx, cr3
		call print_func
		hlt

print_func:						
		lodsb
		or al, al
		jz clear
		or eax,0x0100
		mov word [ebx], ax
		add ebx,2
		jmp print_func
clear:
	cli
	hlt

times 510 - ($-$$) db 0
dw 0xaa55
