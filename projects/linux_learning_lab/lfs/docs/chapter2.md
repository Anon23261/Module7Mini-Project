# Chapter 2: Building the Cross-Toolchain

## Overview
This chapter guides you through building the cross-compilation toolchain, which is essential for creating a clean and independent Linux system.

## Learning Objectives
- Understand cross-compilation concepts
- Build a complete cross-compilation toolchain
- Learn about library dependencies
- Understand the role of system headers

## Cross-Compilation Concepts

### What is Cross-Compilation?
Cross-compilation allows you to build executables for a different system (target) than the one you're building on (host). Benefits include:
- Clean build environment
- Platform independence
- Multiple architecture support

### Toolchain Components
1. Binutils (assembler and linker)
2. GCC (compiler)
3. Linux API Headers
4. Glibc (C library)

## Building the Toolchain

### 1. Binutils (Pass 1)
```bash
# Extract binutils
cd $LFS/sources
tar xf binutils-2.37.tar.xz
cd binutils-2.37

# Create build directory
mkdir build
cd build

# Configure
../configure --prefix=$LFS/tools \
             --with-sysroot=$LFS \
             --target=$LFS_TGT   \
             --disable-nls       \
             --disable-werror

# Build and install
make
make install
```

### 2. GCC (Pass 1)
```bash
# Extract GCC
cd $LFS/sources
tar xf gcc-11.2.0.tar.xz
cd gcc-11.2.0

# Download prerequisites
./contrib/download_prerequisites

# Create build directory
mkdir build
cd build

# Configure
../configure                                       \
    --target=$LFS_TGT                         \
    --prefix=$LFS/tools                       \
    --with-glibc-version=2.11                 \
    --with-sysroot=$LFS                       \
    --with-newlib                             \
    --without-headers                         \
    --enable-initfini-array                   \
    --disable-nls                             \
    --disable-shared                          \
    --disable-multilib                        \
    --disable-decimal-float                   \
    --disable-threads                         \
    --disable-libatomic                       \
    --disable-libgomp                         \
    --disable-libquadmath                     \
    --disable-libssp                          \
    --disable-libvtv                          \
    --disable-libstdcxx                       \
    --enable-languages=c,c++

# Build and install
make
make install
```

### 3. Linux API Headers
```bash
# Extract Linux kernel
cd $LFS/sources
tar xf linux-5.13.12.tar.xz
cd linux-5.13.12

# Clean the kernel tree
make mrproper

# Extract API headers
make headers
find usr/include -name '.*' -delete
rm usr/include/Makefile
cp -rv usr/include $LFS/usr
```

### 4. Glibc
```bash
# Extract Glibc
cd $LFS/sources
tar xf glibc-2.34.tar.xz
cd glibc-2.34

# Create build directory
mkdir build
cd build

# Configure
../configure                             \
      --prefix=/usr                      \
      --host=$LFS_TGT                    \
      --build=$(../scripts/config.guess) \
      --enable-kernel=3.2                \
      --with-headers=$LFS/usr/include    \
      libc_cv_slibdir=/lib

# Build and install
make
make DESTDIR=$LFS install
```

## Verifying the Toolchain

### Testing GCC
```bash
echo 'int main(){}' > dummy.c
$LFS_TGT-gcc dummy.c
readelf -l a.out | grep '/ld-linux'
```

### Testing Basic Functions
```bash
# Create test program
cat > test.c << "EOF"
#include <stdio.h>
int main() {
    printf("Hello from LFS!\n");
    return 0;
}
EOF

# Compile and run
$LFS_TGT-gcc test.c -o test
./test
```

## Common Issues and Solutions

### 1. Missing Prerequisites
Problem: Configure scripts fail
Solution: Install development packages
```bash
sudo apt-get install build-essential bison gawk texinfo
```

### 2. Version Mismatches
Problem: Incompatible tool versions
Solution: Verify versions match LFS book
```bash
gcc --version
ld --version
```

### 3. Path Issues
Problem: Wrong tools being used
Solution: Verify PATH setting
```bash
echo $PATH
which gcc
```

## Knowledge Check
1. Why do we build binutils twice?
2. What is the purpose of the Linux API headers?
3. Why do we disable certain features in GCC?
4. What role does Glibc play in the system?

## Lab Exercises

### Exercise 1: Toolchain Verification
Create a program that uses various C library functions and compile it with your new toolchain.

### Exercise 2: Cross-Compilation Practice
Try compiling a simple program for a different architecture.

### Exercise 3: Library Dependencies
Analyze the dependencies of a compiled program using `ldd` and `readelf`.

## Additional Resources
- [GCC Documentation](https://gcc.gnu.org/onlinedocs/)
- [Binutils Documentation](https://sourceware.org/binutils/docs/)
- [Glibc Documentation](https://www.gnu.org/software/libc/manual/)

## Next Steps
- Review toolchain build logs
- Verify all components work correctly
- Proceed to Chapter 3: Building Basic System Software
