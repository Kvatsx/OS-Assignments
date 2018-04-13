bits 16
org 0x7c00							; Output at this location 0x7c00.

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

.print_string:							; Method to print hello world using loop
			lodsb
			or al, al
			jz stop
			or eax,0x0100
			mov word [ebx], ax
			add ebx,2
			jmp .print_string

stop:
	cli
	hlt

;bits 64

Message: db "Hello-World!", 0

times 510 - ($-$$) db 0
dw 0xaa55
