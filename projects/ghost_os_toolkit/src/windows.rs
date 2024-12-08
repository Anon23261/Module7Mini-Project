#[cfg(feature = "windows-support")]
use windows::Win32::System::{
    SystemInformation::{GetSystemInfo, SYSTEM_INFO},
    ProcessStatus::{K32EnumProcesses, K32GetProcessImageFileNameA},
    Memory::{VirtualProtect, PAGE_PROTECTION_FLAGS},
};
use anyhow::Result;
use crate::{PlatformOps, SystemInfo, ProcessInfo, OsToolkitError};

pub struct WindowsOps;

impl WindowsOps {
    pub fn new() -> Self {
        Self
    }
}

impl PlatformOps for WindowsOps {
    fn get_system_info(&self) -> Result<SystemInfo> {
        unsafe {
            let mut sys_info: SYSTEM_INFO = std::mem::zeroed();
            GetSystemInfo(&mut sys_info);

            Ok(SystemInfo {
                architecture: format!("{:?}", sys_info.u.ProcessorArchitecture),
                os_type: "Windows".to_string(),
                memory_total: sys_info.ullTotalPhys as u64,
                cpu_cores: sys_info.dwNumberOfProcessors as u32,
                cpu_features: vec![], // Would need to be populated with Windows-specific CPU features
            })
        }
    }

    fn modify_memory_protection(&self, addr: usize, size: usize, protect: bool) -> Result<()> {
        unsafe {
            let mut old_protect: PAGE_PROTECTION_FLAGS = PAGE_PROTECTION_FLAGS(0);
            let new_protect = if protect {
                PAGE_PROTECTION_FLAGS(0x40) // PAGE_EXECUTE_READWRITE
            } else {
                PAGE_PROTECTION_FLAGS(0x02) // PAGE_READONLY
            };

            VirtualProtect(
                addr as *const _,
                size,
                new_protect,
                &mut old_protect,
            ).map_err(|e| OsToolkitError::HardwareError(format!("Failed to modify memory protection: {:?}", e)))?;

            Ok(())
        }
    }

    fn enumerate_processes(&self) -> Result<Vec<ProcessInfo>> {
        unsafe {
            let mut processes = Vec::new();
            let mut buffer = vec![0u32; 1024];
            let mut bytes_returned = 0u32;

            K32EnumProcesses(
                buffer.as_mut_ptr(),
                (buffer.len() * std::mem::size_of::<u32>()) as u32,
                &mut bytes_returned,
            ).map_err(|e| OsToolkitError::HardwareError(format!("Failed to enumerate processes: {:?}", e)))?;

            let count = bytes_returned as usize / std::mem::size_of::<u32>();
            for &pid in &buffer[..count] {
                if pid != 0 {
                    let mut name_buf = vec![0u8; 260];
                    if let Ok(_) = K32GetProcessImageFileNameA(
                        pid as isize,
                        &mut name_buf,
                    ) {
                        let name = String::from_utf8_lossy(&name_buf)
                            .trim_matches(char::from(0))
                            .to_string();
                        
                        processes.push(ProcessInfo {
                            pid,
                            name,
                            memory_usage: 0, // Would need GetProcessMemoryInfo
                            cpu_usage: 0.0,  // Would need GetProcessTimes
                        });
                    }
                }
            }

            Ok(processes)
        }
    }
}
