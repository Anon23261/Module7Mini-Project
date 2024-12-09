[BITS 16]                   ; Operate in 16-bit real mode
[ORG 0x7C00]                ; BIOS loads bootloader at this address

; Constants and Data Section
; -------------------------
KERNEL_OFFSET equ 0x1000    ; Kernel load address
KERNEL_SECTORS equ 32       ; Number of sectors to read
DEBUG_PORT equ 0xE9         ; QEMU debug port

; Data
boot_drive: db 0            ; Storage for boot drive number
welcome_msg: db 'GHOST OS Bootloader started...', 13, 10, 0
error_msg: db 'Error loading kernel!', 13, 10, 0
success_msg: db 'Kernel loaded successfully!', 13, 10, 0

; Entry Point
; -----------
start:
    ; Save boot drive number
    mov [boot_drive], dl    ; BIOS provides drive number in DL

    ; Initialize segment registers
    cli                     ; Disable interrupts during setup
    xor ax, ax             ; Clear AX (set to 0)
    mov ds, ax             ; Data Segment = 0
    mov es, ax             ; Extra Segment = 0
    mov ss, ax             ; Stack Segment = 0
    mov sp, 0x7C00         ; Set up stack just below bootloader
    sti                    ; Re-enable interrupts

    ; Print welcome message
    mov si, welcome_msg
    call print_string
    call print_newline

    ; Load kernel from disk
    ; --------------------
    mov ah, 0x02           ; BIOS read sector function
    mov al, KERNEL_SECTORS ; Number of sectors to read
    mov ch, 0              ; Cylinder 0
    mov cl, 2              ; Start from sector 2 (sector 1 is bootloader)
    mov dh, 0              ; Head 0
    mov dl, [boot_drive]   ; Drive number
    mov bx, KERNEL_OFFSET  ; Buffer address
    
    ; Debug: Print loading message
    push ax
    mov al, 'L'            ; 'L' for Loading
    out DEBUG_PORT, al     ; Send to QEMU debug port
    pop ax
    
    int 0x13              ; BIOS interrupt for disk operations
    jc disk_error         ; If carry flag set, there was an error

    ; Success: Print message and continue
    mov si, success_msg
    call print_string

    ; Enable A20 Line
    ; --------------
    call enable_a20
    jc a20_error          ; Check if A20 enable failed

    ; Switch to Protected Mode
    ; ----------------------
    cli                    ; Disable interrupts
    lgdt [gdt_descriptor]  ; Load GDT
    
    ; Set protected mode bit
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Jump to 32-bit code
    jmp CODE_SEG:init_pm

; Error Handlers
; -------------
disk_error:
    mov si, error_msg
    call print_string
    jmp $

a20_error:
    mov si, error_msg
    call print_string
    jmp $

; Utility Functions
; ----------------
print_string:              ; Print null-terminated string from SI
    pusha
    mov ah, 0x0E          ; BIOS teletype function
.loop:
    lodsb                 ; Load next character
    test al, al           ; Check for null terminator
    jz .done              ; If zero, we're done
    int 0x10              ; Print character
    jmp .loop
.done:
    popa
    ret

print_newline:
    pusha
    mov ah, 0x0E          ; BIOS teletype
    mov al, 13            ; Carriage return
    int 0x10
    mov al, 10            ; Line feed
    int 0x10
    popa
    ret

enable_a20:               ; Enable A20 line using BIOS
    mov ax, 0x2401       ; A20 enable function
    int 0x15             ; BIOS interrupt
    ret

; GDT (Global Descriptor Table)
; ---------------------------
gdt_start:
    dq 0                  ; Null descriptor

gdt_code:                 ; Code segment descriptor
    dw 0xFFFF            ; Limit (bits 0-15)
    dw 0                 ; Base (bits 0-15)
    db 0                 ; Base (bits 16-23)
    db 10011010b         ; Flags (8 bits)
    db 11001111b         ; Flags (4 bits) + limit (bits 16-19)
    db 0                 ; Base (bits 24-31)

gdt_data:                ; Data segment descriptor
    dw 0xFFFF            ; Limit (bits 0-15)
    dw 0                 ; Base (bits 0-15)
    db 0                 ; Base (bits 16-23)
    db 10010010b         ; Flags (8 bits)
    db 11001111b         ; Flags (4 bits) + limit (bits 16-19)
    db 0                 ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size (16 bits)
    dd gdt_start                ; GDT address (32 bits)

; Constants for Protected Mode
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

[BITS 32]
init_pm:                  ; Protected mode initialization
    ; Update segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Update stack
    mov ebp, 0x90000     ; New stack base
    mov esp, ebp

    ; Jump to kernel
    jmp KERNEL_OFFSET

; Padding and Boot Signature
; ------------------------
times 510-($-$$) db 0     ; Pad with zeros
dw 0xAA55                 ; Boot signature
