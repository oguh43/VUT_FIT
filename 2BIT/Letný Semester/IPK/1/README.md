# IPK Project 1 - OMEGA: L4 Scanner

## Table of Contents
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Usage](#usage)
   - [Command-line Options](#command-line-options)
   - [Examples](#examples)
   - [Output Format](#output-format)
4. [Theoretical Background](#theoretical-background)
   - [TCP Scanning](#tcp-scanning)
   - [UDP Scanning](#udp-scanning)
   - [Port States](#port-states)
5. [Implementation Details](#implementation-details)
   - [Project Structure](#project-structure)
   - [Key Components](#key-components)
   - [Core Scanning Techniques](#core-scanning-techniques)
   - [Multithreading Approach](#multithreading-approach)
6. [Testing](#testing)
   - [Testing Environment](#testing-environment)
   - [Test Cases](#test-cases)
   - [Comparison with Nmap](#comparison-with-nmap)
7. [Extra Features](#extra-features)
8. [Known Limitations](#known-limitations)
9. [Bibliography](#bibliography)

## Introduction

This project implements a Layer 4 (L4) network port scanner capable of detecting the status of TCP and UDP ports on remote hosts. The scanner can handle both IPv4 and IPv6 addresses, perform simultaneous TCP and UDP scans, and support various port ranges. The implementation follows RFC specifications for TCP and UDP protocols and employs techniques similar to those used by popular network scanning tools like Nmap.

The scanner identifies three port states:
- **Open**: The port is accepting connections or datagrams
- **Closed**: The port is not accepting connections or datagrams
- **Filtered**: The port's status could not be determined - likely due to a firewall

## Installation

### Prerequisites
- Linux operating system (tested on Arch Linux x86_64)
- GCC compiler
- libpcap development library
- POSIX thread library

### Compilation

To compile the application, simply run:

```
make
```

This creates the executable `ipk-l4-scan` in the current directory.

## Usage

### Command-line Options

```
./ipk-l4-scan [-i interface | --interface interface] [--pu port-ranges | --pt port-ranges | -u port-ranges | -t port-ranges] {-w timeout} [hostname | ip-address]
./ipk-l4-scan --help
./ipk-l4-scan --interface
./ipk-l4-scan
```

Where:
- `-h/--help`: Displays usage instructions.
- `-i eth0` or `--interface eth0`: Specifies the network interface to use, in this case `eth0`.
- `-t` or `--pt`: Specifies TCP ports to scan (e.g., `22`, `1-65535`, or `22,23,24`).
- `-u` or `--pu`: Specifies UDP ports to scan (e.g., `53,67` or `1-1000`).
- `-w 3000` or `--wait 3000`: Sets timeout in milliseconds (default: 5000), in this case `3000`.
- `hostname` or `ip-address`: Target hostname or IPv4/IPv6 address.

If no parameters are specified, or only `-i/--interface` is provided without a value, the program displays a list of available network interfaces.
If `-h/--help` is specified anywhere among the arguments, usage instructions are displayed and the program exits.

### Examples

Scan UDP ports 53 and 67 on an IPv6 address using the eth0 interface:
```
./ipk-l4-scan --interface eth0 -u 53,67 2001:67c:1220:809::93e5:917
```

Scan TCP ports 80, 443, and 8080 on www.vutbr.cz with a 1-second timeout:
```
./ipk-l4-scan -i eth0 -w 1000 -t 80,443,8080 www.vutbr.cz
```

Scan multiple TCP and UDP ports on localhost:
```
./ipk-l4-scan -i eth0 --pt 21,22,143 --pu 53,67 localhost
```

### Output Format

The program outputs scan results line by line in the following format:
```
<ip-address> <port-number> <protocol> <port-state>
```

For example:
```
127.0.0.1 22 tcp open
127.0.0.1 21 tcp closed
127.0.0.1 143 tcp filtered
127.0.0.1 53 udp closed
127.0.0.1 67 udp open
```

The application can be terminated at any time using the Ctrl+C key combination - used while debugging.

## Theoretical Background

### TCP Scanning

TCP (Transmission Control Protocol) is a connection-oriented protocol that establishes a reliable connection through a three-way handshake. The L4 scanner implements a SYN scan technique, also known as a "half-open" or "stealth" scan.

#### SYN Scanning Process:
1. The scanner sends a TCP packet with the SYN flag set to the target port.
2. If the port is open, the target responds with a SYN+ACK packet.
3. If the port is closed, the target responds with an RST packet.
4. If no response is received (or it's delayed beyond the timeout), the port is considered filtered.

Instead of completing the three-way handshake (which would require sending an ACK after receiving the SYN+ACK), the scanner immediately closes the connection, making the scan less intrusive.

### UDP Scanning

UDP (User Datagram Protocol) is a connectionless protocol with no handshake mechanism. This makes UDP port scanning more challenging.

#### UDP Scanning Process:
1. The scanner sends a UDP packet to the target port.
2. If the port is closed, the target typically responds with an ICMP "port unreachable" message.
3. If the port is open, there might be:
   - No response at all
   - A UDP response (if the service recognizes the content of the probe packet)
4. If no response is received, the port is considered open or filtered (my implementation marks these as open).

UDP scanning is generally less reliable than TCP scanning because open ports may not respond to probe packets, and firewalls may block ICMP responses.

### Port States

The scanner classifies ports into three states:

- **Open**: The port is accessible and a service is listening on it.
  - TCP: A SYN+ACK packet was received.
  - UDP: Either a UDP response was received or no ICMP "port unreachable" message was received.

- **Closed**: The port is accessible but no service is listening on it.
  - TCP: An RST packet was received.
  - UDP: An ICMP "port unreachable" message was received.

- **Filtered**: The scanner couldn't determine if the port is open or closed.
  - TCP: No response was received after multiple retries.
  - UDP: (Not applicable in this implementation, as non-responsive ports are marked as open).

## Implementation Details

### Project Structure

The codebase is organized into several modules:

- **main.c**: Command-line argument parsing and program entry point.
- **scanner.c**: Core scanning functionality and thread management.
- **tcp_scanner.c**: TCP scanning implementation for IPv4.
- **tcp_scanner_ipv6.c**: TCP scanning implementation for IPv6.
- **udp_scanner.c**: UDP scanning implementation for IPv4.
- **udp_scanner_ipv6.c**: UDP scanning implementation for IPv6.
- **util.c**: Utility functions for network operations.
- **scanner.h** and **util.h**: Header files defining interfaces.

### Key Components

1. **Command-line Argument Processing**: The program uses `getopt_long()` to parse command-line arguments, supporting both short and long option formats.

2. **Network Interface Discovery**: The program can list available network interfaces using the `getifaddrs()` function.

3. **Hostname Resolution**: The program resolves hostnames to both IPv4 and IPv6 addresses using `getaddrinfo()`.

4. **Port Scanning**: The core functionality uses different techniques for TCP and UDP scanning across IPv4 and IPv6.

5. **Multithreaded Scanning**: The program creates separate threads for each protocol (TCP/UDP) and IP address combination to perform scans in parallel.

6. **Signal Handling**: The program can be gracefully terminated using Ctrl+C.

### Core Scanning Techniques

#### TCP Scanning
- **IPv4**: Uses raw sockets to craft and send SYN packets, and libpcap to capture SYN+ACK or RST responses.
- **IPv6**: Uses non-blocking TCP sockets with timeouts to detect port states.

#### UDP Scanning
- **IPv4 and IPv6**: Sends UDP packets and checks for responses or ICMP "port unreachable" messages.

### Multithreading Approach

The scanner creates separate threads for each combination of:
- Target IP address (from hostname resolution)
- Protocol (TCP/UDP)

This allows for efficient parallel scanning of multiple targets and protocols. Each thread handles its assigned protocol's port ranges independently.

```
        ┌────────────────┐
        │ Main Thread    │
        └───────┬────────┘
                │
                ▼
        ┌────────────────┐
        │ Resolve Target │
        └───────┬────────┘
                │
                ▼
        ┌────────────────┐
        │ Create Threads │
        └───────┬────────┘
                │
                ▼
        ┌───────────────────────────┐
        │                           │
        ▼                           ▼
┌────────────────┐         ┌────────────────┐
│ TCP Thread     │         │ UDP Thread     │
│ (IP Address 1) │   ...   │ (IP Address N) │
└───────┬────────┘         └───────┬────────┘
        │                          │
        ▼                          ▼
┌────────────────┐         ┌────────────────┐
│ Scan TCP Ports │         │ Scan UDP Ports │
└───────┬────────┘         └───────┬────────┘
        │                          │
        ▼                          ▼
┌────────────────┐         ┌────────────────┐
│ Print Results  │         │ Print Results  │
└────────────────┘         └────────────────┘
```

## Testing

### Testing Environment

All tests were conducted in the following environment:

- **Operating System**: Arch Linux x86_64
- **Hardware**: A machine with 16 CPU cores and 32GB RAM
- **Network**: Kolejnet + VUT FIT VPN for IPv6 capabilities
- **Test Targets**:
  - Localhost (127.0.0.1 and ::1)
  - Local network devices (192.168.1.0/24)
  - Public web servers
  - Network devices with various firewall configurations

### Test Cases

#### Test Case 1: Basic TCP Scanning
**Description**: Scan common TCP ports on localhost.
**Command**: `sudo ./ipk-l4-scan -i lo -t 22,80,443 localhost`
**Expected Results**:
- Port 22: open (if SSH server is running)
- Port 80: closed or open (depending on whether a web server is running)
- Port 443: closed or open (depending on whether an HTTPS server is running)

**Actual Results**:
```
127.0.0.1 22 tcp open
127.0.0.1 80 tcp closed
127.0.0.1 443 tcp closed
```

#### Test Case 2: Basic UDP Scanning
**Description**: Scan common UDP ports on localhost.
**Command**: `sudo ./ipk-l4-scan -i lo -u 53,123,161 localhost`
**Expected Results**:
- Port 53: closed or open (depending on whether DNS server is running)
- Port 123: closed or open (depending on whether NTP server is running)
- Port 161: closed or open (depending on whether SNMP server is running)

**Actual Results**:
```
127.0.0.1 53 udp closed
127.0.0.1 123 udp closed
127.0.0.1 161 udp closed
```

#### Test Case 3: IPv6 Scanning
**Description**: Scan common TCP and UDP ports on IPv6 localhost.
**Command**: `sudo ./ipk-l4-scan -i lo -t 22,80 -u 53 ::1`
**Expected Results**: Similar to IPv4 localhost results.

**Actual Results**:
```
::1 22 tcp open
::1 80 tcp closed
::1 53 udp closed
```

#### Test Case 4: Remote Host Scanning
**Description**: Scan common ports on a public website.
**Command**: `sudo ./ipk-l4-scan -i eth0 -t 80,443 -w 2000 www.fit.vutbr.cz`
**Expected Results**:
- Port 80: open
- Port 443: open

**Actual Results**:
```
147.229.9.23 80 tcp open
147.229.9.23 443 tcp open
2001:67c:1220:809::93e5:917 80 tcp open
2001:67c:1220:809::93e5:917 443 tcp open
```

#### Test Case 5: Port Range Scanning
**Description**: Scan a range of TCP ports on localhost.
**Command**: `sudo ./ipk-l4-scan -i lo -t 20-25 localhost`
**Expected Results**: Status of ports 20-25 (FTP, SSH, Telnet, SMTP, etc.)

**Actual Results**:
```
127.0.0.1 20 tcp closed
127.0.0.1 21 tcp closed
127.0.0.1 22 tcp open
127.0.0.1 23 tcp closed
127.0.0.1 24 tcp closed
127.0.0.1 25 tcp closed
```

#### Test Case 6: Timeout Testing
**Description**: Test different timeout values.
**Command**: `sudo ./ipk-l4-scan -i eth0 -t 80 -w 1000 www.example.com`
**Expected Results**: Faster scan completion with reduced accuracy.

**Actual Results**:
```
93.184.216.34 80 tcp open
```

### Comparison with Nmap

To validate the accuracy of the scanner, results were compared with the Nmap scanner:

**My Scanner**:
```
sudo ./ipk-l4-scan -i eth0 -t 22,80,443 localhost
```
```
127.0.0.1 22 tcp open
127.0.0.1 80 tcp closed
127.0.0.1 443 tcp closed
```

**Nmap**:
```
nmap -sS -p 22,80,443 localhost
```
```
Starting Nmap 7.94 ( https://nmap.org ) at 2025-03-27 14:30 CET
Nmap scan report for localhost (127.0.0.1)
Host is up (0.000097s latency).

PORT    STATE  SERVICE
22/tcp  open   ssh
80/tcp  closed http
443/tcp closed https

Nmap done: 1 IP address (1 host up) scanned in
```

The results from my scanner match Nmap's findings, confirming the accuracy of my implementation.

## Extra Features

The scanner includes several features beyond the basic requirements:

1. **Multithreaded Scanning**: Parallel scanning of multiple targets and protocols for improved performance.

2. **Full IPv6 Support**: Comprehensive support for IPv6 addresses in both TCP and UDP scanning.

3. **Hostname Resolution**: Automatic resolution of hostnames to both IPv4 and IPv6 addresses, with support for scanning all resolved addresses.

4. **Port Range Specification**: Flexible port selection through ranges (e.g., "1-1000") and individual ports (e.g., "22,80,443").

5. **Verification of 'Filtered' Status**: For TCP scanning, the scanner sends a second SYN packet with reduced timeout to confirm filtered status.

6. **Graceful Termination**: Support for interrupting the scan process at any time with Ctrl+C.

7. **Detailed Interface Information**: When listing network interfaces, the scanner shows both IPv4 and IPv6 addresses.

## Known Limitations

1. **UDP Scanning Accuracy**: UDP scanning is less reliable than TCP scanning due to the nature of the protocol. My implementation marks ports as either "open" or "closed," with no separate "filtered" state for UDP.

2. **IPv6 TCP Scanning Method**: For IPv6 TCP scanning, the implementation uses non-blocking sockets rather than raw sockets, which may be slightly less stealthy than the IPv4 implementation. However, the handshake is never completed as the connection is closed after `SYN`.

3. **Root Privileges**: The scanner requires root privileges to create raw sockets and capture packets.

4. **Firewall Interference**: Network firewalls can affect scan results, particularly for UDP scanning.

5. **High-Volume Scanning**: When scanning large port ranges, the scanner may encounter resource limitations.

6. **Multithreading port scanning**: Unfortunatelly I ran into issues trying to scan every port in their own thread and thus multithread protocols only.

7. **Lack of commits**: There is an issue that prevents github mirroring to gitea. [404.png](https://git.fit.vutbr.cz/xbohach00/IPK_projekt_1/raw/branch/master/404.png) shows the issue I ran into and no ammount of github tokens would help.

## Bibliography

1. RFC 793: Transmission Control Protocol, 1981. [Online]. Request for Comments. Internet Engineering Task Force. Available from: https://datatracker.ietf.org/doc/html/rfc793

2. RFC 768: User Datagram Protocol, 1980. [Online]. Request for Comments. Internet Engineering Task Force. Available from: https://datatracker.ietf.org/doc/html/rfc768

3. DEERING, Steve E. and HINDEN, Bob, 2017. RFC 8200: Internet Protocol, Version 6 (IPv6) Specification. [Online]. Request for Comments. Internet Engineering Task Force. Available from: https://datatracker.ietf.org/doc/html/rfc8200

4. GORDON, Fyodor. "The Art of Port Scanning". Phrack Magazine, Volume 7, Issue 51, September 1997. Available from: http://phrack.org/issues/51/11.html

5. GORDON, Fyodor. Nmap Network Scanning: The Official Nmap Project Guide to Network Discovery and Security Scanning. Nmap Project, 2009. ISBN: 978-0979958717.

6. STEVENS, W. Richard, FENNER, Bill, and RUDOFF, Andrew M. UNIX Network Programming, Volume 1: The Sockets Networking API. Addison-Wesley Professional, 2003. ISBN: 978-0131411555.

7. BEEJ, Brian. "Beej's Guide to Network Programming". 2016. Available from: https://beej.us/guide/bgnet/

8. TCP SYN (Stealth) Scan (-sS) | Nmap Network Scanning. [Online]. Available from: https://nmap.org/book/synscan.html

9. "Port scanner". Wikipedia. 2024. [Online]. Available from: https://en.wikipedia.org/wiki/Port_scanner

10. SATRAPA, Pavel. IPv6: internetový protokol verze 6. CZ. NIC, 2019. ISBN: 978-80-88168-43-0
