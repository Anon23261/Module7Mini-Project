# Ghost OS C++ Toolkit

A cross-platform OS development toolkit written in modern C++17, designed for Windows and ARM platforms.

## Features

### Core Functionality
- System information retrieval
- Memory protection management
- Process enumeration and monitoring
- Hardware access controls
- Cross-platform compatibility layer

### Platform Support
- Windows
  - Native Windows API integration
  - Process manipulation
  - Memory management
  - System information gathering

- ARM
  - Bare metal support
  - Hardware register access
  - Memory protection unit (MPU) configuration
  - System control block (SCB) management

## Prerequisites

### Windows Development
- Visual Studio 2019 or later
- CMake 3.15+
- Boost libraries
- Windows SDK

### ARM Development
- ARM GCC Toolchain
- CMake 3.15+
- ARM development board (for testing)

## Building

### Windows
```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

### ARM Cross-Compilation
```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake ..
make
```

## Usage Example

```cpp
#include <ghost/os_toolkit.hpp>
#include <iostream>

int main() {
    try {
        // Create toolkit with custom configuration
        ghost::os::OsToolkit::Config config;
        config.enable_hardware_access = true;
        config.enable_process_manipulation = true;

        ghost::os::OsToolkit toolkit(config);

        // Get system information
        auto sysInfo = toolkit.getSystemInfo();
        std::cout << "Architecture: " << sysInfo.architecture << "\n";
        std::cout << "OS Type: " << sysInfo.os_type << "\n";
        std::cout << "CPU Cores: " << sysInfo.cpu_cores << "\n";

        // Enumerate processes
        auto processes = toolkit.enumerateProcesses();
        for (const auto& proc : processes) {
            std::cout << "Process: " << proc.name 
                      << " (PID: " << proc.pid << ")\n";
        }
    } catch (const ghost::os::OsToolkitError& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

## Security Notes

This toolkit provides low-level system access. Use with caution:
- Always verify process and memory access permissions
- Follow principle of least privilege
- Test thoroughly in controlled environments
- Be aware of platform-specific security implications

## License

MIT License - See LICENSE file for details
