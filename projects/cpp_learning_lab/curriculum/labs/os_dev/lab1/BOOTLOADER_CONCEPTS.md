# Advanced Bootloader Development: Core Concepts

## 1. Real Mode Fundamentals
### Memory Segmentation
Real mode uses a segmented memory model:
```
Physical Address = Segment * 16 + Offset
Example: Segment 0x1000, Offset 0x0100 = 0x10100
```

### Interrupt Vector Table (IVT)
- Located at 0x0000:0x0000
- Contains 256 4-byte entries
- Each entry: 2 bytes segment, 2 bytes offset
```nasm
; Example: Get interrupt vector
mov ah, 0x35      ; DOS get interrupt vector
mov al, 0x13      ; Interrupt number
int 0x21          ; DOS interrupt
; ES:BX now contains vector
```

## 2. BIOS Services
### Video Services (INT 0x10)
```nasm
; Set video mode
mov ah, 0x00      ; Set mode function
mov al, 0x03      ; 80x25 text mode
int 0x10

; Write character
mov ah, 0x0E      ; Teletype output
mov al, 'A'       ; Character
int 0x10
```

### Disk Services (INT 0x13)
```nasm
; Read sectors
mov ah, 0x02      ; Read sectors function
mov al, 1         ; Number of sectors
mov ch, 0         ; Cylinder
mov cl, 2         ; Sector (1-based)
mov dh, 0         ; Head
mov dl, [drive]   ; Drive number
mov bx, buffer    ; Buffer address
int 0x13
```

## 3. Protected Mode Transition
### Global Descriptor Table (GDT)
```nasm
gdt_start:
    ; Null descriptor
    dq 0x0000000000000000

    ; Code segment
    dw 0xFFFF       ; Limit (0-15)
    dw 0x0000       ; Base (0-15)
    db 0x00         ; Base (16-23)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags + Limit (16-19)
    db 0x00         ; Base (24-31)

    ; Data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
```

### Protected Mode Switch Steps
1. Disable interrupts (CLI)
2. Load GDT (LGDT)
3. Enable A20 line
4. Set PE bit in CR0
5. Far jump to flush pipeline
6. Set up segment registers
7. Set up stack
8. Enable interrupts (STI)

## 4. Long Mode Setup
### Requirements
- CPU must support long mode
- Paging must be enabled
- A20 line must be enabled

### Page Tables
Four levels of paging:
1. PML4 (Page Map Level 4)
2. PDPT (Page Directory Pointer Table)
3. PD (Page Directory)
4. PT (Page Table)

```nasm
; Example: Basic page table setup
setup_pages:
    ; Clear page tables
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd

    ; Set up page tables
    mov edi, cr3
    mov dword [edi], 0x2003      ; PML4[0] -> PDPT
    add edi, 0x1000
    mov dword [edi], 0x3003      ; PDPT[0] -> PD
    add edi, 0x1000
    mov dword [edi], 0x4003      ; PD[0] -> PT
    add edi, 0x1000

    ; Identity map first 2MB
    mov ebx, 0x00000003          ; Present + writable
    mov ecx, 512                 ; 512 entries
.set_entry:
    mov dword [edi], ebx
    add ebx, 0x1000
    add edi, 8
    loop .set_entry
```

## 5. Advanced Debugging Techniques
### QEMU Debugging
```bash
# Run with debug port
qemu-system-x86_64 -debugcon stdio

# GDB debugging
qemu-system-x86_64 -s -S
```

### Bochs Debugging
```text
# bochsrc configuration
magic_break: enabled=1
port_e9_hack: enabled=1
```

### In-Code Debugging
```nasm
; Debug port output
mov al, 'D'
out 0xE9, al

; Bochs breakpoint
xchg bx, bx
```

## 6. Memory Management
### Memory Map (INT 0x15, EAX=0xE820)
```nasm
detect_memory:
    mov di, buffer
    xor ebx, ebx
    mov edx, 0x534D4150    ; 'SMAP'
    mov eax, 0xE820
    mov [es:di + 20], dword 1  ; Force valid ACPI entry
    mov ecx, 24
    int 0x15
    jc .error
```

### Memory Types
1. Type 1: Available RAM
2. Type 2: Reserved
3. Type 3: ACPI Reclaimable
4. Type 4: ACPI NVS
5. Type 5: Bad Memory

## 7. Error Handling Best Practices
### Disk Errors
```nasm
read_disk:
    mov ah, 0x02
    int 0x13
    jc .error
    cmp al, dh          ; AL = sectors read
    jne .error
    ret
.error:
    mov ah, 0x01        ; Get status
    int 0x13
    ; AH contains error code
```

### Memory Detection Errors
```nasm
check_memory:
    mov eax, 0xE820
    int 0x15
    jc .error           ; Carry set = function not supported
    cmp eax, 0x534D4150 ; 'SMAP'
    jne .error          ; EAX should contain 'SMAP'
```

## 8. Advanced Topics
### Multiboot Compliance
```nasm
; Multiboot header
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

align 4
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM
```

### ELF Loading
```nasm
; ELF header structure
struc elf_header
    .magic:     resd 1    ; 0x7F + "ELF"
    .class:     resb 1    ; 1 = 32 bit, 2 = 64 bit
    .data:      resb 1    ; 1 = little endian
    .version:   resb 1    ; Should be 1
    .pad:       resb 9
    .type:      resw 1    ; 2 = executable
    .machine:   resw 1    ; 3 = x86
    .version2:  resd 1
    .entry:     resd 1    ; Entry point
    .phoff:     resd 1    ; Program header offset
    .shoff:     resd 1    ; Section header offset
    .flags:     resd 1
    .ehsize:    resw 1
    .phentsize: resw 1
    .phnum:     resw 1
    .shentsize: resw 1
    .shnum:     resw 1
    .shstrndx:  resw 1
endstruc
```

## 9. Performance Optimization
### Fast A20 Gate
```nasm
enable_a20_fast:
    in al, 0x92
    test al, 2
    jnz .done
    or al, 2
    and al, 0xFE
    out 0x92, al
.done:
    ret
```

### Optimized Memory Copy
```nasm
; Fast memory copy using REP MOVSD
copy_memory:
    cld                     ; Clear direction flag
    mov esi, source
    mov edi, destination
    mov ecx, count
    shr ecx, 2             ; Convert bytes to dwords
    rep movsd
    mov ecx, count
    and ecx, 3             ; Remaining bytes
    rep movsb
```

## 10. Testing and Verification
### Memory Testing
```nasm
test_memory:
    mov edi, start_addr
    mov ecx, length
.loop:
    mov eax, [edi]         ; Save original
    mov ebx, 0xAAAAAAAA
    mov [edi], ebx         ; Test pattern 1
    mov edx, [edi]
    cmp edx, ebx
    jne .error
    mov ebx, 0x55555555
    mov [edi], ebx         ; Test pattern 2
    mov edx, [edi]
    cmp edx, ebx
    jne .error
    mov [edi], eax         ; Restore
    add edi, 4
    loop .loop
    ret
```

### CPU Feature Testing
```nasm
check_cpu:
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

    ; Check long mode
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
```
