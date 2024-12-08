use std::error::Error;
use clap::{Parser, Subcommand};
use log::{info, warn, error};
use anyhow::Result;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Port scanner to identify open ports on a target
    PortScan {
        /// Target IP or hostname
        #[arg(short, long)]
        target: String,
        
        /// Port range to scan (e.g., "1-1024")
        #[arg(short, long, default_value = "1-1024")]
        ports: String,
    },
    
    /// File integrity checker using SHA-256
    Integrity {
        /// Path to file or directory to check
        #[arg(short, long)]
        path: String,
        
        /// Optional baseline hash file
        #[arg(short, long)]
        baseline: Option<String>,
    },
    
    /// Network packet analyzer
    PacketAnalyze {
        /// Network interface to capture
        #[arg(short, long)]
        interface: String,
        
        /// Duration in seconds to capture
        #[arg(short, long, default_value = "60")]
        duration: u64,
    },
}

mod port_scanner;
mod integrity_checker;
mod packet_analyzer;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    env_logger::init();
    let cli = Cli::parse();

    match &cli.command {
        Commands::PortScan { target, ports } => {
            info!("Starting port scan on {} with range {}", target, ports);
            port_scanner::scan_ports(target, ports).await?;
        }
        Commands::Integrity { path, baseline } => {
            info!("Checking file integrity for {}", path);
            integrity_checker::check_integrity(path, baseline.as_deref())?;
        }
        Commands::PacketAnalyze { interface, duration } => {
            info!("Starting packet analysis on {} for {} seconds", interface, duration);
            packet_analyzer::analyze_packets(interface, *duration).await?;
        }
    }

    Ok(())
}
