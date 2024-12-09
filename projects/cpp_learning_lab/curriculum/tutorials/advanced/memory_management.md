# Advanced Memory Management in Operating Systems

## Virtual Memory Implementation Guide

### 1. Page Table Implementation

```cpp
struct PageTableEntry {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw        : 1;   // Read/write permission
    uint32_t user      : 1;   // User/supervisor level
    uint32_t accessed  : 1;   // Has page been accessed?
    uint32_t dirty     : 1;   // Has page been written to?
    uint32_t unused    : 7;   // Reserved bits
    uint32_t frame     : 20;  // Frame address (shifted right 12 bits)
};

struct PageTable {
    PageTableEntry pages[1024];
};

struct PageDirectory {
    // Contains physical addresses of page tables
    uint32_t tables[1024];
    // Contains physical addresses of the tablesPhysical array
    uint32_t tablesPhysical[1024];
    // Physical address of tablesPhysical
    uint32_t physicalAddr;
};
```

### 2. Memory Manager Implementation

```cpp
class MemoryManager {
private:
    PageDirectory* current_directory = nullptr;
    uint32_t* frames;
    uint32_t nframes;

    // Bitmap operations for frame allocation
    static constexpr uint32_t INDEX_FROM_BIT(uint32_t a) {
        return a / (8 * 4);
    }
    static constexpr uint32_t OFFSET_FROM_BIT(uint32_t a) {
        return a % (8 * 4);
    }

public:
    void initialize(uint32_t mem_size) {
        nframes = mem_size / 0x1000;
        frames = new uint32_t[INDEX_FROM_BIT(nframes)];
        memset(frames, 0, INDEX_FROM_BIT(nframes));
    }

    // Set a bit in the frames bitset
    void set_frame(uint32_t frame_addr) {
        uint32_t frame = frame_addr / 0x1000;
        uint32_t idx = INDEX_FROM_BIT(frame);
        uint32_t off = OFFSET_FROM_BIT(frame);
        frames[idx] |= (0x1 << off);
    }

    // Clear a bit in the frames bitset
    void clear_frame(uint32_t frame_addr) {
        uint32_t frame = frame_addr / 0x1000;
        uint32_t idx = INDEX_FROM_BIT(frame);
        uint32_t off = OFFSET_FROM_BIT(frame);
        frames[idx] &= ~(0x1 << off);
    }

    // Find the first free frame
    uint32_t first_frame() {
        for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
            if (frames[i] != 0xFFFFFFFF) {
                for (uint32_t j = 0; j < 32; j++) {
                    uint32_t toTest = 0x1 << j;
                    if (!(frames[i] & toTest)) {
                        return i * 32 + j;
                    }
                }
            }
        }
        return -1;
    }

    // Allocate a frame
    void alloc_frame(PageTableEntry* page, bool is_kernel, bool is_writeable) {
        if (page->frame != 0) {
            return; // Frame was already allocated
        }
        
        uint32_t idx = first_frame();
        if (idx == (uint32_t)-1) {
            throw std::runtime_error("No free frames!");
        }
        
        set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = (is_writeable) ? 1 : 0;
        page->user = (is_kernel) ? 0 : 1;
        page->frame = idx;
    }

    // Free a frame
    void free_frame(PageTableEntry* page) {
        uint32_t frame = page->frame;
        if (!frame) {
            return; // The given page didn't have an allocated frame
        }
        clear_frame(frame);
        page->frame = 0x0;
    }
};
```

### 3. Heap Implementation

```cpp
struct Header {
    size_t size;
    bool is_free;
    Header* next;
};

class KernelHeap {
private:
    Header* start;
    static constexpr size_t ALIGN = 8;
    static constexpr size_t HEADER_SIZE = sizeof(Header);

    // Align size to boundary
    size_t align(size_t n) {
        return (n + ALIGN - 1) & ~(ALIGN - 1);
    }

public:
    KernelHeap(void* start_addr, size_t size) {
        start = (Header*)start_addr;
        start->size = size - HEADER_SIZE;
        start->is_free = true;
        start->next = nullptr;
    }

    void* allocate(size_t size) {
        size = align(size);
        Header* current = start;

        while (current) {
            if (current->is_free && current->size >= size) {
                // Split if we have enough space
                if (current->size > size + HEADER_SIZE) {
                    Header* new_block = (Header*)((char*)current + HEADER_SIZE + size);
                    new_block->size = current->size - size - HEADER_SIZE;
                    new_block->is_free = true;
                    new_block->next = current->next;
                    
                    current->size = size;
                    current->next = new_block;
                }
                
                current->is_free = false;
                return (void*)((char*)current + HEADER_SIZE);
            }
            current = current->next;
        }
        
        return nullptr; // Out of memory
    }

    void free(void* ptr) {
        if (!ptr) return;

        Header* header = (Header*)((char*)ptr - HEADER_SIZE);
        header->is_free = true;

        // Coalesce with next block if it's free
        if (header->next && header->next->is_free) {
            header->size += HEADER_SIZE + header->next->size;
            header->next = header->next->next;
        }

        // Find previous block to coalesce
        Header* current = start;
        while (current != header && current) {
            if (current->is_free && 
                (Header*)((char*)current + HEADER_SIZE + current->size) == header) {
                current->size += HEADER_SIZE + header->size;
                current->next = header->next;
                break;
            }
            current = current->next;
        }
    }
};
```

### 4. Memory Protection

```cpp
class MemoryProtection {
public:
    // Set up memory regions with different protection levels
    void setup_memory_regions() {
        // Kernel space (0-3GB): Supervisor mode only
        for (uint32_t i = 0; i < 768; i++) {
            PageTableEntry* page = get_page(i * 0x1000, true);
            alloc_frame(page, true, false);  // Kernel, read-only
        }

        // User space (3-4GB): User mode allowed
        for (uint32_t i = 768; i < 1024; i++) {
            PageTableEntry* page = get_page(i * 0x1000, true);
            alloc_frame(page, false, true);  // User, read-write
        }
    }

    // Handle page faults
    void page_fault_handler(registers_t regs) {
        uint32_t faulting_address;
        asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

        int present = !(regs.err_code & 0x1);
        int rw = regs.err_code & 0x2;
        int us = regs.err_code & 0x4;
        int reserved = regs.err_code & 0x8;

        // Log the fault details
        printf("Page fault at 0x%x\n", faulting_address);
        if (present) printf("Page not present\n");
        if (rw) printf("Write operation\n");
        if (us) printf("User mode\n");
        if (reserved) printf("Reserved bits overwritten\n");

        // Handle the fault or panic
        panic("Page Fault");
    }
};
```

## Exercises

1. Implement a buddy allocator system
2. Create a slab allocator for fixed-size objects
3. Implement memory mapping for device drivers
4. Add support for demand paging
5. Create a memory leak detector

## Advanced Topics to Explore

1. NUMA architectures
2. TLB management
3. Memory compression
4. Huge pages
5. Memory ballooning
6. Copy-on-write optimization

## References

- Intel® 64 and IA-32 Architectures Software Developer's Manual
- Modern Operating Systems by Andrew S. Tanenbaum
- Linux Kernel Development by Robert Love
