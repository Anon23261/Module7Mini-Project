# Advanced Systems & Security Portfolio

A comprehensive collection of system programming, OS development, and cybersecurity tools.

## Projects

### 1. C++ Learning Lab & OS Development
Harvard-level computer systems programming environment featuring:
- Interactive coding sandbox with real-time compilation
- OS development lab with bootloader and kernel programming
- Advanced C++17/20 tutorials and projects
- Memory management and systems programming
- Cross-platform support for x86 and ARM
- Professional-grade build system with CMake

Key components:
- Custom memory allocator implementation
- Thread scheduler and synchronization primitives
- File system development
- Microkernel architecture
- Hardware abstraction layer

### 2. Ghost OS Toolkit
Cross-platform OS development toolkit with:
- System-level operations
- Memory protection management
- Hardware interfacing
- Process enumeration
- Available in both Rust and C++17
- Comprehensive documentation

### 3. Network Scanner
Advanced network scanning tool with features including:
- Comprehensive network discovery using ARP and Nmap
- Vulnerability assessment
- Traffic analysis and packet inspection
- Detailed logging and reporting
- Multi-threaded scanning for improved performance

### 4. Vulnerability Scanner
C++ based vulnerability scanner with capabilities for:
- SQL Injection detection
- Cross-Site Scripting (XSS) detection
- CSRF vulnerability checks
- File inclusion vulnerability detection
- Multi-threaded scanning
- Comprehensive HTML and JSON reporting
- Integration with SQLite for vulnerability tracking

### 5. SQL Database Forensics Tool
PostgreSQL-based forensics tool featuring:
- Comprehensive query auditing
- Suspicious activity detection
- User activity monitoring
- Security event logging
- Detailed security reporting
- Real-time alert generation

### 6. Malware Analyzer
Advanced malware analysis tool with:
- Static and dynamic analysis capabilities
- YARA rule scanning
- PE file analysis
- String analysis
- Network behavior monitoring
- Integration with VirusTotal API
- Comprehensive HTML and JSON reporting

## Deployment

The portfolio is automatically deployed using GitHub Actions. Here's how it works:

### Frontend (GitHub Pages)

1. Push changes to the `main` branch
2. GitHub Actions automatically deploys to GitHub Pages
3. Visit your portfolio at: https://[your-username].github.io/Module7Mini-Project/

### Backend (Heroku)

The backend handles contact forms and signups. To set it up:

1. Create a Heroku account
2. Create a new app named "ghost-sec-backend"
3. Add the following secrets to your GitHub repository:
   - `HEROKU_API_KEY`: Your Heroku API key
   - `HEROKU_EMAIL`: Your Heroku email
   - `EMAIL_USER`: Gmail address for notifications
   - `EMAIL_PASS`: Gmail app-specific password
   - `ADMIN_EMAIL`: Email to receive notifications
   - `DATABASE_URL`: Your database URL (Heroku will provide this)

## Requirements

### C++ Development
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo apt-get install -y qemu-system-x86 nasm grub-pc-bin xorriso
sudo apt-get install -y gcc-multilib g++-multilib

# CentOS/RHEL
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake
sudo yum install -y qemu-kvm nasm grub2-tools xorriso
sudo yum install -y glibc-devel.i686 libstdc++-devel.i686
```

### Python Dependencies
Install Python dependencies using:
```bash
pip install -r requirements.txt
```

### System Dependencies
- PostgreSQL 13+
- OpenSSL
- Boost libraries
- libcurl
- SQLite3
- YARA

### For Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y postgresql postgresql-contrib libpq-dev
sudo apt-get install -y libboost-all-dev
sudo apt-get install -y libcurl4-openssl-dev
sudo apt-get install -y libsqlite3-dev
sudo apt-get install -y yara
```

### For CentOS/RHEL:
```bash
sudo yum install -y postgresql postgresql-server postgresql-devel
sudo yum install -y boost-devel
sudo yum install -y libcurl-devel
sudo yum install -y sqlite-devel
sudo yum install -y yara
```

## Usage

### C++ Learning Lab & OS Development
```bash
# Build and run the OS
mkdir build
cd build
cmake ..
cmake --build .
./run.sh
```

### Ghost OS Toolkit
```bash
# Build and run the toolkit
mkdir build
cd build
cmake ..
cmake --build .
./ghost_os_toolkit
```

### Network Scanner
```bash
python network_scanner.py --target 192.168.1.0/24 --output scan_results
```

### Vulnerability Scanner
```bash
./vulnerability_scanner http://target-website.com --threads 4
```

### Database Forensics
```sql
-- Enable auditing for a table
SELECT db_forensics.enable_table_audit('public', 'users');

-- Generate security report
SELECT * FROM db_forensics.generate_security_report(
    (CURRENT_TIMESTAMP - INTERVAL '24 hours'),
    CURRENT_TIMESTAMP
);
```

### Malware Analyzer
```bash
python malware_analyzer.py suspicious_file.exe -o analysis_results --verbose
```

## Security Considerations
- These tools are for educational and authorized security testing purposes only
- Always obtain proper authorization before scanning or testing systems
- Follow responsible disclosure practices when finding vulnerabilities
- Handle malware samples in isolated environments
- Protect sensitive data and credentials

## Contributing
Contributions are welcome! Please feel free to submit pull requests.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
