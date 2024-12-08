# Bootloader Development Exercises

## Basic Exercises

### Exercise 1: Hello World Bootloader
Create a simple bootloader that:
1. Prints "Hello, World!" using BIOS interrupts
2. Includes proper boot signature
3. Uses debug port for output

```nasm
[BITS 16]
[ORG 0x7C00]

start:
    ; Your code here
    ; Hint: Use int 0x10 with ah=0x0E

; Add boot signature
```

### Exercise 2: Memory Explorer
Create a bootloader that:
1. Displays first 16 bytes of memory at 0x7C00
2. Formats output in hexadecimal
3. Shows ASCII representation

### Exercise 3: Disk Reader
Create a bootloader that:
1. Reads sector 2 from disk
2. Displays its contents
3. Handles read errors

## Intermediate Exercises

### Exercise 4: Protected Mode Switch
Create a bootloader that:
1. Sets up GDT
2. Enables A20 line
3. Switches to protected mode
4. Prints a character in video memory

### Exercise 5: Memory Map Builder
Create a bootloader that:
1. Uses INT 0x15, EAX=0xE820
2. Builds memory map
3. Displays available memory regions

### Exercise 6: Multi-Stage Loader
Create a two-stage bootloader:
1. Stage 1: Load stage 2 from disk
2. Stage 2: Print memory map
3. Include error handling

## Advanced Exercises

### Exercise 7: Long Mode Bootloader
Create a bootloader that:
1. Checks CPU features
2. Sets up paging
3. Enters long mode
4. Prints "Hello from 64-bit mode!"

### Exercise 8: ELF Loader
Create a bootloader that:
1. Loads ELF format kernel
2. Parses ELF headers
3. Relocates sections
4. Jumps to entry point

### Exercise 9: Boot Menu
Create a bootloader with:
1. Multiple boot options
2. Configuration file support
3. Timeout feature
4. Memory detection

## Debugging Exercises

### Exercise 10: Debug Features
Add to any bootloader:
1. Port 0xE9 debug output
2. Memory dumps
3. Register state display
4. Error tracking

## Testing Your Solutions

### Using QEMU
```bash
# Compile bootloader
nasm -f bin bootloader.asm -o bootloader.bin

# Run in QEMU with debug output
qemu-system-x86_64 -drive format=raw,file=bootloader.bin -debugcon stdio
```

### Using Bochs
```bash
# Add to bochsrc:
magic_break: enabled=1
debug_symbols: file="bootloader.sym"
```

## Common Pitfalls

1. **Segment Registers**
   - Always initialize DS, ES, SS
   - Use proper segment calculations

2. **Stack Setup**
   - Set SS:SP before using stack
   - Avoid stack overflow

3. **Disk Operations**
   - Check carry flag after INT 0x13
   - Save boot drive number

4. **Protected Mode**
   - Enable A20 line first
   - Set up proper GDT
   - Disable interrupts

## Debug Tips

1. **Using Debug Port**
```nasm
mov al, 'A'
out 0xE9, al
```

2. **Memory Dump**
```nasm
mov si, 0x7C00
mov cx, 16
call dump_memory
```

3. **Register Display**
```nasm
pushad
call print_registers
popad
```

## Solutions

Example solutions are provided in the `solutions` directory. Try solving exercises yourself first!
