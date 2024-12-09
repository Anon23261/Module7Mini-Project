#!/usr/bin/env python3

import os
import sys
import yaml
import logging
from pathlib import Path
from typing import Dict, List
from dataclasses import dataclass

@dataclass
class SystemConfig:
    hostname: str
    timezone: str
    locale: str
    keyboard: str
    users: List[Dict]
    network: Dict
    services: List[str]

class SystemConfigurator:
    def __init__(self, root_dir: str, config: Dict):
        self.root_dir = Path(root_dir)
        self.config = config
        self.logger = logging.getLogger('SystemConfigurator')
        
    def configure_system(self):
        """Configure the base system."""
        self.configure_hostname()
        self.configure_timezone()
        self.configure_locale()
        self.configure_keyboard()
        self.configure_users()
        self.configure_network()
        self.configure_services()
        self.configure_bootloader()
        
    def configure_hostname(self):
        """Set system hostname."""
        hostname = self.config['system']['hostname']
        hostname_file = self.root_dir / 'etc/hostname'
        
        with open(hostname_file, 'w') as f:
            f.write(f"{hostname}\n")
            
    def configure_timezone(self):
        """Set system timezone."""
        timezone = self.config['system']['timezone']
        timezone_file = self.root_dir / 'etc/localtime'
        
        if timezone_file.exists():
            timezone_file.unlink()
            
        timezone_file.symlink_to(f"/usr/share/zoneinfo/{timezone}")
        
    def configure_locale(self):
        """Configure system locale."""
        locale = self.config['system']['locale']
        locale_gen = self.root_dir / 'etc/locale.gen'
        locale_conf = self.root_dir / 'etc/locale.conf'
        
        with open(locale_gen, 'w') as f:
            f.write(f"{locale} UTF-8\n")
            
        with open(locale_conf, 'w') as f:
            f.write(f"LANG={locale}\n")
            
        os.system('locale-gen')
        
    def configure_keyboard(self):
        """Configure keyboard layout."""
        keyboard = self.config['system']['keyboard']
        vconsole_conf = self.root_dir / 'etc/vconsole.conf'
        
        with open(vconsole_conf, 'w') as f:
            f.write(f"KEYMAP={keyboard}\n")
            
    def configure_users(self):
        """Configure system users."""
        for user in self.config['system']['users']:
            username = user['name']
            
            # Create user
            cmd = f"useradd -m -G {','.join(user['groups'])} -s {user['shell']} {username}"
            os.system(cmd)
            
            # Set password
            if 'password_hash' in user:
                cmd = f"chpasswd -e <<< '{username}:{user['password_hash']}'"
                os.system(cmd)
                
            # Configure sudo
            if user.get('sudo', False):
                sudoers_file = self.root_dir / f'etc/sudoers.d/{username}'
                with open(sudoers_file, 'w') as f:
                    f.write(f"{username} ALL=(ALL) ALL\n")
                os.chmod(sudoers_file, 0o440)
                
    def configure_network(self):
        """Configure network settings."""
        network = self.config['system']['network']
        
        if network['type'] == 'dhcp':
            self._configure_dhcp()
        else:
            self._configure_static_ip(network)
            
        # Configure DNS
        resolv_conf = self.root_dir / 'etc/resolv.conf'
        with open(resolv_conf, 'w') as f:
            for nameserver in network['dns_servers']:
                f.write(f"nameserver {nameserver}\n")
                
    def _configure_dhcp(self):
        """Configure DHCP networking."""
        network_dir = self.root_dir / 'etc/systemd/network'
        network_dir.mkdir(parents=True, exist_ok=True)
        
        config_file = network_dir / '20-wired.network'
        with open(config_file, 'w') as f:
            f.write("""[Match]
Name=en*

[Network]
DHCP=yes
""")
            
    def _configure_static_ip(self, network: Dict):
        """Configure static IP networking."""
        network_dir = self.root_dir / 'etc/systemd/network'
        network_dir.mkdir(parents=True, exist_ok=True)
        
        config_file = network_dir / '20-wired.network'
        with open(config_file, 'w') as f:
            f.write(f"""[Match]
Name=en*

[Network]
Address={network['address']}
Gateway={network['gateway']}
""")
            
    def configure_services(self):
        """Enable system services."""
        for service in self.config['system']['services']:
            os.system(f"systemctl enable {service}")
            
    def configure_bootloader(self):
        """Configure system bootloader."""
        os.system("grub-install --target=x86_64-efi --efi-directory=/boot/efi")
        os.system("grub-mkconfig -o /boot/grub/grub.cfg")
        
    def configure_security(self):
        """Configure system security."""
        security = self.config['security']
        
        if security.get('selinux', False):
            self._configure_selinux()
            
        if security.get('apparmor', False):
            self._configure_apparmor()
            
        if security.get('firewall', True):
            self._configure_firewall()
            
    def _configure_selinux(self):
        """Configure SELinux."""
        os.system("selinux-activate")
        
        config_file = self.root_dir / 'etc/selinux/config'
        with open(config_file, 'w') as f:
            f.write("""SELINUX=enforcing
SELINUXTYPE=targeted
""")
            
    def _configure_apparmor(self):
        """Configure AppArmor."""
        os.system("systemctl enable apparmor")
        
    def _configure_firewall(self):
        """Configure system firewall."""
        # Enable firewall service
        os.system("systemctl enable firewalld")
        
        # Configure basic rules
        os.system("firewall-cmd --permanent --zone=public --add-service=ssh")
        os.system("firewall-cmd --permanent --zone=public --add-service=http")
        os.system("firewall-cmd --permanent --zone=public --add-service=https")
        os.system("firewall-cmd --reload")
        
def main():
    if len(sys.argv) != 2:
        print("Usage: system_configurator.py <config_file>")
        sys.exit(1)
        
    with open(sys.argv[1]) as f:
        config = yaml.safe_load(f)
        
    configurator = SystemConfigurator('/', config)
    
    try:
        configurator.configure_system()
        configurator.configure_security()
        
    except Exception as e:
        logging.error(f"Configuration failed: {str(e)}")
        sys.exit(1)
        
if __name__ == '__main__':
    main()
