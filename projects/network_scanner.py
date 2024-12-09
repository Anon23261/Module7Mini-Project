#!/usr/bin/env python3
import scapy.all as scapy
import argparse
from datetime import datetime
import sys
import socket
import threading
import queue
import nmap
import json
import ssl
import requests
from concurrent.futures import ThreadPoolExecutor
from typing import Dict, List, Tuple, Optional
import logging
import csv
from dataclasses import dataclass
import netifaces
import pyshark
import pandas as pd
from rich.console import Console
from rich.table import Table

@dataclass
class VulnerabilityInfo:
    name: str
    severity: str
    description: str
    remediation: str
    cve: Optional[str] = None

@dataclass
class ServiceInfo:
    port: int
    name: str
    version: str
    vulnerabilities: List[VulnerabilityInfo]

class EnhancedNetworkScanner:
    def __init__(self, verbose: bool = False):
        self.console = Console()
        self.verbose = verbose
        self.active_hosts = []
        self.services = {}
        self.vulnerabilities = {}
        self.scan_queue = queue.Queue()
        self.nm = nmap.PortScanner()
        self.setup_logging()
        
    def setup_logging(self):
        """Configure logging with detailed formatting"""
        logging.basicConfig(
            level=logging.INFO if self.verbose else logging.WARNING,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler('network_scan.log'),
                logging.StreamHandler()
            ]
        )
        self.logger = logging.getLogger('NetworkScanner')

    def scan_network(self, ip_range: str, scan_type: str = 'full') -> List[Dict]:
        """
        Perform comprehensive network scan with multiple techniques
        
        Args:
            ip_range: Target IP range in CIDR notation
            scan_type: Type of scan ('quick', 'full', 'stealth')
        """
        self.logger.info(f"Starting {scan_type} scan on {ip_range}")
        
        # ARP scan for initial host discovery
        self._arp_scan(ip_range)
        
        # Parallel port scanning and service detection
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(self._scan_host, host) for host in self.active_hosts]
            for future in futures:
                future.result()
        
        # Vulnerability assessment
        self._assess_vulnerabilities()
        
        return self.generate_report()

    def _arp_scan(self, ip_range: str):
        """Enhanced ARP scanning with vendor identification"""
        try:
            arp_request = scapy.ARP(pdst=ip_range)
            broadcast = scapy.Ether(dst="ff:ff:ff:ff:ff:ff")
            arp_request_broadcast = broadcast/arp_request
            answered_list = scapy.srp(arp_request_broadcast, timeout=1, verbose=False)[0]
            
            for element in answered_list:
                host_dict = {
                    "ip": element[1].psrc,
                    "mac": element[1].hwsrc,
                    "vendor": self._get_vendor(element[1].hwsrc),
                    "last_seen": datetime.now().isoformat()
                }
                self.active_hosts.append(host_dict)
                
        except Exception as e:
            self.logger.error(f"ARP scan error: {e}")

    def _scan_host(self, host: Dict):
        """Comprehensive host scanning including service detection"""
        try:
            # Nmap service detection
            self.nm.scan(host['ip'], arguments='-sV -sC --script vuln')
            
            if host['ip'] in self.nm.all_hosts():
                host_data = self.nm[host['ip']]
                self.services[host['ip']] = []
                
                for proto in host_data.all_protocols():
                    ports = host_data[proto].keys()
                    for port in ports:
                        service = host_data[proto][port]
                        service_info = ServiceInfo(
                            port=port,
                            name=service.get('name', ''),
                            version=service.get('version', ''),
                            vulnerabilities=[]
                        )
                        self.services[host['ip']].append(service_info)
                        
                        # Check for known vulnerabilities
                        if 'script' in service:
                            for script_name, output in service['script'].items():
                                if 'VULNERABLE' in output:
                                    vuln = VulnerabilityInfo(
                                        name=script_name,
                                        severity='High',
                                        description=output,
                                        remediation='Update service to latest version'
                                    )
                                    service_info.vulnerabilities.append(vuln)
                
        except Exception as e:
            self.logger.error(f"Host scan error for {host['ip']}: {e}")

    def _assess_vulnerabilities(self):
        """Assess vulnerabilities and security misconfigurations"""
        for host in self.active_hosts:
            ip = host['ip']
            self.vulnerabilities[ip] = []
            
            # Check for common security issues
            self._check_open_ports(ip)
            self._check_service_versions(ip)
            self._check_ssl_security(ip)
            self._check_default_credentials(ip)

    def _check_open_ports(self, ip: str):
        """Check for potentially dangerous open ports"""
        dangerous_ports = {
            21: "FTP",
            23: "Telnet",
            445: "SMB",
            3389: "RDP"
        }
        
        if ip in self.services:
            for service in self.services[ip]:
                if service.port in dangerous_ports:
                    vuln = VulnerabilityInfo(
                        name=f"Open {dangerous_ports[service.port]} Port",
                        severity="Medium",
                        description=f"Potentially dangerous port {service.port} ({dangerous_ports[service.port]}) is open",
                        remediation="Close port if not needed or implement strict firewall rules"
                    )
                    self.vulnerabilities[ip].append(vuln)

    def _check_service_versions(self, ip: str):
        """Check for outdated service versions"""
        if ip in self.services:
            for service in self.services[ip]:
                if service.version:
                    # Check against known vulnerable versions (simplified)
                    if self._is_vulnerable_version(service.name, service.version):
                        vuln = VulnerabilityInfo(
                            name=f"Outdated {service.name}",
                            severity="High",
                            description=f"Running potentially vulnerable version: {service.version}",
                            remediation="Update to latest stable version"
                        )
                        self.vulnerabilities[ip].append(vuln)

    def _check_ssl_security(self, ip: str):
        """Check SSL/TLS configuration"""
        ssl_ports = [443, 8443]
        for service in self.services.get(ip, []):
            if service.port in ssl_ports:
                try:
                    context = ssl.create_default_context()
                    with socket.create_connection((ip, service.port)) as sock:
                        with context.wrap_socket(sock, server_hostname=ip) as ssock:
                            cert = ssock.getpeercert()
                            self._analyze_certificate(ip, cert)
                except Exception as e:
                    self.logger.warning(f"SSL check failed for {ip}:{service.port} - {e}")

    def _analyze_certificate(self, ip: str, cert: Dict):
        """Analyze SSL certificate for security issues"""
        if 'notAfter' in cert:
            expiry = datetime.strptime(cert['notAfter'], '%b %d %H:%M:%S %Y %Z')
            if expiry < datetime.now():
                vuln = VulnerabilityInfo(
                    name="Expired SSL Certificate",
                    severity="High",
                    description="SSL certificate has expired",
                    remediation="Renew SSL certificate"
                )
                self.vulnerabilities[ip].append(vuln)

    def _check_default_credentials(self, ip: str):
        """Check for default credentials on common services"""
        default_creds = {
            'admin': 'admin',
            'root': 'root',
            'administrator': 'password'
        }
        
        for service in self.services.get(ip, []):
            if service.name in ['http', 'https', 'ftp', 'ssh']:
                # Implement actual credential checking here
                pass

    def _get_vendor(self, mac: str) -> str:
        """Get vendor information from MAC address"""
        try:
            # You would typically use a MAC address database here
            return "Unknown Vendor"
        except Exception:
            return "Unknown Vendor"

    def _is_vulnerable_version(self, service: str, version: str) -> bool:
        """Check if service version is known to be vulnerable"""
        # Implement version checking against CVE database
        return False

    def capture_traffic(self, interface: str, duration: int = 60):
        """Capture and analyze network traffic"""
        try:
            capture = pyshark.LiveCapture(interface=interface)
            packets = []
            
            self.logger.info(f"Starting traffic capture on {interface} for {duration} seconds")
            capture.sniff(timeout=duration)
            
            for packet in capture:
                if hasattr(packet, 'ip'):
                    packets.append({
                        'time': packet.sniff_time,
                        'source': packet.ip.src,
                        'destination': packet.ip.dst,
                        'protocol': packet.highest_layer,
                        'length': packet.length
                    })
            
            return self._analyze_traffic(packets)
            
        except Exception as e:
            self.logger.error(f"Traffic capture error: {e}")
            return None

    def _analyze_traffic(self, packets: List[Dict]) -> Dict:
        """Analyze captured traffic for patterns and anomalies"""
        df = pd.DataFrame(packets)
        analysis = {
            'total_packets': len(packets),
            'protocols': df['protocol'].value_counts().to_dict(),
            'top_talkers': df['source'].value_counts().head(10).to_dict(),
            'top_destinations': df['destination'].value_counts().head(10).to_dict(),
            'average_packet_size': df['length'].mean()
        }
        return analysis

    def generate_report(self) -> Dict:
        """Generate comprehensive scan report"""
        report = {
            'scan_time': datetime.now().isoformat(),
            'hosts': [],
            'summary': {
                'total_hosts': len(self.active_hosts),
                'total_vulnerabilities': sum(len(vulns) for vulns in self.vulnerabilities.values()),
                'risk_levels': {'High': 0, 'Medium': 0, 'Low': 0}
            }
        }
        
        for host in self.active_hosts:
            host_report = {
                'ip': host['ip'],
                'mac': host['mac'],
                'vendor': host['vendor'],
                'services': [],
                'vulnerabilities': []
            }
            
            # Add services
            if host['ip'] in self.services:
                for service in self.services[host['ip']]:
                    host_report['services'].append({
                        'port': service.port,
                        'name': service.name,
                        'version': service.version
                    })
            
            # Add vulnerabilities
            if host['ip'] in self.vulnerabilities:
                for vuln in self.vulnerabilities[host['ip']]:
                    host_report['vulnerabilities'].append({
                        'name': vuln.name,
                        'severity': vuln.severity,
                        'description': vuln.description,
                        'remediation': vuln.remediation
                    })
                    report['summary']['risk_levels'][vuln.severity] += 1
            
            report['hosts'].append(host_report)
        
        return report

    def export_report(self, report: Dict, format: str = 'json'):
        """Export report in various formats"""
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        
        if format == 'json':
            with open(f'scan_report_{timestamp}.json', 'w') as f:
                json.dump(report, f, indent=4)
        
        elif format == 'csv':
            # Flatten report for CSV format
            rows = []
            for host in report['hosts']:
                for vuln in host['vulnerabilities']:
                    rows.append({
                        'ip': host['ip'],
                        'mac': host['mac'],
                        'vendor': host['vendor'],
                        'vulnerability': vuln['name'],
                        'severity': vuln['severity'],
                        'description': vuln['description'],
                        'remediation': vuln['remediation']
                    })
            
            if rows:
                df = pd.DataFrame(rows)
                df.to_csv(f'scan_report_{timestamp}.csv', index=False)
        
        elif format == 'html':
            # Generate HTML report with styling
            self._generate_html_report(report, f'scan_report_{timestamp}.html')

    def _generate_html_report(self, report: Dict, filename: str):
        """Generate styled HTML report"""
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>Network Scan Report</title>
            <style>
                body {{ font-family: Arial, sans-serif; margin: 20px; }}
                .vulnerability {{ margin: 10px 0; padding: 10px; border-radius: 5px; }}
                .High {{ background-color: #ffebee; }}
                .Medium {{ background-color: #fff3e0; }}
                .Low {{ background-color: #e8f5e9; }}
            </style>
        </head>
        <body>
            <h1>Network Scan Report</h1>
            <h2>Summary</h2>
            <p>Total Hosts: {report['summary']['total_hosts']}</p>
            <p>Total Vulnerabilities: {report['summary']['total_vulnerabilities']}</p>
            
            <h2>Hosts</h2>
        """
        
        for host in report['hosts']:
            html_content += f"""
            <div class="host">
                <h3>Host: {host['ip']}</h3>
                <p>MAC: {host['mac']}</p>
                <p>Vendor: {host['vendor']}</p>
                
                <h4>Services</h4>
                <ul>
            """
            
            for service in host['services']:
                html_content += f"""
                    <li>Port {service['port']}: {service['name']} ({service['version']})</li>
                """
            
            html_content += """
                </ul>
                <h4>Vulnerabilities</h4>
            """
            
            for vuln in host['vulnerabilities']:
                html_content += f"""
                <div class="vulnerability {vuln['severity']}">
                    <h5>{vuln['name']} ({vuln['severity']})</h5>
                    <p>{vuln['description']}</p>
                    <p><strong>Remediation:</strong> {vuln['remediation']}</p>
                </div>
                """
        
        html_content += """
        </body>
        </html>
        """
        
        with open(filename, 'w') as f:
            f.write(html_content)

def main():
    parser = argparse.ArgumentParser(description='Enhanced Network Security Scanner')
    parser.add_argument('-r', '--range', help='IP range to scan (CIDR notation)')
    parser.add_argument('-t', '--type', choices=['quick', 'full', 'stealth'], 
                        default='full', help='Scan type')
    parser.add_argument('-i', '--interface', help='Interface for traffic capture')
    parser.add_argument('-d', '--duration', type=int, default=60,
                        help='Traffic capture duration in seconds')
    parser.add_argument('-f', '--format', choices=['json', 'csv', 'html'],
                        default='html', help='Report format')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose output')
    args = parser.parse_args()
    
    if not args.range:
        print("Please specify an IP range")
        sys.exit(1)
        
    scanner = EnhancedNetworkScanner(verbose=args.verbose)
    
    # Perform network scan
    report = scanner.scan_network(args.range, args.type)
    
    # Capture traffic if interface specified
    if args.interface:
        traffic_analysis = scanner.capture_traffic(args.interface, args.duration)
        if traffic_analysis:
            report['traffic_analysis'] = traffic_analysis
    
    # Export report
    scanner.export_report(report, args.format)
    print(f"Scan complete. Report saved in {args.format} format.")

if __name__ == "__main__":
    main()
