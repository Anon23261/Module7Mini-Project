; Bootloader Utility Functions
; ========================

; Constants
SECTOR_SIZE equ 512
VIDEO_MODE equ 0x03        ; 80x25 text mode

; Macros for common operations
; --------------------------

; Set video mode
%macro SET_VIDEO_MODE 0
    mov ah, 0x00
    mov al, VIDEO_MODE
    int 0x10
%endmacro

; Clear screen
%macro CLEAR_SCREEN 0
    mov ah, 0x06    ; Scroll up function
    xor al, al      ; Clear entire screen
    xor cx, cx      ; Upper left: (0,0)
    mov dx, 0x184F  ; Lower right: (79,24)
    mov bh, 0x07    ; White on black
    int 0x10
%endmacro

; Print character
%macro PRINT_CHAR 1
    mov ah, 0x0E
    mov al, %1
    int 0x10
%endmacro

; Print string (SI = string address)
%macro PRINT_STRING 1
    mov si, %1
    call print_string
%endmacro

; Read sectors from disk
; AX = number of sectors, BX = buffer, CX = start sector
%macro READ_SECTORS 0
    mov ah, 0x02
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
%endmacro

; Utility Functions
; ---------------

; Print string (SI = string address)
print_string:
    push ax
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop ax
    ret

; Print hex number in AX
print_hex:
    push bx
    push cx
    mov cx, 4       ; 4 hex digits
.loop:
    rol ax, 4       ; Rotate left by 4 bits
    mov bx, ax      
    and bx, 0x0F    ; Mask off high bits
    add bx, '0'     ; Convert to ASCII
    cmp bx, '9'     ; Adjust for A-F
    jle .print
    add bx, 7       ; 'A' - '9' - 1
.print:
    mov ah, 0x0E
    mov al, bl
    int 0x10
    loop .loop
    pop cx
    pop bx
    ret

; Print decimal number in AX
print_decimal:
    push ax
    push bx
    push cx
    push dx
    
    mov bx, 10      ; Divisor
    xor cx, cx      ; Digit counter
.divide:
    xor dx, dx      ; Clear high word
    div bx          ; Divide by 10
    push dx         ; Save remainder
    inc cx          ; Count digits
    test ax, ax     ; Check if done
    jnz .divide
    
.print:
    pop dx          ; Get digit
    add dl, '0'     ; Convert to ASCII
    mov ah, 0x0E
    mov al, dl
    int 0x10
    loop .print
    
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; Read from keyboard
read_key:
    mov ah, 0x00
    int 0x16
    ret

; Wait for keypress
wait_key:
    mov ah, 0x00
    int 0x16
    ret

; Get cursor position
get_cursor:
    mov ah, 0x03
    xor bh, bh
    int 0x10
    ret

; Set cursor position
; DH = row, DL = column
set_cursor:
    mov ah, 0x02
    xor bh, bh
    int 0x10
    ret

; Scroll screen up
; AL = number of lines (0 = clear screen)
scroll_up:
    mov ah, 0x06
    xor cx, cx
    mov dx, 0x184F
    mov bh, 0x07
    int 0x10
    ret

; Check A20 line
check_a20:
    pushf
    push ds
    push es
    push di
    push si
 
    xor ax, ax      ; Set ES:DI to 0000:0500
    mov es, ax
    mov di, 0x0500
 
    mov ax, 0xFFFF  ; Set DS:SI to FFFF:0510
    mov ds, ax
    mov si, 0x0510
 
    mov al, byte [es:di]     ; Save bytes
    push ax
    mov al, byte [ds:si]
    push ax
 
    mov byte [es:di], 0x00   ; Write different values
    mov byte [ds:si], 0xFF
 
    cmp byte [es:di], 0xFF   ; Compare values
 
    pop ax                   ; Restore bytes
    mov byte [ds:si], al
    pop ax
    mov byte [es:di], al
 
    mov ax, 0                ; Return 0 = disabled
    je .done                 ; 1 = enabled
    mov ax, 1
 
.done:
    pop si
    pop di
    pop es
    pop ds
    popf
    ret

; Enable A20 line through keyboard controller
enable_a20_kb:
    cli
    push ax
    
    call .wait_input
    mov al, 0xAD    ; Disable keyboard
    out 0x64, al
    
    call .wait_input
    mov al, 0xD0    ; Read output port
    out 0x64, al
    
    call .wait_output
    in al, 0x60
    push ax
    
    call .wait_input
    mov al, 0xD1    ; Write output port
    out 0x64, al
    
    call .wait_input
    pop ax
    or al, 2        ; Set A20 bit
    out 0x60, al
    
    call .wait_input
    mov al, 0xAE    ; Enable keyboard
    out 0x64, al
    
    call .wait_input
    pop ax
    sti
    ret
    
.wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input
    ret
    
.wait_output:
    in al, 0x64
    test al, 1
    jz .wait_output
    ret

; Enable A20 through BIOS
enable_a20_bios:
    mov ax, 0x2401
    int 0x15
    ret

; Enable A20 through Fast A20 Gate
enable_a20_fast:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; Error Handlers
; ------------
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

a20_error:
    mov si, a20_error_msg
    call print_string
    jmp $

; Messages
disk_error_msg db 'Disk read error!', 13, 10, 0
a20_error_msg db 'A20 line enable failed!', 13, 10, 0
