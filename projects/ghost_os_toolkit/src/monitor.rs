use std::sync::{Arc, Mutex};
use std::collections::VecDeque;
use std::time::{Duration, Instant};
use tokio::time;
use anyhow::Result;
use serde::{Serialize, Deserialize};

use crate::{OsToolkit, SystemInfo, ProcessInfo};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SystemMetrics {
    pub timestamp: u64,
    pub cpu_usage: f32,
    pub memory_usage: f32,
    pub disk_io: DiskMetrics,
    pub network_io: NetworkMetrics,
    pub process_count: usize,
    pub system_load: [f32; 3],
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DiskMetrics {
    pub reads_per_sec: f32,
    pub writes_per_sec: f32,
    pub read_bytes_per_sec: u64,
    pub write_bytes_per_sec: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetrics {
    pub bytes_received: u64,
    pub bytes_sent: u64,
    pub packets_received: u64,
    pub packets_sent: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ProcessMetrics {
    pub pid: u32,
    pub name: String,
    pub cpu_usage: f32,
    pub memory_usage: u64,
    pub disk_io: DiskMetrics,
    pub network_io: NetworkMetrics,
}

pub struct SystemMonitor {
    toolkit: Arc<OsToolkit>,
    metrics_history: Arc<Mutex<VecDeque<SystemMetrics>>>,
    process_metrics: Arc<Mutex<VecDeque<Vec<ProcessMetrics>>>>,
    history_size: usize,
}

impl SystemMonitor {
    pub fn new(toolkit: Arc<OsToolkit>, history_size: usize) -> Self {
        Self {
            toolkit,
            metrics_history: Arc::new(Mutex::new(VecDeque::with_capacity(history_size))),
            process_metrics: Arc::new(Mutex::new(VecDeque::with_capacity(history_size))),
            history_size,
        }
    }

    pub async fn start_monitoring(&self, interval: Duration) -> Result<()> {
        let metrics_history = Arc::clone(&self.metrics_history);
        let process_metrics = Arc::clone(&self.process_metrics);
        let toolkit = Arc::clone(&self.toolkit);
        let history_size = self.history_size;

        tokio::spawn(async move {
            let mut interval = time::interval(interval);
            loop {
                interval.tick().await;
                
                if let Ok(system_metrics) = Self::collect_system_metrics(&toolkit) {
                    let mut history = metrics_history.lock().unwrap();
                    history.push_back(system_metrics);
                    while history.len() > history_size {
                        history.pop_front();
                    }
                }

                if let Ok(proc_metrics) = Self::collect_process_metrics(&toolkit) {
                    let mut history = process_metrics.lock().unwrap();
                    history.push_back(proc_metrics);
                    while history.len() > history_size {
                        history.pop_front();
                    }
                }
            }
        });

        Ok(())
    }

    fn collect_system_metrics(toolkit: &OsToolkit) -> Result<SystemMetrics> {
        let system_info = toolkit.get_system_info()?;
        let processes = toolkit.enumerate_processes()?;
        
        // On Linux, we can get more detailed metrics through the LinuxOps
        #[cfg(target_os = "linux")]
        {
            if let Some(linux_ops) = toolkit.get_platform_ops().downcast_ref::<crate::linux::LinuxOps>() {
                let resources = linux_ops.get_system_resources()?;
                return Ok(SystemMetrics {
                    timestamp: std::time::SystemTime::now()
                        .duration_since(std::time::UNIX_EPOCH)?
                        .as_secs(),
                    cpu_usage: calculate_cpu_usage(&resources.cpu_info),
                    memory_usage: calculate_memory_usage(&resources.memory_info),
                    disk_io: collect_disk_metrics(&resources.disk_stats),
                    network_io: collect_network_metrics(&resources.net_stats),
                    process_count: processes.len(),
                    system_load: [
                        resources.load_average.one,
                        resources.load_average.five,
                        resources.load_average.fifteen,
                    ],
                });
            }
        }

        // Generic metrics for other platforms
        Ok(SystemMetrics {
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)?
                .as_secs(),
            cpu_usage: 0.0, // Implement platform-specific CPU usage calculation
            memory_usage: 0.0, // Implement platform-specific memory usage calculation
            disk_io: DiskMetrics {
                reads_per_sec: 0.0,
                writes_per_sec: 0.0,
                read_bytes_per_sec: 0,
                write_bytes_per_sec: 0,
            },
            network_io: NetworkMetrics {
                bytes_received: 0,
                bytes_sent: 0,
                packets_received: 0,
                packets_sent: 0,
            },
            process_count: processes.len(),
            system_load: [0.0, 0.0, 0.0],
        })
    }

    fn collect_process_metrics(toolkit: &OsToolkit) -> Result<Vec<ProcessMetrics>> {
        let processes = toolkit.enumerate_processes()?;
        let mut metrics = Vec::with_capacity(processes.len());

        for process in processes {
            metrics.push(ProcessMetrics {
                pid: process.pid,
                name: process.name,
                cpu_usage: process.cpu_usage,
                memory_usage: process.memory_usage,
                disk_io: DiskMetrics {
                    reads_per_sec: 0.0,
                    writes_per_sec: 0.0,
                    read_bytes_per_sec: 0,
                    write_bytes_per_sec: 0,
                },
                network_io: NetworkMetrics {
                    bytes_received: 0,
                    bytes_sent: 0,
                    packets_received: 0,
                    packets_sent: 0,
                },
            });
        }

        Ok(metrics)
    }

    pub fn get_metrics_history(&self) -> Result<Vec<SystemMetrics>> {
        Ok(self.metrics_history.lock().unwrap().iter().cloned().collect())
    }

    pub fn get_process_metrics_history(&self) -> Result<Vec<Vec<ProcessMetrics>>> {
        Ok(self.process_metrics.lock().unwrap().iter().cloned().collect())
    }

    pub fn get_latest_metrics(&self) -> Result<Option<SystemMetrics>> {
        Ok(self.metrics_history.lock().unwrap().back().cloned())
    }

    pub fn get_latest_process_metrics(&self) -> Result<Option<Vec<ProcessMetrics>>> {
        Ok(self.process_metrics.lock().unwrap().back().cloned())
    }
}

#[cfg(target_os = "linux")]
fn calculate_cpu_usage(cpu_info: &procfs::CpuInfo) -> f32 {
    // Implement Linux-specific CPU usage calculation
    0.0 // Placeholder
}

#[cfg(target_os = "linux")]
fn calculate_memory_usage(meminfo: &procfs::Meminfo) -> f32 {
    let used = meminfo.mem_total - meminfo.mem_available.unwrap_or(meminfo.mem_free);
    (used as f32 / meminfo.mem_total as f32) * 100.0
}

#[cfg(target_os = "linux")]
fn collect_disk_metrics(disk_stats: &procfs::DiskStats) -> DiskMetrics {
    // Implement Linux-specific disk metrics collection
    DiskMetrics {
        reads_per_sec: 0.0,
        writes_per_sec: 0.0,
        read_bytes_per_sec: 0,
        write_bytes_per_sec: 0,
    }
}

#[cfg(target_os = "linux")]
fn collect_network_metrics(net_stats: &procfs::net::DeviceStats) -> NetworkMetrics {
    // Implement Linux-specific network metrics collection
    NetworkMetrics {
        bytes_received: 0,
        bytes_sent: 0,
        packets_received: 0,
        packets_sent: 0,
    }
}
