# Chapter 2 Labs: Building the Cross-Toolchain

## Lab 2.1: Building Binutils (Pass 1)

### Objective
Build the first pass of Binutils for cross-compilation.

### Tasks
1. Extract and prepare Binutils:
```bash
cd $LFS/sources
tar xf binutils-2.38.tar.xz
cd binutils-2.38
mkdir -v build
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

### Deliverables
- Build logs
- Installation verification
- Test results documentation

## Lab 2.2: Building GCC (Pass 1)

### Objective
Build the first pass of GCC for cross-compilation.

### Tasks
1. Extract and prepare GCC:
```bash
cd $LFS/sources
tar xf gcc-11.2.0.tar.xz
cd gcc-11.2.0

# Extract required packages
tar -xf ../mpfr-4.1.0.tar.xz
mv -v mpfr-4.1.0 mpfr
tar -xf ../gmp-6.2.1.tar.xz
mv -v gmp-6.2.1 gmp
tar -xf ../mpc-1.2.1.tar.gz
mv -v mpc-1.2.1 mpc

# Create build directory
mkdir -v build
cd build

# Configure
../configure                  \
    --target=$LFS_TGT         \
    --prefix=$LFS/tools       \
    --with-glibc-version=2.35 \
    --with-sysroot=$LFS       \
    --with-newlib             \
    --without-headers         \
    --enable-initfini-array   \
    --disable-nls             \
    --disable-shared          \
    --disable-multilib        \
    --disable-decimal-float   \
    --disable-threads         \
    --disable-libatomic       \
    --disable-libgomp         \
    --disable-libquadmath     \
    --disable-libssp          \
    --disable-libvtv          \
    --disable-libstdcxx       \
    --enable-languages=c,c++

# Build and install
make
make install
```

### Deliverables
- Compilation logs
- Installation verification
- Cross-compiler testing results

## Lab 2.3: Linux API Headers

### Objective
Install the Linux API Headers for cross-compilation.

### Tasks
1. Prepare and install headers:
```bash
cd $LFS/sources
tar xf linux-5.16.9.tar.xz
cd linux-5.16.9

# Clean the kernel tree
make mrproper

# Extract API headers
make headers
find usr/include -name '.*' -delete
rm usr/include/Makefile
cp -rv usr/include $LFS/usr
```

### Deliverables
- Header installation verification
- Directory structure documentation
- API headers test results

## Lab 2.4: Glibc

### Objective
Build Glibc for the cross-toolchain.

### Tasks
1. Build and install Glibc:
```bash
cd $LFS/sources
tar xf glibc-2.35.tar.xz
cd glibc-2.35

# Create build directory
mkdir -v build
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

### Deliverables
- Build logs
- Test suite results
- Installation verification

## Additional Challenges
1. Create automated build scripts
2. Implement error handling and recovery
3. Add detailed logging and reporting
4. Create a validation suite for the toolchain

## Troubleshooting Guide
1. Common compilation errors
2. Library dependencies issues
3. Path and environment problems
4. Build system conflicts

## Testing Framework
```bash
#!/bin/bash
# Test script for cross-toolchain

# Test C compilation
echo 'int main(){}' > dummy.c
$LFS_TGT-gcc dummy.c
readelf -l a.out | grep '/ld-linux'

# Test basic C functionality
echo 'int main(){return 0;}' > test.c
$LFS_TGT-gcc test.c -o test
./test
echo $?

# Clean up
rm -f dummy.c a.out test.c test
```

## Documentation Requirements
1. Build logs for each component
2. Test results and validation
3. Environment configuration
4. System state verification
