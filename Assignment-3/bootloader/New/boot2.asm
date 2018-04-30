bits 16 
org 0x7c00		; Output on this offset.

boot:
	mov ax, 0x2401
	int 0x15	; A20 bit enabled.

	mov ax, 0x3		; VGA TestMode 3.
	int 0x10

	cli		; Clear interrupt Flags (Disable Interrupts)

	lgdt [gdt_pointer]		; Loads GDT table.
	mov eax, cr0
	or eax,0x1
	mov cr0, eax
	jmp CODE_SEG:boot2		; jumps to 64 bit code segment.

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
gdt_pointer:
	dw gdt_end - gdt_start
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

bits 32
boot2:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	;mov esi, Message
	;mov ebx, 0xb8000

	; 64 bit, Setting LM-Bit
	mov exc, 0xc0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; Enabling Paging
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	; GDT64 Table
	lgdt [GDT64.Pointer]         ; Load the 64-bit global descriptor table.
    jmp GDT64.Code:Realm64       ; Set the code segment and enter 64-bit long mode.

GDT64:                           ; Global Descriptor Table (64-bit).
    .Null: equ $ - GDT64         ; The null descriptor.
    dw 0xFFFF                    ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 1                         ; Granularity.
    db 0                         ; Base (high).
    .Code: equ $ - GDT64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 10101111b                 ; Granularity, 64 bits flag, limit19:16.
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.

;.loop:
;	lodsb
;	or al,al
;	jz halt
;	or eax,0x0100
;	mov word [ebx], ax
;	add ebx,2
;	jmp .loop
;halt:
;	cli
;Message: db "Hello world!",0

bits 64
 
Realm64:
    cli                           ; Clear the interrupt flag.
    mov ax, GDT64.Data            ; Set the A-register to the data descriptor.
    mov ds, ax                    ; Set the data segment to the A-register.
    mov es, ax                    ; Set the extra segment to the A-register.
    mov fs, ax                    ; Set the F-segment to the A-register.
    mov gs, ax                    ; Set the G-segment to the A-register.
    mov ss, ax                    ; Set the stack segment to the A-register.
    ;mov edi, 0xB8000              ; Set the destination index to 0xB8000.
    ;mov rax, 0x1F201F201F201F20   ; Set the A-register to 0x1F201F201F201F20.
    ;mov ecx, 500                  ; Set the C-register to 500.
    ;rep stosq                     ; Clear the screen.
    ;hlt                           ; Halt the processor.
    mov esi, Message
	mov ebx, 0xb8000

.loop:
	lodsb
	or al,al
	jz halt
	or eax,0x0100
	mov word [ebx], ax
	add ebx,2
	jmp .loop
halt:
	cli
	hlt

Message: db "Hello world!",0

times 510 - ($-$$) db 0
dw 0xaa55
