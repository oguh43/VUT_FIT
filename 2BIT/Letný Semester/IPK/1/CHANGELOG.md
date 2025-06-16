### Features
- TCP scanning uses raw sockets with SYN packets for IPv4 and non-blocking sockets for IPv6
- UDP scanning with ICMP port unreachable detection
- Support for both individual ports and port ranges (e.g., 1-1000)
- Support for comma-separated port lists
- Proper handling of network interface binding
- Hostname resolution to both IPv4 and IPv6 addresses

### Limitations
- UDP Scanning Accuracy: UDP scanning is less reliable than TCP scanning due to the nature of the protocol. My implementation marks ports as either "open" or "closed," with no separate "filtered" state for UDP.
- IPv6 TCP Scanning Method: For IPv6 TCP scanning, the implementation uses non-blocking sockets rather than raw sockets, which may be slightly less stealthy than the IPv4 implementation. However, the handshake is never completed as the connection is closed after `SYN`.
- Root Privileges: The scanner requires root privileges to create raw sockets and capture packets.
- Firewall Interference: Network firewalls can affect scan results, particularly for UDP scanning.
- High-Volume Scanning: When scanning large port ranges, the scanner may encounter resource limitations.
- Multithreading port scanning: Unfortunatelly I ran into issues trying to scan every port in their own thread and thus multithread protocols only.
- Lack of commits: There is an issue that prevents github mirroring to gitea. [404.png](https://git.fit.vutbr.cz/xbohach00/IPK_projekt_1/raw/branch/master/404.png) shows the issue I ran into and no ammount of github tokens would help.