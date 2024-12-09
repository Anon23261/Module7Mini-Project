# Understanding the Bootloader: A Beginner's Guide

## What is a Bootloader?
A bootloader is a small program that runs when your computer starts up. It's responsible for:
1. Loading your operating system from disk into memory
2. Setting up the initial environment
3. Transferring control to the OS kernel

## The Boot Process
1. **Power On**
   - CPU starts in 16-bit Real Mode
   - BIOS performs Power-On Self Test (POST)
   - BIOS loads first sector (boot sector) into memory at 0x7C00

2. **Boot Sector**
   - Size: exactly 512 bytes
   - Must end with boot signature (0xAA55)
   - Contains our first-stage bootloader

3. **Memory Map in Real Mode**
```
0x00000000 - 0x000003FF: Interrupt Vector Table
0x00000400 - 0x000004FF: BIOS Data Area
0x00000500 - 0x00007BFF: Free Memory
0x00007C00 - 0x00007DFF: Our Bootloader (512 bytes)
0x00007E00 - 0x0009FFFF: Free Memory
0x000A0000 - 0x000FFFFF: Hardware, ROM, etc.
```

## Understanding Our Bootloader Code

### 1. Initial Setup
```nasm
[BITS 16]                   ; Tell assembler we're in 16-bit mode
[ORG 0x7C00]                ; Code is loaded at this address
```
- `[BITS 16]`: We start in 16-bit real mode
- `[ORG 0x7C00]`: BIOS loads us here

### 2. Segment Registers
```nasm
xor ax, ax                  ; Clear AX register (set to 0)
mov ds, ax                  ; Data Segment = 0
mov es, ax                  ; Extra Segment = 0
mov ss, ax                  ; Stack Segment = 0
mov sp, 0x7C00             ; Stack grows downward from where we're loaded
```
- Sets up segments for memory access
- Initializes stack for function calls

### 3. Disk Operations
```nasm
mov ah, 0x02               ; BIOS read sector function
mov al, KERNEL_SECTORS     ; Number of sectors to read
mov ch, 0                  ; Cylinder 0
mov cl, 2                  ; Start from sector 2 (sector 1 is bootloader)
mov dh, 0                  ; Head 0
mov dl, [boot_drive]       ; Drive number saved by BIOS
mov bx, KERNEL_OFFSET      ; Where to load the kernel
int 0x13                   ; Call BIOS interrupt
```
- Uses BIOS interrupt 0x13 to read from disk
- Loads kernel into memory

### 4. A20 Line
```nasm
call enable_a20            ; Enable A20 line for >1MB memory access
```
- Required for accessing memory above 1MB
- Historical quirk from IBM PC days

### 5. Protected Mode Switch
```nasm
cli                        ; Disable interrupts
lgdt [gdt_descriptor]      ; Load GDT
mov eax, cr0              
or eax, 0x1               ; Set protected mode bit
mov cr0, eax              ; Switch to protected mode
jmp CODE_SEG:init_pm      ; Far jump to 32-bit code
```
- Prepares for and switches to 32-bit mode
- Sets up new memory segmentation

## Common Pitfalls and Solutions

1. **Boot Signature Missing**
   - Problem: BIOS won't recognize bootloader
   - Solution: Ensure last two bytes are 0x55, 0xAA

2. **Segment Register Issues**
   - Problem: Code/data access wrong memory
   - Solution: Always initialize DS, ES, SS properly

3. **Stack Setup**
   - Problem: Stack overwrites code
   - Solution: Set stack above or well below bootloader

4. **Disk Read Failures**
   - Problem: Can't load kernel
   - Solution: Always check carry flag after int 0x13

## Debugging Tips

1. **Using QEMU Debug Port**
```nasm
mov al, 'A'               ; Character to print
mov dx, 0xE9             ; QEMU debug port
out dx, al               ; Output character
```

2. **Using Bochs Debugger**
- Set breakpoints with `xchg bx, bx`
- Enable magic breakpoint in bochsrc

3. **Memory Dumps**
```nasm
mov si, message          ; Source address
mov cx, length          ; Number of bytes
call dump_memory        ; Your debug routine
```

## Practice Exercises

1. **Hello World Bootloader**
   - Print a message using BIOS int 0x10
   - End with proper boot signature

2. **Extended Read**
   - Read multiple sectors from disk
   - Handle read errors properly

3. **Memory Detection**
   - Use BIOS int 0x15, eax=0xE820
   - Create memory map for kernel

## Next Steps

1. **Second Stage Loader**
   - Load larger kernel
   - Setup paging
   - Parse ELF files

2. **Protected Mode**
   - Setup GDT
   - Enable paging
   - Jump to kernel

Remember: A bootloader is simple but critical. Take time to understand each concept before moving on!
