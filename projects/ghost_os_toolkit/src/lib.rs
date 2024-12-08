pub mod arm;
pub mod windows;
pub mod common;

use anyhow::Result;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum OsToolkitError {
    #[error("Platform not supported: {0}")]
    PlatformNotSupported(String),
    #[error("Hardware access error: {0}")]
    HardwareError(String),
    #[error("Permission denied: {0}")]
    PermissionDenied(String),
}

/// Represents system information across different platforms
#[derive(Debug, Clone)]
pub struct SystemInfo {
    pub architecture: String,
    pub os_type: String,
    pub memory_total: u64,
    pub cpu_cores: u32,
    pub cpu_features: Vec<String>,
}

/// Trait for platform-specific implementations
pub trait PlatformOps {
    fn get_system_info(&self) -> Result<SystemInfo>;
    fn modify_memory_protection(&self, addr: usize, size: usize, protect: bool) -> Result<()>;
    fn enumerate_processes(&self) -> Result<Vec<ProcessInfo>>;
}

#[derive(Debug, Clone)]
pub struct ProcessInfo {
    pub pid: u32,
    pub name: String,
    pub memory_usage: u64,
    pub cpu_usage: f32,
}

/// Configuration for the OS toolkit
#[derive(Debug, Clone)]
pub struct OsToolkitConfig {
    pub enable_hardware_access: bool,
    pub enable_process_manipulation: bool,
    pub enable_memory_manipulation: bool,
}

impl Default for OsToolkitConfig {
    fn default() -> Self {
        Self {
            enable_hardware_access: false,
            enable_process_manipulation: false,
            enable_memory_manipulation: false,
        }
    }
}

/// Main toolkit interface
pub struct OsToolkit {
    config: OsToolkitConfig,
    #[cfg(feature = "windows-support")]
    windows_ops: Option<windows::WindowsOps>,
    #[cfg(feature = "arm-support")]
    arm_ops: Option<arm::ArmOps>,
}

impl OsToolkit {
    pub fn new(config: OsToolkitConfig) -> Self {
        Self {
            config,
            #[cfg(feature = "windows-support")]
            windows_ops: Some(windows::WindowsOps::new()),
            #[cfg(feature = "arm-support")]
            arm_ops: Some(arm::ArmOps::new()),
        }
    }

    pub fn get_system_info(&self) -> Result<SystemInfo> {
        #[cfg(target_os = "windows")]
        {
            if let Some(ops) = &self.windows_ops {
                return ops.get_system_info();
            }
        }

        #[cfg(target_arch = "arm")]
        {
            if let Some(ops) = &self.arm_ops {
                return ops.get_system_info();
            }
        }

        Err(OsToolkitError::PlatformNotSupported("Current platform not supported".into()).into())
    }

    pub fn modify_memory_protection(&self, addr: usize, size: usize, protect: bool) -> Result<()> {
        if !self.config.enable_memory_manipulation {
            return Err(OsToolkitError::PermissionDenied("Memory manipulation not enabled".into()).into());
        }

        #[cfg(target_os = "windows")]
        {
            if let Some(ops) = &self.windows_ops {
                return ops.modify_memory_protection(addr, size, protect);
            }
        }

        #[cfg(target_arch = "arm")]
        {
            if let Some(ops) = &self.arm_ops {
                return ops.modify_memory_protection(addr, size, protect);
            }
        }

        Err(OsToolkitError::PlatformNotSupported("Current platform not supported".into()).into())
    }

    pub fn enumerate_processes(&self) -> Result<Vec<ProcessInfo>> {
        if !self.config.enable_process_manipulation {
            return Err(OsToolkitError::PermissionDenied("Process manipulation not enabled".into()).into());
        }

        #[cfg(target_os = "windows")]
        {
            if let Some(ops) = &self.windows_ops {
                return ops.enumerate_processes();
            }
        }

        #[cfg(target_arch = "arm")]
        {
            if let Some(ops) = &self.arm_ops {
                return ops.enumerate_processes();
            }
        }

        Err(OsToolkitError::PlatformNotSupported("Current platform not supported".into()).into())
    }
}
