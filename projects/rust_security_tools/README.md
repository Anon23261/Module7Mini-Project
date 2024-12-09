# Rust Security Tools

A collection of security tools written in Rust, including:
- Port Scanner
- File Integrity Checker
- Network Packet Analyzer

## Prerequisites

- Rust (latest stable version)
- Cargo (Rust's package manager)
- libssl-dev (for OpenSSL)
- libpcap-dev (for packet capture)

## Installation

1. Install Rust:
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

2. Clone the repository:
```bash
git clone <repository-url>
cd rust_security_tools
```

3. Build the project:
```bash
cargo build --release
```

## Usage

### Port Scanner
```bash
cargo run -- port-scan -t 192.168.1.1 -p 1-1024
```

### File Integrity Checker
```bash
# Generate hash for a file
cargo run -- integrity -p /path/to/file

# Check against baseline
cargo run -- integrity -p /path/to/file -b /path/to/baseline
```

### Packet Analyzer
```bash
# Requires root privileges
sudo cargo run -- packet-analyze -i eth0 -d 60
```

## Features

### Port Scanner
- Fast asynchronous port scanning
- Configurable port ranges
- Timeout settings for unresponsive ports

### File Integrity Checker
- SHA-256 hash calculation
- Directory recursive scanning
- Baseline comparison

### Packet Analyzer (Placeholder)
- Network interface monitoring
- Packet capture duration control
- Basic packet analysis

## Security Notes

- Some features require root privileges
- Always verify scripts and tools before running them
- Use in controlled, authorized environments only

## License

MIT License - See LICENSE file for details
