#!/usr/bin/env python3

import os
import hashlib
import requests
import tarfile
import logging
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass

@dataclass
class Package:
    name: str
    version: str
    dependencies: List[str]
    build_dependencies: List[str]
    source_url: str
    checksum: str

class PackageManager:
    def __init__(self, root_dir: str, config: Dict):
        self.root_dir = Path(root_dir)
        self.config = config
        self.logger = logging.getLogger('PackageManager')
        self.db_path = self.root_dir / 'var/lib/pkg/db'
        self.installed_packages = self._load_installed_packages()
        
    def _load_installed_packages(self) -> Dict[str, str]:
        """Load database of installed packages."""
        if not self.db_path.exists():
            return {}
            
        packages = {}
        with open(self.db_path) as f:
            for line in f:
                name, version = line.strip().split(':')
                packages[name] = version
                
        return packages
        
    def _save_installed_packages(self):
        """Save database of installed packages."""
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(self.db_path, 'w') as f:
            for name, version in self.installed_packages.items():
                f.write(f"{name}:{version}\n")
                
    def download_package(self, package: Package) -> Path:
        """Download package source."""
        sources_dir = self.root_dir / 'sources'
        sources_dir.mkdir(parents=True, exist_ok=True)
        
        archive_path = sources_dir / f"{package.name}-{package.version}.tar.gz"
        
        if not archive_path.exists():
            self.logger.info(f"Downloading {package.name}-{package.version}")
            response = requests.get(package.source_url, stream=True)
            response.raise_for_status()
            
            with open(archive_path, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
                    
        # Verify checksum
        calculated_checksum = self._calculate_checksum(archive_path)
        if calculated_checksum != package.checksum:
            raise ValueError(
                f"Checksum mismatch for {package.name}-{package.version}\n"
                f"Expected: {package.checksum}\n"
                f"Got: {calculated_checksum}"
            )
            
        return archive_path
        
    def _calculate_checksum(self, file_path: Path) -> str:
        """Calculate SHA256 checksum of a file."""
        sha256 = hashlib.sha256()
        
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(8192), b''):
                sha256.update(chunk)
                
        return sha256.hexdigest()
        
    def install_package(self, package: Package):
        """Install a package."""
        if package.name in self.installed_packages:
            if self.installed_packages[package.name] == package.version:
                self.logger.info(f"{package.name}-{package.version} already installed")
                return
                
        # Install dependencies
        for dep in package.dependencies:
            if dep not in self.installed_packages:
                raise RuntimeError(f"Missing dependency: {dep}")
                
        # Download and verify
        archive_path = self.download_package(package)
        
        # Extract
        build_dir = self.root_dir / 'build' / f"{package.name}-{package.version}"
        build_dir.mkdir(parents=True, exist_ok=True)
        
        with tarfile.open(archive_path) as tar:
            tar.extractall(path=build_dir)
            
        # Build and install
        self._build_package(package, build_dir)
        
        # Update database
        self.installed_packages[package.name] = package.version
        self._save_installed_packages()
        
    def _build_package(self, package: Package, build_dir: Path):
        """Build and install a package."""
        os.chdir(build_dir)
        
        # Configure
        configure_cmd = ['./configure',
                        f'--prefix={self.root_dir}/usr',
                        f'--sysconfdir={self.root_dir}/etc']
        
        result = os.system(' '.join(configure_cmd))
        if result != 0:
            raise RuntimeError(f"Configure failed for {package.name}")
            
        # Build
        result = os.system(f"make -j{os.cpu_count()}")
        if result != 0:
            raise RuntimeError(f"Build failed for {package.name}")
            
        # Install
        result = os.system('make install')
        if result != 0:
            raise RuntimeError(f"Installation failed for {package.name}")
            
    def remove_package(self, package_name: str):
        """Remove an installed package."""
        if package_name not in self.installed_packages:
            self.logger.warning(f"{package_name} is not installed")
            return
            
        # Check if any other packages depend on this one
        for name, pkg in self.installed_packages.items():
            if name == package_name:
                continue
                
            if package_name in pkg.dependencies:
                raise RuntimeError(
                    f"Cannot remove {package_name}: {name} depends on it"
                )
                
        # Remove files
        install_dir = self.root_dir / 'usr'
        manifest_file = self.root_dir / 'var/lib/pkg/info' / f"{package_name}.files"
        
        if manifest_file.exists():
            with open(manifest_file) as f:
                for line in f:
                    file_path = install_dir / line.strip()
                    if file_path.exists():
                        file_path.unlink()
                        
            manifest_file.unlink()
            
        # Update database
        del self.installed_packages[package_name]
        self._save_installed_packages()
        
    def upgrade_package(self, package: Package):
        """Upgrade an installed package."""
        if package.name not in self.installed_packages:
            raise RuntimeError(f"{package.name} is not installed")
            
        current_version = self.installed_packages[package.name]
        if current_version == package.version:
            self.logger.info(f"{package.name} is already at version {package.version}")
            return
            
        self.remove_package(package.name)
        self.install_package(package)
        
    def list_installed(self) -> List[str]:
        """List all installed packages."""
        return [f"{name}-{version}"
                for name, version in self.installed_packages.items()]
                
    def get_package_info(self, package_name: str) -> Optional[Dict]:
        """Get information about an installed package."""
        if package_name not in self.installed_packages:
            return None
            
        version = self.installed_packages[package_name]
        info_file = self.root_dir / 'var/lib/pkg/info' / f"{package_name}.info"
        
        if not info_file.exists():
            return {'name': package_name, 'version': version}
            
        with open(info_file) as f:
            info = eval(f.read())
            
        return info
