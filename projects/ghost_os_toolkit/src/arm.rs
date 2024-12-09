#[cfg(feature = "arm-support")]
use cortex_m::peripheral::{CPUID, SCB};
use anyhow::Result;
use crate::{PlatformOps, SystemInfo, ProcessInfo, OsToolkitError};

pub struct ArmOps {
    #[cfg(feature = "arm-support")]
    cpuid: CPUID,
    #[cfg(feature = "arm-support")]
    scb: SCB,
}

impl ArmOps {
    pub fn new() -> Self {
        #[cfg(feature = "arm-support")]
        {
            let cp = cortex_m::Peripherals::take().expect("Failed to take peripherals");
            Self {
                cpuid: cp.CPUID,
                scb: cp.SCB,
            }
        }

        #[cfg(not(feature = "arm-support"))]
        Self {}
    }

    #[cfg(feature = "arm-support")]
    fn get_cpu_features(&self) -> Vec<String> {
        let mut features = Vec::new();
        
        // Check ARM-specific features
        if self.cpuid.isa() & (1 << 4) != 0 {
            features.push("thumb".to_string());
        }
        if self.cpuid.isa() & (1 << 5) != 0 {
            features.push("thumb2".to_string());
        }
        if self.cpuid.isa() & (1 << 6) != 0 {
            features.push("jazelle".to_string());
        }

        features
    }
}

impl PlatformOps for ArmOps {
    fn get_system_info(&self) -> Result<SystemInfo> {
        #[cfg(feature = "arm-support")]
        {
            let cpu_features = self.get_cpu_features();
            let revision = self.cpuid.revision().revision();
            let variant = self.cpuid.revision().variant();

            Ok(SystemInfo {
                architecture: format!("ARMv{}-M", variant),
                os_type: "Bare Metal ARM".to_string(),
                memory_total: 0, // Would need to be configured per device
                cpu_cores: 1,    // Most Cortex-M are single core
                cpu_features,
            })
        }

        #[cfg(not(feature = "arm-support"))]
        Err(OsToolkitError::PlatformNotSupported("ARM support not enabled".into()).into())
    }

    fn modify_memory_protection(&self, addr: usize, size: usize, protect: bool) -> Result<()> {
        #[cfg(feature = "arm-support")]
        {
            // On ARM Cortex-M, memory protection is handled by the MPU
            // This is a simplified version - real implementation would need to configure MPU regions
            if protect {
                // Enable MPU and configure region
                unsafe {
                    self.scb.enable_mpu();
                    // Would need to properly configure MPU regions here
                }
            } else {
                unsafe {
                    self.scb.disable_mpu();
                }
            }
            Ok(())
        }

        #[cfg(not(feature = "arm-support"))]
        Err(OsToolkitError::PlatformNotSupported("ARM support not enabled".into()).into())
    }

    fn enumerate_processes(&self) -> Result<Vec<ProcessInfo>> {
        #[cfg(feature = "arm-support")]
        {
            // On bare metal ARM, there typically aren't multiple processes
            // This could be implemented for RTOS scenarios
            Ok(vec![ProcessInfo {
                pid: 1,
                name: "main".to_string(),
                memory_usage: 0,
                cpu_usage: 100.0,
            }])
        }

        #[cfg(not(feature = "arm-support"))]
        Err(OsToolkitError::PlatformNotSupported("ARM support not enabled".into()).into())
    }
}
