# Chapter 4: System Configuration and Boot Scripts

## Overview
This chapter covers system initialization, boot scripts, and essential system configuration. You'll learn how to set up a proper boot process and configure core system services.

## Learning Objectives
- Understand the Linux boot process
- Create and configure init scripts
- Set up system logging
- Configure network services
- Implement system security

## System Initialization

### 1. The Boot Process
- BIOS/UEFI initialization
- Bootloader (GRUB) configuration
- Kernel loading and initialization
- Init system startup

### 2. Init Systems
#### SysVinit
```bash
# Basic init configuration
cat > /etc/inittab << "EOF"
# Begin /etc/inittab

id:3:initdefault:

si::sysinit:/etc/rc.d/init.d/rc S

l0:0:wait:/etc/rc.d/init.d/rc 0
l1:S1:wait:/etc/rc.d/init.d/rc 1
l2:2:wait:/etc/rc.d/init.d/rc 2
l3:3:wait:/etc/rc.d/init.d/rc 3
l4:4:wait:/etc/rc.d/init.d/rc 4
l5:5:wait:/etc/rc.d/init.d/rc 5
l6:6:wait:/etc/rc.d/init.d/rc 6

ca:12345:ctrlaltdel:/sbin/shutdown -t1 -a -r now

su:S016:once:/sbin/sulogin

1:2345:respawn:/sbin/agetty --noclear tty1 9600
2:2345:respawn:/sbin/agetty tty2 9600
3:2345:respawn:/sbin/agetty tty3 9600
4:2345:respawn:/sbin/agetty tty4 9600
5:2345:respawn:/sbin/agetty tty5 9600
6:2345:respawn:/sbin/agetty tty6 9600

# End /etc/inittab
EOF
```

#### Systemd
```bash
# Basic systemd configuration
mkdir -pv /etc/systemd/system
cat > /etc/systemd/system/default.target << "EOF"
[Unit]
Description=Multi-User System
Documentation=man:systemd.special(7)
Requires=basic.target
Conflicts=rescue.service rescue.target
After=basic.target rescue.service rescue.target
AllowIsolate=yes
EOF
```

## Boot Scripts

### 1. Basic Boot Scripts
```bash
#!/bin/bash
# /etc/rc.d/init.d/mountfs

. /lib/lsb/init-functions

case "${1}" in
   start)
      log_info_msg "Mounting root file system in read-write mode..."
      mount -o remount,rw / >/dev/null
      evaluate_retval

      # Remove fsck-related file system watermarks.
      rm -f /fastboot /forcefsck
      ;;

   stop)
      log_info_msg "Mounting root file system in read-only mode..."
      mount -o remount,ro / >/dev/null
      evaluate_retval
      ;;

   *)
      echo "Usage: ${0} {start|stop}"
      exit 1
      ;;
esac

exit 0
```

### 2. Network Configuration
```bash
# Basic network interface configuration
cat > /etc/sysconfig/ifconfig.eth0 << "EOF"
ONBOOT=yes
IFACE=eth0
SERVICE=ipv4-static
IP=192.168.1.2
GATEWAY=192.168.1.1
PREFIX=24
BROADCAST=192.168.1.255
EOF
```

## System Services

### 1. System Logging
```bash
# Configure syslog
cat > /etc/syslog.conf << "EOF"
# Begin /etc/syslog.conf

auth,authpriv.* -/var/log/auth.log
*.*;auth,authpriv.none -/var/log/sys.log
daemon.* -/var/log/daemon.log
kern.* -/var/log/kern.log
mail.* -/var/log/mail.log
user.* -/var/log/user.log
*.emerg *

# End /etc/syslog.conf
EOF
```

### 2. Cron Service
```bash
# Set up basic cron configuration
mkdir -pv /etc/cron.{hourly,daily,weekly,monthly}
cat > /etc/crontab << "EOF"
# Begin /etc/crontab

SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root
HOME=/

# Run hourly cron jobs
01 * * * * root run-parts /etc/cron.hourly

# Run daily cron jobs
02 4 * * * root run-parts /etc/cron.daily

# Run weekly cron jobs
22 4 * * 0 root run-parts /etc/cron.weekly

# Run monthly cron jobs
42 4 1 * * root run-parts /etc/cron.monthly

# End /etc/crontab
EOF
```

## System Security

### 1. Basic Security Configuration
```bash
# Configure login security
cat > /etc/security/limits.conf << "EOF"
# Begin /etc/security/limits.conf

* soft core 0
* hard core 0
root soft core unlimited
root hard core unlimited

# End /etc/security/limits.conf
EOF
```

### 2. Password Security
```bash
# Configure password policies
cat > /etc/login.defs << "EOF"
# Begin /etc/login.defs

PASS_MAX_DAYS   99999
PASS_MIN_DAYS   0
PASS_MIN_LEN    5
PASS_WARN_AGE   7

# End /etc/login.defs
EOF
```

## Bootloader Configuration

### GRUB Configuration
```bash
# Install GRUB
grub-install /dev/sda

# Configure GRUB
cat > /boot/grub/grub.cfg << "EOF"
# Begin /boot/grub/grub.cfg

set default=0
set timeout=5

insmod ext2
set root=(hd0,1)

menuentry "Linux From Scratch" {
        linux   /boot/vmlinuz-5.16.9-lfs-11.1 root=/dev/sda1 ro
}
EOF
```

## Practical Exercises

### Exercise 4.1: Boot Script Creation
Create a custom boot script that:
1. Mounts all filesystems
2. Initializes network interfaces
3. Starts essential services

### Exercise 4.2: System Service Configuration
Configure and test:
1. System logging
2. Cron jobs
3. Network services

### Exercise 4.3: Security Hardening
Implement:
1. Password policies
2. Resource limits
3. Access controls

## Common Issues and Solutions

### 1. Boot Problems
- Bootloader configuration errors
- Missing kernel modules
- Filesystem mount issues

### 2. Service Issues
- Service dependencies
- Permission problems
- Resource conflicts

### 3. Network Problems
- Interface configuration
- DNS resolution
- Routing issues

## Next Steps
- Configure additional system services
- Set up user environment
- Install additional software
- System optimization

## Review Questions
1. Explain the Linux boot process stages
2. How do init systems manage services?
3. What are the key security considerations?
4. How do you troubleshoot boot problems?

## Additional Resources
- [Linux Documentation Project](http://tldp.org/)
- [Systemd Documentation](https://www.freedesktop.org/wiki/Software/systemd/)
- [GRUB Manual](https://www.gnu.org/software/grub/manual/)

---
Proceed to Chapter 5: System Configuration and Package Management
