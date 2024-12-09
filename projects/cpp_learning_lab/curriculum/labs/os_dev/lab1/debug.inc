; Debug Utility Functions
; ====================

; Constants
DEBUG_PORT equ 0xE9           ; QEMU debug port
BOCHS_PORT equ 0xE9          ; Bochs debug port
VIDEO_MEM equ 0xB8000        ; Text mode video memory

; Debug Macros
; -----------

; Print debug message to QEMU/Bochs debug port
%macro DEBUG_MSG 1
    pusha
    mov si, %1
    call debug_print
    call debug_newline
    popa
%endmacro

; Print register value in hex
%macro DEBUG_REG 1
    pusha
    mov eax, %1
    call print_hex
    call debug_newline
    popa
%endmacro

; Print memory dump
%macro DEBUG_DUMP 2
    pusha
    mov esi, %1             ; Source address
    mov ecx, %2             ; Number of bytes
    call dump_memory
    popa
%endmacro

; Break execution (Bochs magic breakpoint)
%macro DEBUG_BREAK 0
    xchg bx, bx
%endmacro

; Utility Functions
; ---------------

; Print null-terminated string to debug port
debug_print:
    lodsb
    test al, al
    jz .done
    out DEBUG_PORT, al
    jmp debug_print
.done:
    ret

; Print newline to debug port
debug_newline:
    mov al, 13
    out DEBUG_PORT, al
    mov al, 10
    out DEBUG_PORT, al
    ret

; Print hex value in EAX
print_hex:
    pusha
    mov cx, 8               ; 8 hex digits
.loop:
    rol eax, 4             ; Rotate left by 4 bits
    mov bl, al
    and bl, 0x0F           ; Mask off high bits
    add bl, '0'            ; Convert to ASCII
    cmp bl, '9'
    jle .print
    add bl, 7              ; Adjust for A-F
.print:
    mov al, bl
    out DEBUG_PORT, al
    dec cx
    jnz .loop
    popa
    ret

; Dump memory region
; ESI = source address, ECX = count
dump_memory:
    push eax
    push ebx
    push edx

.line:
    ; Print address
    mov eax, esi
    call print_hex
    mov al, ':'
    out DEBUG_PORT, al
    mov al, ' '
    out DEBUG_PORT, al

    ; Store counter
    push ecx
    mov edx, ecx
    cmp edx, 16
    jl .partial_line
    mov edx, 16
.partial_line:

    ; Print hex values
    mov ebx, esi
.hex:
    mov al, [ebx]
    shr al, 4
    call print_hex_digit
    mov al, [ebx]
    and al, 0x0F
    call print_hex_digit
    mov al, ' '
    out DEBUG_PORT, al
    inc ebx
    dec edx
    jnz .hex

    ; Print ASCII
    mov edx, 16
    sub edx, ebx
    add edx, esi
    mov al, ' '
.pad_spaces:
    test edx, edx
    jz .ascii
    out DEBUG_PORT, al
    out DEBUG_PORT, al
    out DEBUG_PORT, al
    dec edx
    jmp .pad_spaces

.ascii:
    mov al, '|'
    out DEBUG_PORT, al
    mov edx, ecx
    cmp edx, 16
    jl .partial_ascii
    mov edx, 16
.partial_ascii:
    mov ebx, esi
.ascii_char:
    mov al, [ebx]
    cmp al, 32
    jl .dot
    cmp al, 126
    jle .print_ascii
.dot:
    mov al, '.'
.print_ascii:
    out DEBUG_PORT, al
    inc ebx
    dec edx
    jnz .ascii_char
    
    mov al, '|'
    out DEBUG_PORT, al
    call debug_newline

    ; Update position and counter
    add esi, 16
    pop ecx
    sub ecx, 16
    jg .line

    pop edx
    pop ebx
    pop eax
    ret

; Print single hex digit in AL
print_hex_digit:
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jle .print
    add al, 7
.print:
    out DEBUG_PORT, al
    ret

; Error Handling
; -------------

; Print error message and halt
%macro PANIC 1
    cli                     ; Disable interrupts
    DEBUG_MSG %1            ; Print error message
    DEBUG_BREAK             ; Break into debugger
    hlt                     ; Halt CPU
    jmp $                   ; Infinite loop
%endmacro

; Assert condition
%macro ASSERT 2
    jnz %%ok               ; If ZF=0, condition is true
    PANIC %2               ; Otherwise, panic with message
%%ok:
%endmacro

; Memory Testing
; -------------

; Test memory region
; ESI = start address, ECX = size
test_memory:
    push eax
    push ebx
    push edx

.test_loop:
    ; Write pattern
    mov eax, esi           ; Use address as pattern
    mov [esi], eax
    
    ; Read back and verify
    mov ebx, [esi]
    cmp eax, ebx
    jne .error
    
    add esi, 4
    sub ecx, 4
    jnz .test_loop
    
    pop edx
    pop ebx
    pop eax
    ret

.error:
    DEBUG_MSG memory_test_failed
    DEBUG_REG eax          ; Expected value
    DEBUG_REG ebx          ; Actual value
    DEBUG_REG esi          ; Failed address
    PANIC memory_test_failed

; Messages
memory_test_failed db 'Memory test failed!', 0
