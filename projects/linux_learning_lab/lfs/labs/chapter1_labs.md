# Chapter 1 Labs: Introduction to Linux From Scratch

## Lab 1.1: Host System Preparation

### Objective
Set up and verify the host system environment for LFS building.

### Tasks
1. Check host system requirements:
```bash
# Version check script
cat > version-check.sh << "EOF"
#!/bin/bash
# Simple script to check system requirements
export LC_ALL=C

# Bash version check
bash --version | head -n1 | cut -d" " -f2-4

# Binutils version check
ld --version | head -n1 | cut -d" " -f3-

# Bison version check
bison --version | head -n1

# Check for /usr/bin/yacc
if [ -h /usr/bin/yacc ]; then
  echo "/usr/bin/yacc -> `readlink -f /usr/bin/yacc`";
elif [ -x /usr/bin/yacc ]; then
  echo yacc is `/usr/bin/yacc --version | head -n1`
else
  echo "yacc not found" 
fi

# GCC version check
gcc --version | head -n1

# Check for g++
g++ --version | head -n1

# Check for make, patch, m4
make --version | head -n1
patch --version | head -n1
m4 --version | head -n1

# Python version check
python3 --version
EOF

chmod +x version-check.sh
./version-check.sh
```

2. Create the LFS partition:
```bash
# Create a new partition (example using fdisk)
sudo fdisk /dev/sdX

# Format the partition
mkfs -v -t ext4 /dev/sdX1

# Set LFS variable
export LFS=/mnt/lfs

# Create mount point and mount partition
sudo mkdir -pv $LFS
sudo mount -v -t ext4 /dev/sdX1 $LFS
```

3. Create source directory:
```bash
sudo mkdir -v $LFS/sources
sudo chmod -v a+wt $LFS/sources
```

### Deliverables
- Screenshot of version-check.sh output
- Partition layout documentation
- Mount point configuration

## Lab 1.2: Package Management

### Objective
Download and verify source packages for LFS.

### Tasks
1. Create package list:
```bash
# Create wget-list
cat > wget-list << "EOF"
https://download.savannah.gnu.org/releases/acl/acl-2.3.1.tar.xz
https://download.savannah.gnu.org/releases/attr/attr-2.5.1.tar.gz
https://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.xz
https://ftp.gnu.org/gnu/automake/automake-1.16.5.tar.xz
https://ftp.gnu.org/gnu/bash/bash-5.1.16.tar.gz
https://github.com/gavinhoward/bc/releases/download/5.2.2/bc-5.2.2.tar.xz
https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz
https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz
EOF

# Download packages
wget --input-file=wget-list --continue --directory-prefix=$LFS/sources
```

2. Verify package integrity:
```bash
# Create MD5SUMS file
cat > md5sums << "EOF"
95ce715fe09acca7c12d3306d0f076b2  acl-2.3.1.tar.xz
ac1c5a7a084f0f83b8cace34211f64d8  attr-2.5.1.tar.gz
12cfa1687ffa2606337efe1a64416106  autoconf-2.71.tar.xz
4017e96f89fca45ca946f1c5db6be714  automake-1.16.5.tar.xz
c17b20a09fc38d67fb303aeb6c130b4e  bash-5.1.16.tar.gz
632344cdb052af0e06087bd3b0034126  bc-5.2.2.tar.xz
6e39cad1bb414add02b5b1169c18fdc5  binutils-2.38.tar.xz
c28f119f405a2304ff0a7ccdcc629713  bison-3.8.2.tar.xz
EOF

# Verify packages
pushd $LFS/sources
  md5sum -c md5sums
popd
```

### Deliverables
- Complete package download log
- MD5 verification results
- Package organization structure

## Lab 1.3: Build Environment Setup

### Objective
Create and configure the build environment.

### Tasks
1. Create LFS user:
```bash
# Create lfs user
sudo groupadd lfs
sudo useradd -s /bin/bash -g lfs -m -k /dev/null lfs
sudo passwd lfs

# Grant lfs full access to $LFS
sudo chown -v lfs $LFS/{usr{,/*},lib,var,etc,bin,sbin,tools}
sudo chown -v lfs $LFS/sources
```

2. Set up build environment:
```bash
# Create .bash_profile
cat > ~/.bash_profile << "EOF"
exec env -i HOME=$HOME TERM=$TERM PS1='\u:\w\$ ' /bin/bash
EOF

# Create .bashrc
cat > ~/.bashrc << "EOF"
set +h
umask 022
LFS=/mnt/lfs
LC_ALL=POSIX
LFS_TGT=$(uname -m)-lfs-linux-gnu
PATH=/usr/bin
if [ ! -L /bin ]; then PATH=/bin:$PATH; fi
PATH=$LFS/tools/bin:$PATH
CONFIG_SITE=$LFS/usr/share/config.site
export LFS LC_ALL LFS_TGT PATH CONFIG_SITE
EOF
```

### Deliverables
- User environment configuration files
- Environment variable verification
- Directory permission documentation

## Additional Challenges
1. Create a script to automate the entire environment setup
2. Implement error checking in the setup scripts
3. Create a backup/restore system for the build environment
