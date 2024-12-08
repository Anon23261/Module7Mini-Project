#!/bin/bash

# LFS Build Script - Cross Toolchain
set -e

# Check if running as LFS user
if [ "$(whoami)" != "lfs" ]; then
    echo "This script must be run as the 'lfs' user"
    exit 1
fi

# Check if LFS is set
if [ -z "$LFS" ]; then
    echo "LFS environment variable is not set"
    exit 1
fi

# Function to build a package
build_package() {
    local PKG_NAME=$1
    local PKG_VERSION=$2
    local EXTRA_CONFIG=$3

    echo "Building $PKG_NAME-$PKG_VERSION..."
    
    cd $LFS/sources
    tar xf $PKG_NAME-$PKG_VERSION.tar.*
    cd $PKG_NAME-$PKG_VERSION

    # Create build directory if needed
    mkdir -pv build
    cd build

    # Configure
    ../configure $EXTRA_CONFIG

    # Build and install
    make
    make install

    # Cleanup
    cd $LFS/sources
    rm -rf $PKG_NAME-$PKG_VERSION
}

# Build Binutils (Pass 1)
build_binutils() {
    echo "Building Binutils (Pass 1)..."
    build_package "binutils" "2.37" "\
        --prefix=/tools \
        --with-sysroot=$LFS \
        --target=$LFS_TGT \
        --disable-nls \
        --disable-werror"
}

# Build GCC (Pass 1)
build_gcc() {
    echo "Building GCC (Pass 1)..."
    
    # Download prerequisites
    cd $LFS/sources
    tar xf gcc-11.2.0.tar.*
    cd gcc-11.2.0
    ./contrib/download_prerequisites

    build_package "gcc" "11.2.0" "\
        --prefix=/tools \
        --with-sysroot=$LFS \
        --target=$LFS_TGT \
        --disable-nls \
        --disable-shared \
        --disable-multilib \
        --disable-decimal-float \
        --disable-threads \
        --disable-libatomic \
        --disable-libgomp \
        --disable-libquadmath \
        --disable-libssp \
        --disable-libvtv \
        --disable-libstdcxx \
        --enable-languages=c,c++"
}

# Build Linux API Headers
build_linux_headers() {
    echo "Building Linux API Headers..."
    cd $LFS/sources
    tar xf linux-5.13.12.tar.*
    cd linux-5.13.12
    
    make mrproper
    make headers
    find usr/include -name '.*' -delete
    rm usr/include/Makefile
    cp -rv usr/include $LFS/usr
}

# Build Glibc
build_glibc() {
    echo "Building Glibc..."
    build_package "glibc" "2.34" "\
        --prefix=/tools \
        --host=$LFS_TGT \
        --build=$(../scripts/config.guess) \
        --enable-kernel=3.2 \
        --with-headers=$LFS/usr/include \
        libc_cv_slibdir=/tools/lib"
}

# Main build sequence
echo "Starting LFS Cross-Toolchain build..."

# Create necessary directories
mkdir -pv $LFS/tools
mkdir -pv $LFS/sources

# Build the toolchain
build_binutils
build_gcc
build_linux_headers
build_glibc

echo "Cross-Toolchain build completed successfully!"
