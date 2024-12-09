# C++ Learning Lab & OS Development Environment

<div align="center">

![C++ Version](https://img.shields.io/badge/C++-17%2F20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-x86%20%7C%20ARM-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
[![Build Status](https://img.shields.io/github/workflow/status/ghost/cpp-learning-lab/CI?style=for-the-badge)](https://github.com/ghost/cpp-learning-lab/actions)

A Harvard-level computer systems programming environment for mastering C++ and OS development.

[Getting Started](#getting-started) •
[Documentation](#documentation) •
[Examples](#examples) •
[Contributing](#contributing)

</div>

## Educational Philosophy

This project follows Harvard's CS50 and advanced systems programming curriculum principles:
- Learn by doing with hands-on projects
- Deep understanding of computer systems
- Industry-standard tools and practices
- Rigorous academic standards

## Features

### Interactive Code Sandbox
- Real-time compilation with LLVM/Clang
- Integrated debugger with memory inspection
- Performance profiling tools
- Code analysis and suggestions
- Multi-file project support
- Version control integration

### OS Development Lab
- Professional bootloader development
  - Protected mode transition
  - Paging setup
  - Multi-stage boot process
- Modern kernel architecture
  - Microkernel design
  - Memory management unit
  - Advanced scheduler
  - IPC mechanisms
- Hardware abstraction layer
  - Device driver framework
  - Interrupt handling
  - DMA support
- Cross-platform support
  - x86/x64 architecture
  - ARM architecture (v7/v8)

### Advanced Tutorial Series
1. **Systems Programming Fundamentals**
   - Memory models and management
   - Cache optimization
   - SIMD instructions
   - Lock-free programming

2. **Operating System Concepts**
   - Virtual memory implementation
   - Process scheduling algorithms
   - File system design
   - Network stack development

3. **Modern C++ Mastery**
   - Template metaprogramming
   - Coroutines and async programming
   - Custom allocators
   - CRTP and static polymorphism

## Building from Source

### Prerequisites

#### Ubuntu/Debian
```bash
# Essential build tools
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build

# Development tools
sudo apt-get install -y llvm clang clang-format clang-tidy
sudo apt-get install -y gdb lldb valgrind

# OS development tools
sudo apt-get install -y qemu-system-x86 nasm grub-pc-bin xorriso
sudo apt-get install -y gcc-multilib g++-multilib

# Documentation
sudo apt-get install -y doxygen graphviz
```

### Build Commands
```bash
# Configure with all features
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_TESTING=ON \
      -DENABLE_SANITIZERS=ON \
      -DBUILD_DOCUMENTATION=ON

# Build everything
cmake --build build -j$(nproc)

# Run tests
cd build && ctest --output-on-failure
```

## Documentation

- [Getting Started Guide](docs/getting_started.md)
- [OS Development Tutorial](docs/os_dev/README.md)
- [API Reference](https://ghost.github.io/cpp-learning-lab/)
- [Contributing Guidelines](CONTRIBUTING.md)

## Examples

### Custom Memory Allocator
```cpp
template<typename T>
class PoolAllocator {
    static constexpr size_t BlockSize = 4096;
    std::vector<std::byte> memory;
    // ... implementation
};
```

### Kernel Thread Scheduler
```cpp
class Scheduler {
    std::priority_queue<Thread> ready_queue;
    std::vector<Thread> blocked_queue;
    // ... implementation
};
```

## Contributing

We welcome contributions! See our [Contributing Guidelines](CONTRIBUTING.md) for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Harvard CS50 team for inspiration
- OSDEV community
- Modern C++ community
