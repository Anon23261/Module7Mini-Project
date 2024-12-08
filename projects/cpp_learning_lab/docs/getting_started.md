# Getting Started with C++ Learning Lab

## Prerequisites

Before diving into the C++ Learning Lab, ensure you have:

1. **Development Environment**
   - Linux/Unix-based system (Ubuntu 20.04+ recommended)
   - Visual Studio Code or CLion IDE
   - Git for version control

2. **System Requirements**
   - 4GB RAM minimum (8GB recommended)
   - 20GB free disk space
   - x86_64 or ARM64 processor

3. **Knowledge Prerequisites**
   - Basic programming concepts
   - Command line familiarity
   - Understanding of computer architecture

## Installation

### Step 1: Clone the Repository
```bash
git clone https://github.com/ghost/cpp-learning-lab.git
cd cpp-learning-lab
```

### Step 2: Install Dependencies
```bash
# Install build tools and compilers
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build
sudo apt-get install -y llvm clang clang-format clang-tidy
sudo apt-get install -y gdb lldb valgrind

# Install OS development tools
sudo apt-get install -y qemu-system-x86 nasm grub-pc-bin xorriso
sudo apt-get install -y gcc-multilib g++-multilib

# Install documentation tools
sudo apt-get install -y doxygen graphviz
```

### Step 3: Build the Project
```bash
# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_TESTING=ON \
      -DENABLE_SANITIZERS=ON \
      -DBUILD_DOCUMENTATION=ON

# Build all components
cmake --build build -j$(nproc)
```

### Step 4: Verify Installation
```bash
# Run tests
cd build && ctest --output-on-failure

# Launch sandbox
./sandbox/interactive_sandbox
```

## Quick Start Guide

### 1. Interactive Sandbox
The sandbox provides a real-time C++ development environment:

```cpp
// Example: Hello World
#include <iostream>

int main() {
    std::cout << "Hello, C++ Learning Lab!" << std::endl;
    return 0;
}
```

Key bindings:
- `F5`: Compile and run
- `F10`: Exit
- `Ctrl+Space`: Code completion
- `F12`: Go to definition

### 2. OS Development Lab
Start with basic bootloader development:

```bash
# Build and run the OS
cd os_lab
make os
make run
```

### 3. Tutorials
Begin with the fundamentals:

1. Basic C++:
   ```bash
   cd tutorials/01_basics
   make hello_world
   ./hello_world
   ```

2. Advanced Topics:
   ```bash
   cd tutorials/02_advanced
   make templates
   ./templates
   ```

3. OS Development:
   ```bash
   cd tutorials/03_os
   make bootloader
   ```

## Project Structure

```
cpp_learning_lab/
├── sandbox/               # Interactive coding environment
├── os_lab/               # OS development components
│   ├── bootloader/
│   ├── kernel/
│   └── drivers/
├── tutorials/            # Educational content
│   ├── 01_basics/
│   ├── 02_advanced/
│   └── 03_os/
└── examples/            # Example projects
```

## Next Steps

1. Complete the [Basic C++ Tutorial](tutorials/01_basics/README.md)
2. Explore [Memory Management](tutorials/02_advanced/memory.md)
3. Start [OS Development](tutorials/03_os/README.md)

## Troubleshooting

### Common Issues

1. Build Failures
   ```bash
   # Clean build directory
   rm -rf build
   cmake -B build
   cmake --build build
   ```

2. QEMU Problems
   ```bash
   # Check QEMU installation
   qemu-system-x86_64 --version
   
   # Enable KVM acceleration
   sudo modprobe kvm
   sudo modprobe kvm_intel  # or kvm_amd
   ```

3. Permission Issues
   ```bash
   # Fix permissions
   sudo chown -R $USER:$USER .
   chmod +x scripts/*.sh
   ```

### Getting Help

- Check our [FAQ](docs/faq.md)
- Join our [Discord Community](https://discord.gg/cpp-learning-lab)
- Open an [Issue](https://github.com/ghost/cpp-learning-lab/issues)

## Contributing

See our [Contributing Guidelines](../CONTRIBUTING.md) for details on:
- Code style
- Pull request process
- Development workflow

## License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.
