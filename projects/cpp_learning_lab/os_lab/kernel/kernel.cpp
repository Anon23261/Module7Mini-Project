#include "kernel.hpp"
#include "memory/memory_manager.hpp"
#include "drivers/vga.hpp"
#include "interrupts/idt.hpp"

namespace ghost_os {

class Kernel {
private:
    MemoryManager mem_manager;
    VGADriver vga;
    InterruptDescriptorTable idt;
    bool initialized;

public:
    Kernel() : initialized(false) {}

    void init() {
        if (initialized) return;

        // Initialize VGA driver
        vga.init();
        vga.clear_screen();
        vga.print("GHOST OS Kernel Initializing...\n");

        // Initialize memory manager
        mem_manager.init();
        vga.print("Memory Manager initialized\n");

        // Setup interrupt descriptor table
        idt.init();
        vga.print("IDT initialized\n");

        // Enable interrupts
        asm volatile("sti");
        vga.print("Interrupts enabled\n");

        initialized = true;
        vga.print("Kernel initialization complete!\n");
    }

    void panic(const char* message) {
        vga.set_color(VGAColor::RED, VGAColor::BLACK);
        vga.print("\nKERNEL PANIC: ");
        vga.print(message);
        vga.print("\nSystem halted.");

        // Disable interrupts and halt
        asm volatile("cli; hlt");
    }

    void run() {
        if (!initialized) {
            panic("Kernel not initialized!");
            return;
        }

        vga.print("\nGHOST OS is running!\n");
        vga.print("Type 'help' for available commands.\n");

        while (true) {
            vga.print("\n> ");
            char command[256];
            read_command(command, sizeof(command));
            execute_command(command);
        }
    }

private:
    void read_command(char* buffer, size_t size) {
        size_t i = 0;
        while (i < size - 1) {
            char c = keyboard_read();
            if (c == '\n') break;
            if (c == '\b' && i > 0) {
                vga.print("\b \b");
                i--;
                continue;
            }
            if (c >= ' ' && c <= '~') {
                vga.putchar(c);
                buffer[i++] = c;
            }
        }
        buffer[i] = '\0';
    }

    void execute_command(const char* command) {
        if (strcmp(command, "help") == 0) {
            vga.print("\nAvailable commands:\n");
            vga.print("  help     - Show this help message\n");
            vga.print("  clear    - Clear the screen\n");
            vga.print("  memory   - Show memory information\n");
            vga.print("  shutdown - Shutdown the system\n");
        }
        else if (strcmp(command, "clear") == 0) {
            vga.clear_screen();
        }
        else if (strcmp(command, "memory") == 0) {
            mem_manager.print_info();
        }
        else if (strcmp(command, "shutdown") == 0) {
            vga.print("\nShutting down...");
            // Implement ACPI shutdown here
            panic("Shutdown not implemented");
        }
        else if (strlen(command) > 0) {
            vga.print("\nUnknown command: ");
            vga.print(command);
        }
    }

    char keyboard_read() {
        // Simple polling keyboard driver
        const uint16_t KEYBOARD_DATA_PORT = 0x60;
        const uint16_t KEYBOARD_STATUS_PORT = 0x64;
        
        while (true) {
            if (inb(KEYBOARD_STATUS_PORT) & 1) {
                uint8_t scancode = inb(KEYBOARD_DATA_PORT);
                if (scancode > 0) {
                    return scancode_to_ascii(scancode);
                }
            }
        }
    }

    uint8_t inb(uint16_t port) {
        uint8_t value;
        asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    char scancode_to_ascii(uint8_t scancode) {
        // Simple scancode to ASCII conversion
        static const char ascii_table[] = {
            0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
            0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
            0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
            '*', 0, ' '
        };
        return (scancode < sizeof(ascii_table)) ? ascii_table[scancode] : 0;
    }
};

} // namespace ghost_os

extern "C" void kernel_main() {
    ghost_os::Kernel kernel;
    kernel.init();
    kernel.run();
}
