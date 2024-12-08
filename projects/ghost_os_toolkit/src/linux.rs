use std::fs;
use std::process::Command;
use anyhow::{Result, Context};
use crate::{PlatformOps, SystemInfo, ProcessInfo, OsToolkitError};
use procfs::{CpuInfo, Meminfo, Process, ProcessCgroup};
use nix::sys::mman::{mprotect, ProtFlags};
use nix::libc;

pub struct LinuxOps {
    cpu_info: CpuInfo,
    meminfo: Meminfo,
}

impl LinuxOps {
    pub fn new() -> Result<Self> {
        Ok(Self {
            cpu_info: CpuInfo::new().context("Failed to get CPU info")?,
            meminfo: Meminfo::new().context("Failed to get memory info")?,
        })
    }

    fn get_cpu_features(&self) -> Vec<String> {
        self.cpu_info.flags()
            .unwrap_or_default()
            .into_iter()
            .collect()
    }

    fn get_kernel_version(&self) -> Result<String> {
        let output = Command::new("uname")
            .arg("-r")
            .output()
            .context("Failed to execute uname")?;
        
        String::from_utf8(output.stdout)
            .context("Invalid kernel version string")
            .map(|s| s.trim().to_string())
    }

    fn get_distro_info(&self) -> Result<String> {
        fs::read_to_string("/etc/os-release")
            .context("Failed to read os-release")
            .map(|content| {
                content.lines()
                    .find(|line| line.starts_with("PRETTY_NAME="))
                    .map(|line| line.split('=').nth(1).unwrap_or("Unknown"))
                    .unwrap_or("Unknown")
                    .trim_matches('"')
                    .to_string()
            })
    }

    fn get_process_cgroups(&self, pid: i32) -> Result<Vec<ProcessCgroup>> {
        Process::new(pid)
            .context("Failed to access process")?
            .cgroups()
            .context("Failed to get cgroups")
    }

    fn get_process_namespaces(&self, pid: i32) -> Result<Vec<String>> {
        let ns_types = ["ipc", "mnt", "net", "pid", "user", "uts"];
        let mut namespaces = Vec::new();

        for ns_type in ns_types.iter() {
            if let Ok(ns) = fs::read_link(format!("/proc/{}/ns/{}", pid, ns_type)) {
                namespaces.push(format!("{}:{}", ns_type, ns.to_string_lossy()));
            }
        }

        Ok(namespaces)
    }

    fn get_process_capabilities(&self, pid: i32) -> Result<String> {
        fs::read_to_string(format!("/proc/{}/status", pid))
            .context("Failed to read process status")
            .map(|content| {
                content.lines()
                    .filter(|line| line.starts_with("Cap"))
                    .map(|line| line.to_string())
                    .collect::<Vec<_>>()
                    .join("\n")
            })
    }
}

impl PlatformOps for LinuxOps {
    fn get_system_info(&self) -> Result<SystemInfo> {
        Ok(SystemInfo {
            architecture: std::env::consts::ARCH.to_string(),
            os_type: format!("{} {}", 
                self.get_distro_info()?, 
                self.get_kernel_version()?
            ),
            memory_total: self.meminfo.mem_total,
            cpu_cores: self.cpu_info.num_cores() as u32,
            cpu_features: self.get_cpu_features(),
        })
    }

    fn modify_memory_protection(&self, addr: usize, size: usize, protect: bool) -> Result<()> {
        let flags = if protect {
            ProtFlags::PROT_READ
        } else {
            ProtFlags::PROT_READ | ProtFlags::PROT_WRITE | ProtFlags::PROT_EXEC
        };

        unsafe {
            mprotect(
                addr as *mut libc::c_void,
                size,
                flags,
            ).context("Failed to modify memory protection")?;
        }

        Ok(())
    }

    fn enumerate_processes(&self) -> Result<Vec<ProcessInfo>> {
        let mut processes = Vec::new();
        let all_procs = procfs::process::all_processes()
            .context("Failed to enumerate processes")?;

        for proc_result in all_procs {
            if let Ok(proc) = proc_result {
                if let Ok(stat) = proc.stat() {
                    let memory_usage = proc.statm()
                        .map(|statm| statm.resident * procfs::page_size())
                        .unwrap_or(0) as u64;

                    processes.push(ProcessInfo {
                        pid: stat.pid as u32,
                        name: stat.comm,
                        memory_usage,
                        cpu_usage: stat.utime as f32 / procfs::ticks_per_second() as f32,
                    });
                }
            }
        }

        Ok(processes)
    }
}

// Advanced Linux-specific functionality
impl LinuxOps {
    pub fn get_process_details(&self, pid: i32) -> Result<ProcessDetails> {
        let process = Process::new(pid)
            .context("Failed to access process")?;
        
        Ok(ProcessDetails {
            cgroups: self.get_process_cgroups(pid)?,
            namespaces: self.get_process_namespaces(pid)?,
            capabilities: self.get_process_capabilities(pid)?,
            exe_path: process.exe()
                .context("Failed to get executable path")?
                .to_string_lossy()
                .to_string(),
            root_dir: process.root()
                .context("Failed to get root directory")?
                .to_string_lossy()
                .to_string(),
            cwd: process.cwd()
                .context("Failed to get working directory")?
                .to_string_lossy()
                .to_string(),
        })
    }

    pub fn get_system_resources(&self) -> Result<SystemResources> {
        Ok(SystemResources {
            memory_info: self.meminfo.clone(),
            cpu_info: self.cpu_info.clone(),
            load_average: procfs::LoadAverage::new()
                .context("Failed to get load average")?,
            disk_stats: procfs::DiskStats::new()
                .context("Failed to get disk stats")?,
            net_stats: procfs::net::DeviceStats::default()
                .context("Failed to get network stats")?,
        })
    }

    pub fn modify_process_resources(&self, pid: i32, cpu_shares: Option<u64>, memory_limit: Option<u64>) -> Result<()> {
        let cgroup_path = format!("/sys/fs/cgroup/cpu/ghost/{}", pid);
        fs::create_dir_all(&cgroup_path)
            .context("Failed to create cgroup directory")?;

        if let Some(shares) = cpu_shares {
            fs::write(
                format!("{}/cpu.shares", cgroup_path),
                shares.to_string(),
            ).context("Failed to set CPU shares")?;
        }

        if let Some(limit) = memory_limit {
            fs::write(
                format!("{}/memory.limit_in_bytes", cgroup_path),
                limit.to_string(),
            ).context("Failed to set memory limit")?;
        }

        fs::write(
            format!("{}/tasks", cgroup_path),
            pid.to_string(),
        ).context("Failed to add process to cgroup")?;

        Ok(())
    }
}

#[derive(Debug)]
pub struct ProcessDetails {
    pub cgroups: Vec<ProcessCgroup>,
    pub namespaces: Vec<String>,
    pub capabilities: String,
    pub exe_path: String,
    pub root_dir: String,
    pub cwd: String,
}

#[derive(Debug)]
pub struct SystemResources {
    pub memory_info: Meminfo,
    pub cpu_info: CpuInfo,
    pub load_average: procfs::LoadAverage,
    pub disk_stats: procfs::DiskStats,
    pub net_stats: procfs::net::DeviceStats,
}
