[BITS 16]
[ORG 0x9000]                ; Second stage loaded here

; Constants
; ---------
KERNEL_LOAD_ADDR equ 0x100000  ; Load kernel at 1MB
VIDEO_MEMORY equ 0xB8000       ; Text mode video memory
DEBUG_PORT equ 0xE9            ; QEMU debug port
MEMORY_MAP_ADDR equ 0x8000     ; Where to store memory map
MAX_MEMORY_ENTRIES equ 20      ; Maximum memory map entries

; Data Section
; ------------
section .data
    stage2_msg db 'GHOST OS Stage 2 Bootloader', 0
    memory_msg db 'Detecting System Memory...', 0
    cpu_msg db 'Checking CPU features...', 0
    paging_msg db 'Setting up paging...', 0
    kernel_msg db 'Loading kernel...', 0
    error_msg db 'Error: ', 0
    success_msg db 'Success!', 0
    memory_map_count dw 0      ; Number of memory map entries

; Debug Macros
; ------------
%macro DEBUG_PRINT 1
    pusha
    mov si, %1
    call debug_print
    popa
%endmacro

%macro CHECK_ERROR 1
    jnc %%no_error
    mov si, error_msg
    call debug_print
    mov si, %1
    call debug_print
    jmp error_halt
%%no_error:
%endmacro

; Entry Point
; ----------
start:
    ; Set up segments and stack
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFF

    ; Initialize debug output
    DEBUG_PRINT stage2_msg
    call print_newline

    ; Detect memory map
    call detect_memory
    CHECK_ERROR memory_msg

    ; Check CPU features
    call check_cpu
    CHECK_ERROR cpu_msg

    ; Load kernel from disk
    call load_kernel
    CHECK_ERROR kernel_msg

    ; Set up paging
    call setup_paging
    CHECK_ERROR paging_msg

    ; Switch to long mode
    call enter_long_mode

    ; Jump to kernel
    jmp KERNEL_LOAD_ADDR

; Memory Detection
; ---------------
detect_memory:
    pushad
    DEBUG_PRINT memory_msg
    
    mov di, MEMORY_MAP_ADDR    ; Where to store memory map
    xor ebx, ebx              ; Start with ebx = 0
    mov edx, 0x534D4150       ; 'SMAP'
    xor bp, bp                ; Entry count

.loop:
    mov eax, 0xE820           ; BIOS memory detection
    mov ecx, 24               ; Entry size
    int 0x15
    jc .done                  ; Carry set = end of list
    
    cmp eax, 0x534D4150       ; Check signature
    jne .done
    
    test ebx, ebx             ; ebx = 0 means last entry
    je .done
    
    inc bp                    ; Increment count
    add di, 24                ; Next entry
    cmp bp, MAX_MEMORY_ENTRIES
    jb .loop

.done:
    mov [memory_map_count], bp
    popad
    ret

; CPU Feature Detection
; -------------------
check_cpu:
    pushad
    DEBUG_PRINT cpu_msg

    ; Check CPUID support
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid

    ; Check long mode support
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode

    popad
    ret

.no_cpuid:
    mov si, no_cpuid_msg
    jmp error_halt

.no_long_mode:
    mov si, no_long_mode_msg
    jmp error_halt

; Kernel Loading
; -------------
load_kernel:
    pushad
    DEBUG_PRINT kernel_msg

    ; Set up DAP (Disk Address Packet)
    mov ah, 0x42             ; Extended read
    mov dl, [boot_drive]     ; Boot drive
    mov si, dap              ; Address of DAP
    int 0x13
    CHECK_ERROR kernel_msg

    popad
    ret

; Paging Setup
; -----------
setup_paging:
    pushad
    DEBUG_PRINT paging_msg

    ; Clear page tables
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd

    ; Set up page tables
    mov edi, cr3
    
    ; PML4
    mov dword [edi], 0x2003      ; Page directory pointer table
    add edi, 0x1000
    
    ; PDPT
    mov dword [edi], 0x3003      ; Page directory
    add edi, 0x1000
    
    ; PD
    mov dword [edi], 0x4003      ; Page table
    add edi, 0x1000
    
    ; PT
    mov ebx, 0x00000003          ; Present + writable
    mov ecx, 512                 ; 512 entries
    
.set_entry:
    mov dword [edi], ebx
    add ebx, 0x1000
    add edi, 8
    loop .set_entry

    popad
    ret

; Long Mode Entry
; -------------
enter_long_mode:
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

; Utility Functions
; ---------------
debug_print:
    lodsb
    test al, al
    jz .done
    out DEBUG_PORT, al
    jmp debug_print
.done:
    ret

print_newline:
    mov al, 13
    out DEBUG_PORT, al
    mov al, 10
    out DEBUG_PORT, al
    ret

error_halt:
    cli
    hlt
    jmp error_halt

; Data Structures
; -------------
align 8
dap:    ; Disk Address Packet
    db 0x10      ; Size of packet
    db 0         ; Reserved
    dw 127       ; Number of sectors
    dw 0         ; Offset
    dw 0x1000    ; Segment
    dq 1         ; Starting LBA

; Error Messages
; ------------
no_cpuid_msg db 'CPUID not supported', 0
no_long_mode_msg db 'Long mode not supported', 0
boot_drive db 0

; GDT for Long Mode
; ---------------
align 8
gdt64:
    dq 0                ; Null descriptor
.code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53) ; Code segment
.data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) ; Data segment
.pointer:
    dw $ - gdt64 - 1    ; Limit
    dq gdt64            ; Base

; Padding
; -------
times 4096-($-$$) db 0  ; Pad to 4KB
