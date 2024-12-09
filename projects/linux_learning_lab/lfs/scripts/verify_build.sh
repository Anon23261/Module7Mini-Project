#!/bin/bash

# LFS Build Verification Script
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# Function to check if a command exists
check_command() {
    local cmd=$1
    local expected_version=$2
    
    if command -v $cmd >/dev/null 2>&1; then
        version=$($cmd --version 2>&1 | head -n1)
        echo -e "${GREEN}✓${NC} Found $cmd: $version"
        
        if [ ! -z "$expected_version" ]; then
            if echo "$version" | grep -q "$expected_version"; then
                echo -e "  ${GREEN}✓${NC} Version matches expected ($expected_version)"
            else
                echo -e "  ${RED}✗${NC} Version mismatch (expected: $expected_version)"
                return 1
            fi
        fi
    else
        echo -e "${RED}✗${NC} $cmd not found"
        return 1
    fi
}

# Function to check library exists
check_library() {
    local lib=$1
    if [ -f "$lib" ]; then
        echo -e "${GREEN}✓${NC} Found library: $lib"
    else
        echo -e "${RED}✗${NC} Missing library: $lib"
        return 1
    fi
}

# Check environment
echo "Checking build environment..."
echo "-------------------------"

# Check LFS variable
if [ -z "$LFS" ]; then
    echo -e "${RED}✗${NC} LFS environment variable not set"
    exit 1
else
    echo -e "${GREEN}✓${NC} LFS environment variable set to: $LFS"
fi

# Check LFS_TGT variable
if [ -z "$LFS_TGT" ]; then
    echo -e "${RED}✗${NC} LFS_TGT environment variable not set"
    exit 1
else
    echo -e "${GREEN}✓${NC} LFS_TGT environment variable set to: $LFS_TGT"
fi

# Check critical directories
echo -e "\nChecking directories..."
echo "-------------------------"
directories=(
    "$LFS/tools"
    "$LFS/sources"
    "$LFS/boot"
    "$LFS/etc"
    "$LFS/bin"
    "$LFS/lib"
    "$LFS/sbin"
    "$LFS/usr"
    "$LFS/var"
)

for dir in "${directories[@]}"; do
    if [ -d "$dir" ]; then
        echo -e "${GREEN}✓${NC} Directory exists: $dir"
    else
        echo -e "${RED}✗${NC} Missing directory: $dir"
        exit 1
    fi
done

# Check toolchain
echo -e "\nChecking toolchain..."
echo "-------------------------"
check_command gcc "11.2.0"
check_command ld "2.37"
check_command make "4.3"

# Check critical tools
echo -e "\nChecking build tools..."
echo "-------------------------"
tools=(
    "bash"
    "binutils"
    "bison"
    "bzip2"
    "coreutils"
    "diffutils"
    "findutils"
    "gawk"
    "gcc"
    "glibc"
    "grep"
    "gzip"
    "m4"
    "make"
    "patch"
    "perl"
    "python3"
    "sed"
    "tar"
    "texinfo"
    "xz"
)

for tool in "${tools[@]}"; do
    check_command $tool
done

# Verify basic compilation
echo -e "\nTesting basic compilation..."
echo "-------------------------"

# Create test directory
TEST_DIR=$(mktemp -d)
cd $TEST_DIR

# Write a simple C program
cat > test.c << "EOF"
#include <stdio.h>
int main() {
    printf("Hello from LFS!\n");
    return 0;
}
EOF

# Try to compile it
if gcc test.c -o test; then
    if ./test | grep -q "Hello from LFS!"; then
        echo -e "${GREEN}✓${NC} Basic compilation test passed"
    else
        echo -e "${RED}✗${NC} Program compiled but produced unexpected output"
        exit 1
    fi
else
    echo -e "${RED}✗${NC} Compilation test failed"
    exit 1
fi

# Cleanup
rm -rf $TEST_DIR

echo -e "\nVerification complete!"
