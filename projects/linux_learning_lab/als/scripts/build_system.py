#!/usr/bin/env python3

import os
import sys
import yaml
import logging
import subprocess
import multiprocessing
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass
from concurrent.futures import ThreadPoolExecutor

@dataclass
class BuildConfig:
    name: str
    version: str
    target_triplet: str
    parallel_jobs: int
    optimization: str
    debug_info: bool

class SystemBuilder:
    def __init__(self, config_path: str):
        self.config = self._load_config(config_path)
        self.build_config = self._create_build_config()
        self.logger = self._setup_logging()
        
    def _load_config(self, config_path: str) -> Dict:
        with open(config_path) as f:
            return yaml.safe_load(f)
            
    def _create_build_config(self) -> BuildConfig:
        sys_config = self.config['system']
        build_config = self.config['build']
        
        return BuildConfig(
            name=sys_config['name'],
            version=sys_config['version'],
            target_triplet=sys_config['target_triplet'],
            parallel_jobs=build_config['parallel_jobs'],
            optimization=build_config['optimization'],
            debug_info=build_config['debug_info']
        )
        
    def _setup_logging(self) -> logging.Logger:
        log_config = self.config['logging']
        logger = logging.getLogger('SystemBuilder')
        logger.setLevel(log_config['level'].upper())
        
        formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        
        file_handler = logging.FileHandler(log_config['file'])
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)
        
        if log_config['verbose']:
            console_handler = logging.StreamHandler()
            console_handler.setFormatter(formatter)
            logger.addHandler(console_handler)
            
        return logger
        
    def prepare_environment(self):
        """Prepare the build environment."""
        paths = self.config['paths']
        
        # Create necessary directories
        for path in paths.values():
            Path(path).mkdir(parents=True, exist_ok=True)
            
        # Set environment variables
        os.environ['LFS'] = paths['root']
        os.environ['LFS_TGT'] = self.build_config.target_triplet
        os.environ['PATH'] = f"{paths['tools']}/bin:{os.environ['PATH']}"
        
    def build_toolchain(self):
        """Build the cross-compilation toolchain."""
        toolchain = self.config['toolchain']
        
        self.logger.info("Building cross-compilation toolchain")
        
        # Build binutils (pass 1)
        self._build_package({
            'name': 'binutils',
            'version': toolchain['binutils_version'],
            'configure_options': [
                f'--prefix={self.config["paths"]["tools"]}',
                f'--with-sysroot={self.config["paths"]["root"]}',
                f'--target={self.build_config.target_triplet}',
                '--disable-nls',
                '--disable-werror'
            ]
        })
        
        # Build GCC (pass 1)
        self._build_package({
            'name': 'gcc',
            'version': toolchain['gcc_version'],
            'configure_options': [
                f'--prefix={self.config["paths"]["tools"]}',
                f'--target={self.build_config.target_triplet}',
                '--disable-nls',
                '--disable-shared',
                '--without-headers'
            ]
        })
        
    def build_base_system(self):
        """Build the base system packages."""
        self.logger.info("Building base system packages")
        
        packages = self.config['packages']['base']
        with ThreadPoolExecutor(max_workers=self.build_config.parallel_jobs) as executor:
            executor.map(self._build_package, packages)
            
    def build_desktop(self):
        """Build the desktop environment."""
        if not self.config['features']['desktop']['enable_xorg']:
            return
            
        self.logger.info("Building desktop environment")
        
        packages = self.config['packages']['desktop']
        with ThreadPoolExecutor(max_workers=self.build_config.parallel_jobs) as executor:
            executor.map(self._build_package, packages)
            
    def _build_package(self, package: Dict):
        """Build a single package."""
        name = package['name']
        version = package['version']
        
        self.logger.info(f"Building {name}-{version}")
        
        build_dir = Path(self.config['paths']['build']) / f"{name}-{version}"
        build_dir.mkdir(parents=True, exist_ok=True)
        
        try:
            # Extract
            self._extract_source(name, version)
            
            # Configure
            configure_cmd = ['./configure'] + package.get('configure_options', [])
            self._run_command(configure_cmd, build_dir)
            
            # Build
            make_cmd = ['make', f'-j{self.build_config.parallel_jobs}']
            self._run_command(make_cmd, build_dir)
            
            # Install
            self._run_command(['make', 'install'], build_dir)
            
            self.logger.info(f"Successfully built {name}-{version}")
            
        except Exception as e:
            self.logger.error(f"Failed to build {name}-{version}: {str(e)}")
            if package.get('critical', False):
                raise
                
    def _extract_source(self, name: str, version: str):
        """Extract source archive."""
        sources_dir = Path(self.config['paths']['sources'])
        archive = next(sources_dir.glob(f"{name}-{version}.*"))
        
        if archive.suffix == '.gz':
            cmd = ['tar', 'xzf']
        elif archive.suffix == '.xz':
            cmd = ['tar', 'xJf']
        elif archive.suffix == '.bz2':
            cmd = ['tar', 'xjf']
        else:
            raise ValueError(f"Unsupported archive format: {archive.suffix}")
            
        self._run_command(cmd + [str(archive)])
        
    def _run_command(self, cmd: List[str], cwd: Optional[Path] = None):
        """Run a shell command."""
        result = subprocess.run(
            cmd,
            cwd=str(cwd) if cwd else None,
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0:
            raise RuntimeError(
                f"Command failed with exit code {result.returncode}:\n"
                f"Command: {' '.join(cmd)}\n"
                f"Output: {result.stdout}\n"
                f"Error: {result.stderr}"
            )
            
def main():
    if len(sys.argv) != 2:
        print("Usage: build_system.py <config_file>")
        sys.exit(1)
        
    builder = SystemBuilder(sys.argv[1])
    
    try:
        builder.prepare_environment()
        builder.build_toolchain()
        builder.build_base_system()
        builder.build_desktop()
        
    except Exception as e:
        builder.logger.error(f"Build failed: {str(e)}")
        sys.exit(1)
        
if __name__ == '__main__':
    main()
