# 7. Deployment View

## 7.1 Infrastructure Overview

### Physical Deployment

```mermaid
graph TB
    subgraph "Home Network Environment"
        subgraph "Network Infrastructure"
            Router[WiFi Router<br/>192.168.1.1<br/>2.4 GHz]
            Switch[Network Switch<br/>Ethernet]
        end
        
        subgraph "Client Devices"
            PC[Desktop/Laptop<br/>Web Browser]
            Mobile[Mobile Device<br/>iOS/Android]
            Tablet[Tablet<br/>Browser]
        end
        
        subgraph "Model Railway Setup"
            ESP[ESP32 Controller<br/>RailHub32<br/>192.168.1.100]
            Power[5V Power Supply<br/>2A]
            Layout[Model Railway<br/>Turnouts, Signals, Lights]
        end
    end
    
    Router <--> PC
    Router <--> Mobile
    Router <--> Tablet
    Router <--> ESP
    
    Power --> ESP
    ESP --> Layout
    
    style ESP fill:#90EE90
    style Router fill:#87CEEB
    style Layout fill:#FFE4B5
```

### Deployment Topology

```mermaid
C4Deployment
    title Deployment Diagram - Model Railway Environment
    
    Deployment_Node(home, "Home Network", "Local Area Network") {
        Deployment_Node(router, "WiFi Router", "802.11n 2.4GHz") {
            Container(dhcp, "DHCP Server", "", "IP Address Assignment")
            Container(dns, "mDNS Responder", "", "Service Discovery")
        }
        
        Deployment_Node(client, "User Device", "Windows/Mac/iOS/Android") {
            Container(browser, "Web Browser", "Chrome/Firefox/Safari", "User Interface")
        }
        
        Deployment_Node(controller, "ESP32 Board", "Hardware Platform") {
            Container(firmware, "RailHub32 Firmware", "C++/Arduino", "Control System")
            ContainerDb(flash, "Flash Storage", "4MB", "Firmware + Data")
        }
        
        Deployment_Node(railway, "Model Railway", "Physical Infrastructure") {
            Container(accessories, "Accessories", "Hardware", "Turnouts, Signals, Lights")
        }
    }
    
    Rel(browser, firmware, "HTTP Requests", "WiFi 2.4GHz")
    Rel(firmware, dhcp, "DHCP Client", "UDP")
    Rel(firmware, dns, "mDNS Service", "Multicast")
    Rel(firmware, accessories, "Control Signals", "GPIO 3.3V")
```

## 7.2 Hardware Requirements

### ESP32 Controller Board

```mermaid
graph TB
    subgraph "ESP32 Development Board"
        MCU[ESP32-WROOM-32<br/>Dual-core 240MHz<br/>520KB SRAM<br/>4MB Flash]
        
        subgraph "Power"
            USB[USB Port<br/>5V Input]
            REG[Voltage Regulator<br/>3.3V Output]
            USB --> REG
        end
        
        subgraph "Programming"
            UART[USB-to-UART<br/>CP2102/CH340]
            BOOT[Boot Button]
            RESET[Reset Button]
        end
        
        subgraph "GPIO Expansion"
            PIN1[GPIO 2-5]
            PIN2[GPIO 12-19]
            PIN3[GPIO 21-27]
        end
        
        subgraph "WiFi"
            ANT[Antenna<br/>2.4GHz<br/>PCB/External]
        end
        
        REG --> MCU
        UART --> MCU
        BOOT --> MCU
        RESET --> MCU
        MCU --> PIN1
        MCU --> PIN2
        MCU --> PIN3
        MCU --> ANT
    end
    
    style MCU fill:#90EE90
    style REG fill:#FFE4B5
    style ANT fill:#87CEEB
```

| Component | Specification | Purpose |
|-----------|--------------|---------|
| **Microcontroller** | ESP32-WROOM-32 | Main processor |
| **CPU** | Dual-core Xtensa LX6 @ 240MHz | Application execution |
| **RAM** | 520 KB SRAM | Runtime memory |
| **Flash** | 4 MB SPI Flash | Firmware + data storage |
| **WiFi** | 802.11 b/g/n 2.4GHz | Network connectivity |
| **GPIO** | 34 pins (subset usable) | Accessory control |
| **Power** | 5V USB or VIN, 3.3V regulated | System power |
| **Programming** | USB-to-UART (CP2102/CH340) | Firmware upload |

### Recommended Boards

- **ESP32 DevKitC**: Standard development board, widely available
- **ESP32-WROOM-32**: Core module for custom PCB
- **NodeMCU-32S**: Alternative with similar pinout
- **ESP32-DevKitC-VE**: With external antenna connector

### Power Supply Requirements

```mermaid
graph LR
    subgraph "Power Distribution"
        AC[AC Mains<br/>110-240V] --> Adapter[AC/DC Adapter<br/>5V 2A]
        Adapter --> USB[USB Cable<br/>5V]
        USB --> Board[ESP32 Board]
        Board --> VReg[3.3V Regulator<br/>LDO]
        
        VReg --> ESP[ESP32 SoC<br/>~200mA]
        VReg --> GPIO[GPIO Outputs<br/>~100mA]
        
        Board --> ExtPower[External<br/>Accessory Power]
    end
    
    style Adapter fill:#FFE4B5
    style VReg fill:#87CEEB
    style ESP fill:#90EE90
```

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| **ESP32 Core** | 3.3V | 80-260mA | Variable with WiFi activity |
| **WiFi TX** | 3.3V | +170mA | Peak during transmission |
| **GPIO Outputs** | 3.3V | Max 40mA per pin | Total max 200mA |
| **Total System** | 5V | 500mA typical | 2A recommended for safety margin |

## 7.3 Network Deployment

### Network Architecture

```mermaid
graph TB
    subgraph "Internet"
        ISP[Internet Service Provider]
    end
    
    subgraph "Home Network - 192.168.1.0/24"
        Router[WiFi Router/Gateway<br/>192.168.1.1]
        
        subgraph "WiFi Clients - 2.4GHz"
            ESP[ESP32 RailHub32<br/>192.168.1.100<br/>railhub32-controller.local]
            Phone[Smartphone<br/>192.168.1.101]
            Laptop[Laptop<br/>192.168.1.102]
        end
        
        subgraph "Wired Clients"
            Desktop[Desktop PC<br/>192.168.1.10]
        end
    end
    
    ISP <--> Router
    Router <-.WiFi 2.4GHz.-> ESP
    Router <-.WiFi.-> Phone
    Router <-.WiFi.-> Laptop
    Router ---|Ethernet| Desktop
    
    Phone -.HTTP.-> ESP
    Laptop -.HTTP.-> ESP
    Desktop -.HTTP.-> ESP
    
    style ESP fill:#90EE90
    style Router fill:#87CEEB
```

### Network Configuration

| Setting | Value | Description |
|---------|-------|-------------|
| **IP Assignment** | DHCP (default) | Automatic IP from router |
| **Static IP** | Optional | Can be configured via router |
| **Subnet** | 192.168.1.0/24 | Typical home network |
| **Gateway** | 192.168.1.1 | Router IP address |
| **DNS** | Router/ISP | Not critical for local operation |
| **Hostname** | railhub32-{chipid} | Configurable, sanitized |
| **mDNS** | {hostname}.local | Automatic service discovery |
| **Port** | 80 (HTTP) | Web server port |

### Firewall Requirements

```mermaid
graph LR
    subgraph "Internet"
        Threat[External Threats]
    end
    
    subgraph "Router Firewall"
        FW[Firewall Rules]
    end
    
    subgraph "Local Network"
        ESP[ESP32 Controller]
        Clients[Client Devices]
    end
    
    Threat -.Blocked.-> FW
    FW -->|Allow| Clients
    FW -->|Allow| ESP
    
    Clients <-.Local Traffic Only.-> ESP
    
    style FW fill:#FFB6C6
    style ESP fill:#90EE90
```

**Firewall Rules:**
- ✅ Allow: Local network → ESP32 port 80 (HTTP)
- ✅ Allow: ESP32 → Router (DHCP, DNS)
- ✅ Allow: mDNS multicast (224.0.0.251:5353)
- ❌ Block: Internet → ESP32 (no port forwarding)
- ❌ Block: ESP32 → Internet (optional, for security)

## 7.4 Software Deployment

### Firmware Installation Methods

```mermaid
graph TB
    A[Firmware Binary<br/>firmware.bin] --> B{Installation Method}
    
    B --> C[USB Serial Flash]
    B --> D[OTA Update]
    
    C --> C1[Connect USB Cable]
    C1 --> C2[PlatformIO Upload]
    C2 --> C3[esptool.py]
    C3 --> C4[Firmware Flashed]
    
    D --> D1[Access Web Interface]
    D1 --> D2[Upload firmware.bin]
    D2 --> D3[ESP32 Reboots]
    D3 --> D4[New Version Running]
    
    style C fill:#87CEEB
    style D fill:#90EE90
```

#### Method 1: USB Serial Flash (Initial Setup)

```bash
# Using PlatformIO
pio run --target upload

# Using esptool.py
esptool.py --chip esp32 --port COM8 write_flash 0x10000 firmware.bin

# Using ESP Flash Download Tool (Windows GUI)
# - Select firmware.bin at address 0x10000
# - Click "Start" to flash
```

#### Method 2: Over-The-Air (OTA) Update

```mermaid
sequenceDiagram
    participant Dev as Developer
    participant Build as Build System
    participant Web as Web Browser
    participant ESP as ESP32
    
    Dev->>Build: Compile New Version
    Build-->>Dev: firmware.bin
    
    Dev->>Web: Open http://railhub32.local/update
    Web->>ESP: GET /update
    ESP-->>Web: Upload Form
    
    Dev->>Web: Select firmware.bin
    Dev->>Web: Click Upload
    
    Web->>ESP: POST firmware.bin
    ESP->>ESP: Write to Flash
    ESP-->>Web: Progress Updates
    
    ESP->>ESP: Verify & Reboot
    ESP-->>Web: Update Complete
    
    Note over ESP: Running new version
```

### Deployment Steps

#### Initial Deployment

```mermaid
flowchart TD
    A[Start] --> B[Flash Firmware via USB]
    B --> C[Power On ESP32]
    C --> D{Credentials Stored?}
    
    D -->|No| E[ESP32 Starts AP Mode]
    E --> F[Connect to RailHub32-Setup]
    F --> G[Configure WiFi via Captive Portal]
    G --> H[Save Credentials]
    H --> I[ESP32 Connects to Network]
    
    D -->|Yes| I
    
    I --> J[Access via http://railhub32.local]
    J --> K[Configure Accessories]
    K --> L[System Ready]
    
    style L fill:#90EE90
    style E fill:#FFE4B5
```

**Step-by-Step:**

1. **Prepare Hardware**
   - Connect ESP32 to computer via USB
   - Install USB drivers if needed (CP2102/CH340)

2. **Flash Firmware**
   ```bash
   cd esp32-controller
   pio run --target upload
   ```

3. **Initial Configuration**
   - ESP32 boots and starts AP mode
   - Connect to WiFi network "RailHub32-Setup"
   - Browser opens captive portal automatically
   - Select home WiFi network and enter password
   - Click "Save"

4. **Verify Deployment**
   - ESP32 connects to home network
   - Find IP address in serial monitor or router
   - Access web interface via mDNS: `http://railhub32-controller.local`

5. **Configure Accessories**
   - Name each accessory (Turnout 1, Signal A, etc.)
   - Test each GPIO output
   - Save configuration

#### Update Deployment

```bash
# Build new firmware
cd esp32-controller
pio run

# Locate firmware binary
# .pio/build/esp32dev/firmware.bin

# Upload via web interface
# http://railhub32.local/update
```

## 7.5 Physical Installation

### Wiring Diagram

```mermaid
graph TB
    subgraph "ESP32 Board"
        ESP[ESP32 DevKitC]
        P1[GPIO 2]
        P2[GPIO 4]
        P3[GPIO 5]
        P4[GPIO 12]
        PDOT[...]
        GND[GND]
        V5[5V]
    end
    
    subgraph "Power Supply"
        PSU[5V 2A Adapter]
        PGND[GND]
        P5V[+5V]
    end
    
    subgraph "Model Railway"
        T1[Turnout 1 Motor]
        S1[Signal 1 LED]
        L1[Layout Lighting]
        T2[Turnout 2 Motor]
        LDOT[...]
        LGND[Common GND]
    end
    
    PSU --> |USB Cable| ESP
    
    P1 --> |Control| T1
    P2 --> |Control| S1
    P3 --> |Control| L1
    P4 --> |Control| T2
    PDOT --> |Control| LDOT
    
    GND --> LGND
    T1 --> LGND
    S1 --> LGND
    L1 --> LGND
    T2 --> LGND
    LDOT --> LGND
    
    style ESP fill:#90EE90
    style PSU fill:#FFE4B5
```

### Installation Checklist

- [ ] **Power Supply**: 5V 2A minimum, quality adapter recommended
- [ ] **USB Cable**: Good quality, short length preferred
- [ ] **Mounting**: Secure ESP32 board away from moisture
- [ ] **GPIO Connections**: Proper gauge wire, soldered or screw terminals
- [ ] **Common Ground**: All accessories share common ground with ESP32
- [ ] **Current Limiting**: Resistors for LEDs, proper drivers for motors
- [ ] **Isolation**: Consider optocouplers for high-voltage accessories
- [ ] **Labels**: Mark all connections for troubleshooting
- [ ] **Cable Management**: Organize wires to prevent shorts
- [ ] **Ventilation**: Ensure adequate cooling for ESP32

### Safety Considerations

```mermaid
graph TB
    A[Safety Measures] --> B[Electrical Safety]
    A --> C[Component Protection]
    A --> D[User Safety]
    
    B --> B1[Proper Grounding]
    B --> B2[Fused Power Supply]
    B --> B3[No Exposed Conductors]
    
    C --> C1[Current Limiting Resistors]
    C --> C2[Flyback Diodes for Motors]
    C --> C3[ESD Protection]
    
    D --> D1[Low Voltage 5V/3.3V]
    D --> D2[Insulated Enclosure]
    D --> D3[Clear Labeling]
    
    style A fill:#FFB6C6
    style B1 fill:#90EE90
    style C1 fill:#90EE90
    style D1 fill:#90EE90
```

## 7.6 Scaling and Distribution

### Single Controller Deployment

```mermaid
graph TB
    subgraph "Small Layout"
        ESP1[ESP32 #1<br/>16 Accessories]
        L1[Layout Section A]
        L2[Layout Section B]
    end
    
    ESP1 --> L1
    ESP1 --> L2
    
    U1[User Device] -.WiFi.-> ESP1
    
    style ESP1 fill:#90EE90
```

### Multiple Controller Deployment (Future)

```mermaid
graph TB
    subgraph "Large Layout"
        subgraph "Zone 1"
            ESP1[ESP32 #1<br/>Section A<br/>192.168.1.100]
            L1[Accessories 1-16]
        end
        
        subgraph "Zone 2"
            ESP2[ESP32 #2<br/>Section B<br/>192.168.1.101]
            L2[Accessories 17-32]
        end
        
        subgraph "Zone 3"
            ESP3[ESP32 #3<br/>Section C<br/>192.168.1.102]
            L3[Accessories 33-48]
        end
    end
    
    Router[WiFi Router] <--> ESP1
    Router <--> ESP2
    Router <--> ESP3
    
    ESP1 --> L1
    ESP2 --> L2
    ESP3 --> L3
    
    User[User Device] -.-> Router
    
    style Router fill:#87CEEB
    style ESP1 fill:#90EE90
    style ESP2 fill:#90EE90
    style ESP3 fill:#90EE90
```

### Multi-Controller Considerations

| Aspect | Current (v1) | Future (v2+) |
|--------|-------------|--------------|
| **Coordination** | Manual, separate interfaces | Synchronized via MQTT |
| **Discovery** | Individual mDNS names | Central controller registry |
| **State Sync** | Independent | Shared state via message bus |
| **Failover** | N/A | Redundancy for critical sections |
| **Scalability** | Up to 16 accessories | Unlimited with multiple units |
