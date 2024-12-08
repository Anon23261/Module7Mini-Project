; Simple bootloader - Stage 1
[org 0x7c00]
[bits 16]

; Initialize segments
cli                     ; Clear interrupts
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00
sti                     ; Enable interrupts

; Print welcome message
mov si, welcome_msg
call print_string

; Load Stage 2
mov ah, 0x02           ; BIOS read sector function
mov al, 1              ; Number of sectors to read
mov ch, 0              ; Cylinder number
mov cl, 2              ; Sector number (1-based, sector 2)
mov dh, 0              ; Head number
mov dl, [boot_drive]   ; Drive number
mov bx, 0x7e00        ; Load stage 2 right after stage 1

int 0x13               ; BIOS interrupt for disk operations
jc disk_error          ; If carry flag set, there was an error

; Jump to Stage 2
jmp 0x7e00

disk_error:
    mov si, error_msg
    call print_string
    jmp $

; Print string function
print_string:
    pusha
    mov ah, 0x0e       ; BIOS teletype output
.loop:
    lodsb              ; Load next character
    test al, al        ; Check if end of string (0)
    jz .done
    int 0x10           ; Print character
    jmp .loop
.done:
    popa
    ret

welcome_msg db 'GHOST OS Bootloader Stage 1', 13, 10, 0
error_msg db 'Error loading Stage 2', 13, 10, 0
boot_drive db 0

times 510-($-$$) db 0  ; Pad to 510 bytes
dw 0xaa55              ; Boot signature
