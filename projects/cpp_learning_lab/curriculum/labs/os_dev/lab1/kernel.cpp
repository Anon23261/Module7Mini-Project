#include <stdint.h>

// VGA text mode color constants
enum class VGAColor : uint8_t {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    LightMagenta = 13,
    Yellow = 14,
    White = 15
};

// Hardware text mode color scheme
class Terminal {
private:
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;
    static const uint16_t VGA_MEMORY = 0xB8000;

    size_t terminal_row;
    size_t terminal_column;
    uint8_t terminal_color;
    uint16_t* terminal_buffer;

    uint8_t make_color(VGAColor fg, VGAColor bg) {
        return static_cast<uint8_t>(fg) | static_cast<uint8_t>(bg) << 4;
    }

    uint16_t make_vgaentry(char c, uint8_t color) {
        uint16_t c16 = c;
        uint16_t color16 = color;
        return c16 | color16 << 8;
    }

public:
    Terminal() 
        : terminal_row(0)
        , terminal_column(0)
        , terminal_color(make_color(VGAColor::White, VGAColor::Black))
        , terminal_buffer(reinterpret_cast<uint16_t*>(VGA_MEMORY))
    {
        clear();
    }

    void clear() {
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t index = y * VGA_WIDTH + x;
                terminal_buffer[index] = make_vgaentry(' ', terminal_color);
            }
        }
    }

    void setColor(VGAColor fg, VGAColor bg) {
        terminal_color = make_color(fg, bg);
    }

    void putEntryAt(char c, uint8_t color, size_t x, size_t y) {
        const size_t index = y * VGA_WIDTH + x;
        terminal_buffer[index] = make_vgaentry(c, color);
    }

    void putChar(char c) {
        if (c == '\n') {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                scroll();
            }
            return;
        }

        putEntryAt(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                scroll();
            }
        }
    }

    void write(const char* data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            putChar(data[i]);
        }
    }

    void writeString(const char* str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            putChar(str[i]);
        }
    }

    void scroll() {
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t current_index = y * VGA_WIDTH + x;
                const size_t next_index = (y + 1) * VGA_WIDTH + x;
                terminal_buffer[current_index] = terminal_buffer[next_index];
            }
        }

        // Clear the last line
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            terminal_buffer[index] = make_vgaentry(' ', terminal_color);
        }
        terminal_row = VGA_HEIGHT - 1;
    }
};

// GDT setup
struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct GDTPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

class GDT {
private:
    static const size_t GDT_ENTRIES = 3;
    GDTEntry entries[GDT_ENTRIES];
    GDTPointer pointer;

public:
    GDT() {
        // Null descriptor
        setEntry(0, 0, 0, 0, 0);

        // Code segment
        setEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

        // Data segment
        setEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

        pointer.limit = (sizeof(GDTEntry) * GDT_ENTRIES) - 1;
        pointer.base = reinterpret_cast<uint32_t>(&entries[0]);

        loadGDT();
    }

private:
    void setEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
        entries[num].base_low = base & 0xFFFF;
        entries[num].base_middle = (base >> 16) & 0xFF;
        entries[num].base_high = (base >> 24) & 0xFF;

        entries[num].limit_low = limit & 0xFFFF;
        entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

        entries[num].access = access;
    }

    void loadGDT() {
        asm volatile("lgdt %0" : : "m"(pointer));
        
        // Reload segments
        asm volatile(
            "ljmp $0x08, $reload_cs\n\t"
            "reload_cs:\n\t"
            "mov $0x10, %ax\n\t"
            "mov %ax, %ds\n\t"
            "mov %ax, %es\n\t"
            "mov %ax, %fs\n\t"
            "mov %ax, %gs\n\t"
            "mov %ax, %ss"
        );
    }
};

// Kernel entry point
extern "C" void kernel_main() {
    // Initialize terminal
    Terminal terminal;
    
    // Set up GDT
    GDT gdt;

    // Welcome message
    terminal.setColor(VGAColor::Green, VGAColor::Black);
    terminal.writeString("GHOST OS Kernel v1.0\n");
    terminal.writeString("-------------------\n\n");
    
    terminal.setColor(VGAColor::White, VGAColor::Black);
    terminal.writeString("System initialized successfully!\n");
    terminal.writeString("GDT loaded and segments reloaded\n");
    terminal.writeString("Terminal interface active\n");

    // Kernel loop
    while (true) {
        asm volatile("hlt");
    }
}

// Entry point must be at the start of kernel binary
extern "C" void _start() {
    kernel_main();
}
