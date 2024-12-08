# Chapter 4 Labs: System Configuration and Boot Scripts

## Lab 4.1: Boot Process Configuration

### Objective
Set up and configure a complete boot process using GRUB and init scripts.

### Tasks
1. Install and configure GRUB:
```bash
# Install GRUB
grub-install /dev/sda

# Create GRUB configuration
cat > /boot/grub/grub.cfg << "EOF"
# Begin /boot/grub/grub.cfg
set default=0
set timeout=5

insmod ext2
set root=(hd0,1)

menuentry "LFS Development" {
        linux   /boot/vmlinuz-5.16.9-lfs root=/dev/sda1 ro quiet
}

menuentry "LFS Development (Recovery Mode)" {
        linux   /boot/vmlinuz-5.16.9-lfs root=/dev/sda1 ro single
}
EOF
```

2. Create basic init scripts:
```bash
#!/bin/bash
# /etc/rc.d/init.d/template

### BEGIN INIT INFO
# Provides:            template
# Required-Start:      $local_fs $network
# Required-Stop:       $local_fs $network
# Default-Start:       2 3 4 5
# Default-Stop:        0 1 6
# Short-Description:   Template init script
### END INIT INFO

. /lib/lsb/init-functions

case "${1}" in
   start)
      log_info_msg "Starting..."
      # Add start commands here
      evaluate_retval
      ;;

   stop)
      log_info_msg "Stopping..."
      # Add stop commands here
      evaluate_retval
      ;;

   restart)
      ${0} stop
      sleep 1
      ${0} start
      ;;

   *)
      echo "Usage: ${0} {start|stop|restart}"
      exit 1
      ;;
esac

exit 0
```

### Deliverables
- GRUB configuration file
- Set of basic init scripts
- Boot process documentation

## Lab 4.2: System Service Configuration

### Objective
Configure essential system services and logging.

### Tasks
1. Configure system logging:
```bash
# Create syslog configuration
cat > /etc/syslog.conf << "EOF"
# System logging configuration
auth,authpriv.*                 /var/log/auth.log
*.*;auth,authpriv.none         /var/log/syslog
daemon.*                       /var/log/daemon.log
kern.*                         /var/log/kern.log
mail.*                         /var/log/mail.log
user.*                         /var/log/user.log
*.=info;*.=notice;*.=warn     /var/log/messages
*.emerg                       *
EOF

# Create log rotation configuration
cat > /etc/logrotate.d/syslog << "EOF"
/var/log/syslog
/var/log/mail.log
/var/log/kern.log
/var/log/auth.log
/var/log/user.log
/var/log/daemon.log
/var/log/messages
{
        rotate 7
        daily
        missingok
        notifempty
        compress
        delaycompress
        sharedscripts
        postrotate
                /bin/kill -HUP `cat /var/run/syslogd.pid 2>/dev/null` 2>/dev/null || true
        endscript
}
EOF
```

2. Set up cron jobs:
```bash
# Create system maintenance script
cat > /etc/cron.daily/system-maintenance << "EOF"
#!/bin/bash

# Clean /tmp directory
find /tmp -type f -atime +7 -delete

# Update locate database
updatedb

# Check disk space
df -h | mail -s "Disk Usage Report" root
EOF
chmod +x /etc/cron.daily/system-maintenance
```

### Deliverables
- Logging configuration
- Cron job scripts
- Service status documentation

## Lab 4.3: Network Configuration

### Objective
Set up and test network configuration.

### Tasks
1. Configure network interfaces:
```bash
# Create network interface configuration
cat > /etc/sysconfig/ifconfig.eth0 << "EOF"
ONBOOT="yes"
IFACE="eth0"
SERVICE="ipv4-static"
IP="192.168.1.2"
GATEWAY="192.168.1.1"
PREFIX="24"
BROADCAST="192.168.1.255"
EOF

# Create DNS configuration
cat > /etc/resolv.conf << "EOF"
nameserver 8.8.8.8
nameserver 8.8.4.4
EOF
```

2. Create network testing script:
```bash
#!/bin/bash
# /usr/local/bin/network-test

echo "Testing network configuration..."

# Test local interface
echo "Testing local interface..."
ip addr show eth0

# Test routing
echo "Testing routing..."
ip route show

# Test DNS resolution
echo "Testing DNS resolution..."
ping -c 3 google.com

# Test connectivity
echo "Testing connectivity..."
wget -q --spider http://google.com

if [ $? -eq 0 ]; then
    echo "Network is functioning correctly"
else
    echo "Network test failed"
fi
```

### Deliverables
- Network configuration files
- Test results
- Troubleshooting documentation

## Lab 4.4: Security Configuration

### Objective
Implement basic system security measures.

### Tasks
1. Configure system security limits:
```bash
# Create security limits configuration
cat > /etc/security/limits.conf << "EOF"
# System security limits
*               soft    core            0
*               hard    core            0
*               soft    nproc           50
*               hard    nproc           100
root            soft    nproc           unlimited
root            hard    nproc           unlimited
*               soft    nofile          1024
*               hard    nofile          2048
EOF
```

2. Create security audit script:
```bash
#!/bin/bash
# /usr/local/sbin/security-audit

echo "Running security audit..."

# Check for SUID/SGID files
echo "Checking for SUID/SGID files..."
find / -type f \( -perm -4000 -o -perm -2000 \) -exec ls -l {} \;

# Check open ports
echo "Checking open ports..."
netstat -tuln

# Check running processes
echo "Checking running processes..."
ps aux

# Check login attempts
echo "Checking failed login attempts..."
grep "Failed password" /var/log/auth.log

# Output report
echo "Security audit complete. Check output for potential issues."
```

### Deliverables
- Security configuration files
- Audit script results
- Security recommendations

## Additional Challenges
1. Create an automated system initialization script
2. Implement service monitoring
3. Create a backup solution
4. Implement intrusion detection

## Testing Framework
```bash
#!/bin/bash
# Test framework for system configuration

# Test boot configuration
echo "Testing boot configuration..."
test -f /boot/grub/grub.cfg || echo "GRUB configuration missing"
grep "menuentry" /boot/grub/grub.cfg || echo "No boot entries found"

# Test system services
echo "Testing system services..."
test -d /etc/rc.d/init.d || echo "Init directory missing"
test -f /etc/inittab || echo "Inittab missing"

# Test network configuration
echo "Testing network configuration..."
ip link show || echo "Network interface error"
ping -c 1 8.8.8.8 || echo "Network connectivity error"

# Test security configuration
echo "Testing security configuration..."
test -f /etc/security/limits.conf || echo "Security limits missing"
test -f /etc/login.defs || echo "Login definitions missing"
```

## Documentation Requirements
1. System configuration documentation
2. Network topology diagram
3. Service dependency chart
4. Security policy document
