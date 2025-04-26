### **1. ifconfig**
**Purpose**:  
`ifconfig` is used to configure network interfaces. Although deprecated and replaced by `ip` on modern Linux systems, it’s still useful for legacy systems.

**Syntax**:  
```bash
ifconfig [interface] [options]
```

**Options**:
- **`ifconfig`**: Displays active interfaces.
- **`ifconfig -a`**: Displays all interfaces, including inactive ones.
- **`ifconfig eth0 up/down`**: Activates (`up`) or deactivates (`down`) the `eth0` interface.
- **`ifconfig eth0 192.168.1.10 netmask 255.255.255.0`**: Assigns an IP address and netmask.

**Examples**:
1. Display all active interfaces:
   ```bash
   ifconfig
   ```
2. Assign an IP to an interface:
   ```bash
   ifconfig eth0 192.168.1.50 netmask 255.255.255.0
   ```
3. Bring an interface up:
   ```bash
   ifconfig eth0 up
   ```

---

### **2. traceroute**
**Purpose**:  
`traceroute` is used to trace the route packets take to reach a target host. It identifies the intermediate hops and the time taken.

**Syntax**:  
```bash
traceroute [options] <hostname/IP>
```

**Options**:
- **`-n`**: Avoids DNS resolution for faster output.
- **`-m <max_hops>`**: Limits the number of hops.
- **`-I`**: Uses ICMP instead of UDP.

**Examples**:
1. Trace the route to Google:
   ```bash
   traceroute google.com
   ```
2. Limit the maximum hops to 5:
   ```bash
   traceroute -m 5 google.com
   ```
3. Skip DNS resolution for faster output:
   ```bash
   traceroute -n google.com
   ```

---

### **3. ping**
**Purpose**:  
`ping` is used to test connectivity to a host by sending ICMP Echo Request packets and measuring the response time.

**Syntax**:  
```bash
ping [options] <hostname/IP>
```

**Options**:
- **`-c <count>`**: Sends a specific number of packets.
- **`-i <interval>`**: Sets the interval between packets (in seconds).
- **`-t <ttl>`**: Specifies the time-to-live (TTL) value.

**Examples**:
1. Ping a host indefinitely:
   ```bash
   ping google.com
   ```
2. Send only 4 packets:
   ```bash
   ping -c 4 google.com
   ```
3. Set a custom interval of 2 seconds:
   ```bash
   ping -i 2 google.com
   ```

---

### **4. dig**
**Purpose**:  
`dig` (Domain Information Groper) is a flexible tool for querying DNS records, such as A, MX, NS, etc.

**Syntax**:  
```bash
dig [options] <hostname>
```

**Options**:
- **`<hostname>`**: Queries A records by default.
- **`<hostname> mx`**: Queries mail exchange (MX) records.
- **`+short`**: Outputs only the essential information.

**Examples**:
1. Query the A record for a domain:
   ```bash
   dig google.com
   ```
2. Query MX (Mail Exchange) records:
   ```bash
   dig google.com mx
   ```
3. Simplified output:
   ```bash
   dig google.com +short
   ```

---

### **5. telnet**
**Purpose**:  
`telnet` connects to a remote host on a specified port, often used for testing network services.

**Syntax**:  
```bash
telnet <hostname> <port>
```

**Examples**:
1. Test connectivity to a web server (port 80):
   ```bash
   telnet example.com 80
   ```
2. Test if an SSH server is running (port 22):
   ```bash
   telnet example.com 22
   ```

---

### **6. nslookup**
**Purpose**:  
`nslookup` queries DNS servers for information about a domain or IP address.

**Syntax**:  
```bash
nslookup [options] <hostname/IP>
```

**Examples**:
1. Query the IP address of a domain:
   ```bash
   nslookup google.com
   ```
2. Reverse lookup of an IP address:
   ```bash
   nslookup 8.8.8.8
   ```

---

### **7. netstat**
**Purpose**:  
`netstat` displays network connections, routing tables, and interface statistics.

**Syntax**:  
```bash
netstat [options]
```

**Options**:
- **`-a`**: Displays all connections (listening and non-listening).
- **`-tuln`**: Shows TCP/UDP connections without resolving hostnames.

**Examples**:
1. Display all network connections:
   ```bash
   netstat -a
   ```
2. Show only listening TCP/UDP ports:
   ```bash
   netstat -tuln
   ```

---

### **8. scp**
**Purpose**:  
`scp` (Secure Copy) securely transfers files between systems using SSH.

**Syntax**:  
```bash
scp [options] <source> <destination>
```

**Examples**:
1. Copy a file to a remote server:
   ```bash
   scp file.txt user@remote_host:/path/to/destination
   ```
2. Copy a directory recursively:
   ```bash
   scp -r /local_dir user@remote_host:/remote_dir
   ```

---

### **9. w**
**Purpose**:  
`w` displays information about currently logged-in users and their activities.

**Syntax**:  
```bash
w
```

**Example**:
1. View logged-in users:
   ```bash
   w
   ```

---

### **10. nmap**
**Purpose**:  
`nmap` is a powerful network scanner used for discovering devices, open ports, and vulnerabilities.

**Syntax**:  
```bash
nmap [options] <hostname/IP>
```

**Options**:
- **`-sS`**: Conduct a stealth SYN scan.
- **`-p <port>`**: Scan specific ports.
- **`-A`**: Enable OS and service detection.

**Examples**:
1. Scan all open ports:
   ```bash
   nmap google.com
   ```
2. Perform a service detection scan:
   ```bash
   nmap -A google.com
   ```

---

### **11. ifup/ifdown**
**Purpose**:  
Simple scripts to bring network interfaces up or down.

**Syntax**:  
```bash
ifup <interface>
ifdown <interface>
```

**Examples**:
1. Bring up eth0:
   ```bash
   ifup eth0
   ```
2. Bring down eth0:
   ```bash
   ifdown eth0
   ```

---

### **12. route**
**Purpose**:  
Display or modify the IP routing table.

**Syntax**:  
```bash
route [options] [add|del]
```

**Examples**:
1. Display routing table:
   ```bash
   route -n
   ```
2. Add default gateway:
   ```bash
   route add default gw 192.168.1.1
   ```

---

### **13. host**
**Purpose**:  
DNS lookup utility, simpler alternative to dig.

**Syntax**:  
```bash
host [options] hostname [server]
```

**Examples**:
1. Basic DNS lookup:
   ```bash
   host google.com
   ```
2. Query MX records:
   ```bash
   host -t mx google.com
   ```

---

### **14. arp**
**Purpose**:  
View or manipulate the ARP cache.

**Syntax**:  
```bash
arp [options]
```

**Examples**:
1. Display ARP table:
   ```bash
   arp -a
   ```
2. Delete an entry:
   ```bash
   arp -d 192.168.1.100
   ```

---

### **15. ethtool**
**Purpose**:  
Query or control network driver and hardware settings.

**Syntax**:  
```bash
ethtool [options] <interface>
```

**Examples**:
1. Show interface information:
   ```bash
   ethtool eth0
   ```
2. Set speed and duplex:
   ```bash
   ethtool -s eth0 speed 100 duplex full
   ```

---

### **16. iwconfig**
**Purpose**:  
Configure wireless network interfaces.

**Syntax**:  
```bash
iwconfig [interface] [options]
```

**Examples**:
1. Display wireless settings:
   ```bash
   iwconfig
   ```
2. Set wireless mode:
   ```bash
   iwconfig wlan0 mode managed
   ```

---

### **17. system-config-network**
**Purpose**:  
GUI tool for network configuration (Red Hat/Fedora).

**Syntax**:  
```bash
system-config-network
```

---

### **18. bmon**
**Purpose**:  
Bandwidth monitor and rate estimator.

**Syntax**:  
```bash
bmon [options]
```

**Examples**:
1. Monitor all interfaces:
   ```bash
   bmon
   ```

---

### **19. ssh**
**Purpose**:  
Secure Shell for remote system access.

**Syntax**:  
```bash
ssh [options] [user@]hostname [command]
```

**Examples**:
1. Connect to remote host:
   ```bash
   ssh user@example.com
   ```
2. Run remote command:
   ```bash
   ssh user@example.com 'ls -l'
   ```

---

### **20. tcpdump**
**Purpose**:  
Network packet analyzer.

**Syntax**:  
```bash
tcpdump [options] [filter]
```

**Examples**:
1. Capture packets on interface:
   ```bash
   tcpdump -i eth0
   ```
2. Filter HTTP traffic:
   ```bash
   tcpdump port 80
   ```

---

### **21. dstat**
**Purpose**:  
Versatile resource statistics tool.

**Syntax**:  
```bash
dstat [options]
```

**Examples**:
1. Show all stats:
   ```bash
   dstat -a
   ```

---

### **22. dhclient**
**Purpose**:  
DHCP client for dynamic IP configuration.

**Syntax**:  
```bash
dhclient [options] [interface]
```

**Examples**:
1. Request new IP:
   ```bash
   dhclient eth0
   ```
2. Release IP:
   ```bash
   dhclient -r eth0
   ```

---

### **23. nload**
**Purpose**:  
Real-time network traffic monitor.

**Syntax**:  
```bash
nload [options] [devices]
```

**Examples**:
1. Monitor eth0:
   ```bash
   nload eth0
   ```

---

### **24. iftop**
**Purpose**:  
Display bandwidth usage by hosts.

**Syntax**:  
```bash
iftop [options]
```

**Examples**:
1. Monitor interface:
   ```bash
   iftop -i eth0
   ```

---

### **25. ip**
**Purpose**:  
Modern replacement for ifconfig, route, etc.

**Syntax**:  
```bash
ip [options] object [command]
```

**Examples**:
1. Show addresses:
   ```bash
   ip addr show
   ```
2. Add route:
   ```bash
   ip route add 192.168.1.0/24 via 192.168.1.1
   ```

---

### **26. iptables**
**Purpose**:  
Firewall and NAT configuration tool.

**Syntax**:  
```bash
iptables [options] [chain] [rule]
```

**Examples**:
1. List rules:
   ```bash
   iptables -L
   ```
2. Allow incoming SSH:
   ```bash
   iptables -A INPUT -p tcp --dport 22 -j ACCEPT
   ```

---

### **27. sftp**
**Purpose**:  
Secure FTP over SSH.

**Syntax**:  
```bash
sftp [user@]host
```

**Examples**:
1. Connect to server:
   ```bash
   sftp user@example.com
   ```

---

### **28. socat**
**Purpose**:  
Multipurpose relay for bidirectional data transfer.

**Syntax**:  
```bash
socat [options] <address1> <address2>
```

**Examples**:
1. Create TCP listener:
   ```bash
   socat TCP-LISTEN:8080,fork STDOUT
   ```

---

### **29. rsync**
**Purpose**:  
Fast, versatile file copying tool.

**Syntax**:  
```bash
rsync [options] source destination
```

**Examples**:
1. Sync directories:
   ```bash
   rsync -av /source/ /destination/
   ```

---

### **30. wget**
**Purpose**:  
Non-interactive network downloader.

**Syntax**:  
```bash
wget [options] [URL]
```

**Examples**:
1. Download file:
   ```bash
   wget https://example.com/file.txt
   ```
2. Mirror website:
   ```bash
   wget -m https://example.com
   ```

---

### **31. curl**
**Purpose**:  
Transfer data from or to a server.

**Syntax**:  
```bash
curl [options] [URL]
```

**Examples**:
1. GET request:
   ```bash
   curl https://api.example.com
   ```
2. POST request:
   ```bash
   curl -X POST -d "data" https://api.example.com
   ```
