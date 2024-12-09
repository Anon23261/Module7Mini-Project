use std::net::{IpAddr, SocketAddr};
use tokio::net::TcpStream;
use tokio::time::{timeout, Duration};
use anyhow::{Result, Context};
use log::{info, warn};

pub async fn scan_ports(target: &str, port_range: &str) -> Result<Vec<u16>> {
    let ip: IpAddr = target.parse().context("Failed to parse target IP")?;
    let (start_port, end_port) = parse_port_range(port_range)?;
    let mut open_ports = Vec::new();

    for port in start_port..=end_port {
        let addr = SocketAddr::new(ip, port);
        match timeout(Duration::from_secs(1), TcpStream::connect(&addr)).await {
            Ok(Ok(_)) => {
                info!("Port {} is open", port);
                open_ports.push(port);
            }
            Ok(Err(_)) => {
                warn!("Port {} is closed", port);
            }
            Err(_) => {
                warn!("Port {} timed out", port);
            }
        }
    }

    Ok(open_ports)
}

fn parse_port_range(range: &str) -> Result<(u16, u16)> {
    let parts: Vec<&str> = range.split('-').collect();
    match parts.as_slice() {
        [start, end] => {
            let start_port = start.parse().context("Invalid start port")?;
            let end_port = end.parse().context("Invalid end port")?;
            Ok((start_port, end_port))
        }
        [single] => {
            let port = single.parse().context("Invalid port")?;
            Ok((port, port))
        }
        _ => anyhow::bail!("Invalid port range format. Use 'start-end' or 'port'"),
    }
}
