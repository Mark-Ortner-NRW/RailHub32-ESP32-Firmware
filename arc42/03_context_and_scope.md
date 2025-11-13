# 3. System Scope and Context

## 3.1 Business Context

### System Overview

RailHub32 operates as a standalone WiFi-connected controller that bridges the gap between digital control interfaces (web browsers) and physical model railway accessories.

```mermaid
graph TB
    subgraph "External World"
        U[User with Device]
        R[WiFi Router]
        M[Model Railway Layout]
    end
    
    subgraph "RailHub32 System Boundary"
        W[Web Interface]
        C[ESP32 Controller]
        G[GPIO Outputs]
    end
    
    U -->|HTTP Requests| W
    W <-->|WiFi Network| R
    R <-->|WiFi Connection| C
    C -->|Control Signals| G
    G -->|Power & Control| M
    M -->|State Feedback| G
    
    style C fill:#90EE90
    style W fill:#87CEEB
    style M fill:#FFE4B5
```

### External Entities

| Entity | Description | Interface | Protocol |
|--------|-------------|-----------|----------|
| **User Browser** | Web-based control interface | HTTP Client | HTTP/1.1 |
| **WiFi Router** | Network infrastructure | 802.11 n | WiFi, DHCP, mDNS |
| **Model Railway** | Physical accessories (turnouts, signals, etc.) | GPIO Pins | Digital I/O |
| **Power Supply** | 5V DC power source | Power Jack | DC 5V, 2A |
| **Configuration Device** | Initial WiFi setup | Captive Portal | HTTP |

```mermaid
C4Context
    title Business Context Diagram
    
    Person(user, "Model Railway Operator", "Controls layout via web interface")
    Person(admin, "System Administrator", "Configures and maintains system")
    
    System(railhub, "RailHub32 ESP32 Controller", "WiFi-based model railway control system")
    
    System_Ext(router, "WiFi Router", "Network connectivity")
    System_Ext(railway, "Model Railway Layout", "Physical accessories and infrastructure")
    System_Ext(power, "Power Supply", "5V DC power source")
    
    Rel(user, railhub, "Controls via browser", "HTTP")
    Rel(admin, railhub, "Configures", "HTTP/Captive Portal")
    Rel(railhub, router, "Connects to", "WiFi 2.4GHz")
    Rel(railhub, railway, "Controls", "GPIO signals")
    Rel(power, railhub, "Powers", "DC 5V")
    
    UpdateRelStyle(user, railhub, $offsetX="-50", $offsetY="-20")
    UpdateRelStyle(railhub, railway, $offsetX="-50", $offsetY="20")
```

## 3.2 Technical Context

### System Interfaces

```mermaid
graph TB
    subgraph "User Layer"
        B1[Web Browser<br/>Chrome/Firefox/Safari]
        B2[Mobile Browser<br/>iOS/Android]
    end
    
    subgraph "Network Layer"
        N1[WiFi 802.11n<br/>2.4 GHz]
        N2[mDNS/Bonjour<br/>Service Discovery]
        N3[DHCP Client<br/>IP Assignment]
    end
    
    subgraph "Application Layer"
        A1[HTTP Server<br/>Port 80]
        A2[REST API<br/>JSON]
        A3[Static Web Assets<br/>HTML/CSS/JS]
    end
    
    subgraph "Control Layer"
        C1[GPIO Driver<br/>Digital Output]
        C2[State Manager<br/>Accessory Status]
        C3[Preferences<br/>NVS Storage]
    end
    
    subgraph "Hardware Layer"
        H1[ESP32 SoC<br/>Dual Core]
        H2[WiFi Radio<br/>2.4 GHz]
        H3[GPIO Pins<br/>34 pins]
        H4[Flash Memory<br/>4 MB]
    end
    
    B1 --> A1
    B2 --> A1
    A1 --> N1
    N1 --> H2
    
    N2 --> A1
    N3 --> A1
    
    A1 --> A2
    A2 --> C1
    C1 --> H3
    
    A3 --> H4
    C3 --> H4
    
    A2 --> C2
    C2 --> C3
    
    style H1 fill:#FFE4B5
    style A1 fill:#87CEEB
    style C1 fill:#90EE90
```

### Interface Specifications

#### 1. HTTP Web Interface

| Property | Specification |
|----------|--------------|
| **Protocol** | HTTP/1.1 |
| **Port** | 80 (standard) |
| **Methods** | GET, POST |
| **Content-Type** | application/json, text/html |
| **Authentication** | None (local network only) |
| **CORS** | Enabled for local access |

**API Endpoints:**

```mermaid
graph LR
    A[HTTP API] --> B[GET /]
    A --> C[GET /status]
    A --> D[POST /control]
    A --> E[GET /config]
    A --> F[POST /config]
    
    B --> B1[Web Interface HTML]
    C --> C1[System Status JSON]
    D --> D1[Control Commands]
    E --> E1[Configuration JSON]
    F --> F1[Update Settings]
    
    style A fill:#87CEEB
```

**Request/Response Examples:**

```json
// GET /status
{
  "hostname": "railhub32-controller",
  "ip": "192.168.1.100",
  "uptime": 3600,
  "accessories": [
    {"id": 1, "name": "Turnout 1", "state": true},
    {"id": 2, "name": "Signal A", "state": false}
  ]
}

// POST /control
{
  "accessory": 1,
  "state": true
}

// Response
{
  "success": true,
  "accessory": 1,
  "state": true
}
```

#### 2. WiFi Network Interface

```mermaid
sequenceDiagram
    participant ESP as ESP32
    participant Router as WiFi Router
    participant DNS as mDNS Service
    participant Client as User Device
    
    Note over ESP,Router: Initial Connection
    ESP->>Router: WiFi Association
    Router->>ESP: DHCP Offer
    ESP->>Router: DHCP Request
    Router->>ESP: IP Address Assigned
    
    Note over ESP,DNS: Service Discovery
    ESP->>DNS: Register hostname.local
    DNS->>DNS: Store mDNS Record
    
    Note over Client,DNS: Client Discovery
    Client->>DNS: Query hostname.local
    DNS->>Client: IP Address Response
    Client->>ESP: HTTP Connection
    ESP->>Client: Web Interface
```

| Property | Specification |
|----------|--------------|
| **Standard** | IEEE 802.11 b/g/n |
| **Frequency** | 2.4 GHz |
| **Security** | WPA2-PSK, WPA3-SAE |
| **IP Mode** | DHCP Client (default), Static optional |
| **Hostname** | Configurable, default: "railhub32-{chipid}" |
| **mDNS** | Enabled, {hostname}.local |

#### 3. GPIO Hardware Interface

```mermaid
graph TB
    subgraph "ESP32 GPIO"
        G1[GPIO 2] --> A1[Accessory 1]
        G2[GPIO 4] --> A2[Accessory 2]
        G3[GPIO 5] --> A3[Accessory 3]
        G4[GPIO 12] --> A4[Accessory 4]
        G5[GPIO 13] --> A5[Accessory 5]
        G6[GPIO 14] --> A6[Accessory 6]
        G7[GPIO 15] --> A7[Accessory 7]
        G8[GPIO 16] --> A8[Accessory 8]
        GDOT[...] --> ADOT[...]
    end
    
    subgraph "Model Railway"
        A1 --> T1[Turnout Motor]
        A2 --> S1[Signal LED]
        A3 --> L1[Lighting]
        A4 --> T2[Turnout Motor]
        A5 --> S2[Signal LED]
        A6 --> R1[Relay Module]
        A7 --> R2[Relay Module]
        A8 --> D1[Decoder]
        ADOT --> ETC[Additional Accessories]
    end
    
    style G1 fill:#90EE90
    style A1 fill:#FFE4B5
```

| Pin | Function | Voltage | Current | Protection |
|-----|----------|---------|---------|------------|
| GPIO 2-27 | Digital Output | 3.3V | Max 40mA | Current limiting resistors |
| GND | Common Ground | 0V | - | - |
| 5V | Power Supply | 5V | Max 2A | Fuse/overcurrent protection |

**Pin Assignment:**

```cpp
// Default GPIO pin mapping
const int GPIO_PINS[] = {
    2, 4, 5, 12, 13, 14, 15, 16,  // Accessories 1-8
    17, 18, 19, 21, 22, 23, 25, 26 // Accessories 9-16
};

// Signal characteristics
// - Logic HIGH: 3.3V (Accessory ON)
// - Logic LOW: 0V (Accessory OFF)
// - Switching speed: < 1ms
// - Pull-up/down: Configurable per pin
```

#### 4. Storage Interface

```mermaid
graph TB
    subgraph "Flash Memory 4MB"
        P1[Partition 1<br/>Firmware<br/>~1.5 MB]
        P2[Partition 2<br/>File System<br/>~2 MB]
        P3[Partition 3<br/>NVS Storage<br/>~500 KB]
    end
    
    P1 --> F1[Application Code]
    P1 --> F2[Arduino Libraries]
    
    P2 --> F3[Web Assets]
    P2 --> F4[Static Files]
    
    P3 --> F5[WiFi Credentials]
    P3 --> F6[Accessory States]
    P3 --> F7[Configuration]
    
    style P1 fill:#FFB6C6
    style P2 fill:#87CEEB
    style P3 fill:#90EE90
```

| Storage Type | Technology | Capacity | Use Case |
|-------------|------------|----------|----------|
| **Program Flash** | SPI NOR Flash | ~1.5 MB | Firmware binary |
| **File System** | LittleFS/SPIFFS | ~2 MB | Web assets, logs |
| **NVS** | Non-Volatile Storage | ~500 KB | Config, credentials, state |

## 3.3 External Dependencies

```mermaid
graph TB
    A[RailHub32 System] --> B[External Dependencies]
    
    B --> C[Hardware Dependencies]
    C --> C1[ESP32 SoC]
    C --> C2[WiFi Router]
    C --> C3[Power Supply 5V]
    C --> C4[Model Railway Accessories]
    
    B --> D[Network Dependencies]
    D --> D1[DHCP Server]
    D --> D2[mDNS Support]
    D --> D3[WiFi 2.4GHz Network]
    
    B --> E[Software Dependencies]
    E --> E1[Arduino Framework]
    E --> E2[ESPAsyncWebServer]
    E --> E3[ESPAsyncWiFiManager]
    E --> E4[ArduinoJson]
    E --> E5[ESPmDNS]
    
    B --> F[User Dependencies]
    F --> F1[Modern Web Browser]
    F --> F2[Basic Network Knowledge]
    F --> F3[WiFi-capable Device]
    
    style A fill:#90EE90
    style C fill:#FFE4B5
    style D fill:#87CEEB
    style E fill:#DDA0DD
    style F fill:#FFB6C6
```

### Critical External Systems

| System | Criticality | Failure Impact | Mitigation |
|--------|-------------|----------------|------------|
| **WiFi Router** | High | No remote control, local operation only | Fallback to AP mode |
| **Power Supply** | Critical | Complete system failure | Capacitor backup for clean shutdown |
| **Web Browser** | High | No user interface | Support multiple browsers |
| **mDNS Service** | Medium | Manual IP entry required | Display IP on serial console |
| **Model Railway** | Medium | No physical control | Software still operational |

## 3.4 Context Boundaries

### What the System Does

✅ **In Scope:**
- Control up to 16 digital outputs via WiFi
- Provide web-based user interface
- Store accessory states persistently
- Support WiFi configuration via captive portal
- Advertise services via mDNS
- Respond to HTTP control commands
- Monitor and report system status

### What the System Does NOT Do

❌ **Out of Scope:**
- Analog signal generation or measurement
- DCC (Digital Command Control) protocol
- Battery power operation
- 5 GHz WiFi support
- Bluetooth connectivity
- Train speed control
- Track occupancy detection
- Automatic train routing
- Multi-controller synchronization (current version)
- Cloud connectivity or remote access

```mermaid
graph TB
    subgraph "System Boundary"
        direction TB
        A[RailHub32 Controller]
        A --> B[Web Interface]
        A --> C[WiFi Manager]
        A --> D[GPIO Control]
        A --> E[mDNS Service]
        A --> F[State Storage]
    end
    
    subgraph "External - In Scope"
        G[WiFi Network]
        H[Web Browser]
        I[Model Railway Accessories]
    end
    
    subgraph "External - Out of Scope"
        J[DCC Decoders]
        K[Cloud Services]
        L[Train Sensors]
        M[Other Controllers]
    end
    
    B <--> H
    C <--> G
    D <--> I
    
    J -.Not Supported.-> A
    K -.Not Supported.-> A
    L -.Not Supported.-> A
    M -.Not Supported.-> A
    
    style A fill:#90EE90
    style J fill:#FFB6C6
    style K fill:#FFB6C6
    style L fill:#FFB6C6
    style M fill:#FFB6C6
```
