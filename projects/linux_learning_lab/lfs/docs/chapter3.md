# Chapter 3: Building Basic System Software

## Overview
This chapter guides you through building the foundational components of your Linux system, including core utilities, system libraries, and essential tools.

## Learning Objectives
- Build and install core system libraries
- Configure basic system utilities
- Understand system initialization
- Set up basic system structure

## Core System Components

### 1. Glibc (GNU C Library)
- Building the C library
- Configuring locales
- Testing the installation
- Common pitfalls and solutions

### 2. Tool Chain Programs
- GCC (final build)
- Binutils (final build)
- Development tools

### 3. Core Utilities
- Coreutils
- Diffutils
- File utilities
- Findutils
- Grep
- Gzip
- M4
- Make
- Patch
- Perl
- Python
- Sed
- Tar

### 4. Shell and Shell Utilities
- Bash
- Readline
- Ncurses
- Vim

## Practical Exercises

### Exercise 3.1: Building Glibc
```bash
# Extract glibc
tar xf glibc-2.xx.x.tar.xz
cd glibc-2.xx.x

# Create build directory
mkdir -v build
cd build

# Configure
../configure --prefix=/usr                   \
             --disable-werror               \
             --enable-kernel=3.2            \
             --enable-stack-protector=strong \
             --with-headers=/usr/include    \
             libc_cv_slibdir=/lib

# Build and test
make -j$(nproc)
make check

# Install
make install
```

### Exercise 3.2: Configuring Basic System Structure
```bash
# Create essential directories
mkdir -pv /{boot,home,mnt,opt,srv}
mkdir -pv /etc/{opt,sysconfig}
mkdir -pv /lib/firmware
mkdir -pv /media/{floppy,cdrom}
mkdir -pv /usr/{,local/}{include,src}
mkdir -pv /usr/local/{bin,lib,sbin}
mkdir -pv /var/{cache,local,log,mail,opt,spool}
mkdir -pv /var/lib/{color,misc,locate}

# Create symbolic links
ln -sfv /run /var/run
ln -sfv /run/lock /var/lock
```

### Exercise 3.3: Building Core Utilities
```bash
# Build coreutils
tar xf coreutils-8.xx.tar.xz
cd coreutils-8.xx

./configure --prefix=/usr                     \
            --host=$LFS_TGT                   \
            --build=$(build-aux/config.guess) \
            --enable-install-program=hostname  \
            --enable-no-install-program=kill,uptime

make
make DESTDIR=$LFS install
```

## Common Issues and Solutions

### 1. Compilation Errors
- Missing dependencies
- Incorrect paths
- Incompatible versions

### 2. Installation Problems
- Permission issues
- Space constraints
- Path conflicts

### 3. Testing Failures
- Hardware limitations
- Resource constraints
- Configuration issues

## Next Steps
- Configure system bootscripts
- Set up system initialization
- Install boot loader
- Create system configuration files

## Review Questions
1. What is the purpose of Glibc in a Linux system?
2. Why is the order of package installation important?
3. How do you verify successful installation of core utilities?
4. What role do shell utilities play in the system?

## Additional Resources
- [GNU C Library Manual](https://www.gnu.org/software/libc/manual/)
- [Core Utilities Documentation](https://www.gnu.org/software/coreutils/manual/)
- [Bash Reference Manual](https://www.gnu.org/software/bash/manual/)

## Lab Assignments
1. Build and test Glibc with custom optimizations
2. Create a minimal system directory structure
3. Compile and install essential utilities
4. Configure basic system settings

---
Proceed to Chapter 4: System Configuration and Boot Scripts
