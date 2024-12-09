# OS Development Guide

## Introduction to OS Development

This guide covers the fundamentals of operating system development, from bootloader to kernel implementation.

### Topics Covered

1. Boot Process
2. Protected Mode
3. Memory Management
4. Process Scheduling
5. File Systems
6. Device Drivers

## 1. Boot Process

### Stage 1 Bootloader
```nasm
[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    
    ; Set up stack
    mov ss, ax
    mov sp, 0x7C00
    
    ; Load stage 2
    mov ah, 0x02        ; BIOS read sectors
    mov al, 1           ; Number of sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2
    mov dh, 0           ; Head 0
    mov bx, 0x7E00      ; Load to ES:BX
    int 0x13
    
    ; Jump to stage 2
    jmp 0x7E00

times 510-($-$$) db 0   ; Pad to 510 bytes
dw 0xAA55               ; Boot signature
```

### Protected Mode Setup
```cpp
// GDT setup
struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// Initialize GDT
void init_gdt() {
    // Set up GDT entries
    gdt_entries[0] = create_gdt_entry(0, 0, 0, 0);                // Null segment
    gdt_entries[1] = create_gdt_entry(0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_entries[2] = create_gdt_entry(0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    
    // Load GDT
    gdt_flush((uint32_t)&gdt_ptr);
}
```

## 2. Memory Management

### Virtual Memory
```cpp
class PageDirectory {
public:
    PageDirectory() {
        tables = new PageTable[1024];
        physicalAddr = (uint32_t)tables;
    }
    
    void map_page(uint32_t virtual_addr, uint32_t physical_addr, bool kernel, bool writeable) {
        uint32_t pd_idx = virtual_addr >> 22;
        uint32_t pt_idx = (virtual_addr >> 12) & 0x3FF;
        
        // Map the page
        tables[pd_idx].pages[pt_idx].present = 1;
        tables[pd_idx].pages[pt_idx].rw = writeable ? 1 : 0;
        tables[pd_idx].pages[pt_idx].user = kernel ? 0 : 1;
        tables[pd_idx].pages[pt_idx].frame = physical_addr >> 12;
    }
    
private:
    PageTable* tables;
    uint32_t physicalAddr;
};
```

### Memory Allocation
```cpp
class KernelHeap {
public:
    void* allocate(size_t size) {
        // Find a free block
        Block* block = find_free_block(size);
        if (!block) {
            // Expand heap
            block = expand_heap(size);
        }
        
        // Split if necessary
        if (block->size > size + sizeof(Block)) {
            split_block(block, size);
        }
        
        block->free = false;
        return block->data;
    }
    
private:
    struct Block {
        size_t size;
        bool free;
        Block* next;
        uint8_t data[];
    };
    
    Block* head;
};
```

## 3. Process Management

### Process Structure
```cpp
struct Process {
    uint32_t pid;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    PageDirectory* page_directory;
    uint32_t kernel_stack;
    uint32_t state;
};

class ProcessManager {
public:
    Process* create_process(void (*entry)()) {
        Process* process = new Process();
        process->pid = next_pid++;
        process->page_directory = new PageDirectory();
        
        // Set up stack
        process->kernel_stack = allocate_kernel_stack();
        process->esp = process->kernel_stack + STACK_SIZE;
        
        // Set up initial state
        process->eip = (uint32_t)entry;
        process->state = PROCESS_STATE_READY;
        
        return process;
    }
    
private:
    uint32_t next_pid = 1;
};
```

### Scheduler
```cpp
class Scheduler {
public:
    void schedule() {
        Process* current = get_current_process();
        Process* next = nullptr;
        
        // Round-robin scheduling
        if (current) {
            if (current->state == PROCESS_STATE_RUNNING) {
                current->state = PROCESS_STATE_READY;
                ready_queue.push(current);
            }
        }
        
        if (!ready_queue.empty()) {
            next = ready_queue.front();
            ready_queue.pop();
            next->state = PROCESS_STATE_RUNNING;
            switch_to_process(next);
        }
    }
    
private:
    std::queue<Process*> ready_queue;
};
```

## 4. File Systems

### Virtual File System
```cpp
class VFS {
public:
    struct FileNode {
        char name[256];
        uint32_t flags;
        uint32_t inode;
        uint32_t size;
        uint32_t impl;
        void* private_data;
        
        // Function pointers for operations
        ssize_t (*read)(struct FileNode*, uint64_t, size_t, uint8_t*);
        ssize_t (*write)(struct FileNode*, uint64_t, size_t, const uint8_t*);
        void (*open)(struct FileNode*);
        void (*close)(struct FileNode*);
    };
    
    FileNode* root;
    
    void mount(const char* path, FileNode* node) {
        // Mount implementation
    }
    
    FileNode* find(const char* path) {
        // Path resolution
        return nullptr;
    }
};
```

## 5. Device Drivers

### Driver Interface
```cpp
class Driver {
public:
    virtual void init() = 0;
    virtual void cleanup() = 0;
    virtual ssize_t read(uint64_t offset, size_t size, void* buffer) = 0;
    virtual ssize_t write(uint64_t offset, size_t size, const void* buffer) = 0;
    virtual int ioctl(uint32_t request, void* arg) = 0;
};

class KeyboardDriver : public Driver {
public:
    void init() override {
        // Register interrupt handler
        register_interrupt_handler(IRQ1, keyboard_handler);
        
        // Initialize keyboard controller
        while (inb(0x64) & 2);
        outb(0x64, 0xAE);  // Enable keyboard
    }
    
    static void keyboard_handler(registers_t regs) {
        uint8_t scancode = inb(0x60);
        // Handle scancode
    }
};
```

## Next Steps

1. Complete the [Memory Management Tutorial](memory.md)
2. Implement a [Basic Shell](shell.md)
3. Add [File System Support](filesystem.md)

## References

- Intel® 64 and IA-32 Architectures Software Developer's Manual
- Operating Systems: Three Easy Pieces
- OSDev Wiki
