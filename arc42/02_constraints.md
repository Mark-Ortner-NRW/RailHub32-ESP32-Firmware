# 2. Architecture Constraints

## 2.1 Technical Constraints

### Hardware Constraints

| Constraint | Description | Impact |
|------------|-------------|--------|
| **ESP32 CPU** | Dual-core Xtensa LX6, 240 MHz | Limited processing power for complex algorithms |
| **RAM** | 520 KB SRAM | Memory-constrained, careful allocation needed |
| **Flash Memory** | 4 MB typical | Limited space for firmware and web assets |
| **GPIO Pins** | 34 pins (subset usable) | Limited I/O expansion, careful pin planning |
| **WiFi** | 2.4 GHz only | No 5 GHz support, potential interference |
| **Power Supply** | 3.3V logic, 5V input | Requires voltage regulation |

```mermaid
graph TB
    subgraph "ESP32 Hardware Limits"
        A[ESP32 SoC] --> B[520 KB RAM]
        A --> C[4 MB Flash]
        A --> D[240 MHz CPU]
        A --> E[34 GPIO Pins]
        A --> F[2.4 GHz WiFi]
    end
    
    B --> B1[Careful Memory Management]
    C --> C1[Compressed Web Assets]
    D --> D1[Efficient Algorithms]
    E --> E1[Pin Multiplexing]
    F --> F1[Channel Selection]
    
    style A fill:#FFE4B5
    style B1 fill:#FFB6C6
    style C1 fill:#FFB6C6
    style D1 fill:#FFB6C6
    style E1 fill:#FFB6C6
    style F1 fill:#FFB6C6
```

### Software Constraints

| Constraint | Description | Rationale |
|------------|-------------|-----------|
| **Arduino Framework** | ESP32 Arduino Core v3.x | Simplicity, community support, extensive libraries |
| **PlatformIO** | Build system and dependency management | Cross-platform, version control, library management |
| **C++** | Programming language | Performance, hardware access, Arduino compatibility |
| **Async Web Server** | ESPAsyncWebServer library | Non-blocking I/O, better performance |
| **No RTOS Tasks** | Single-loop architecture | Simplicity, easier debugging |

```mermaid
graph LR
    A[Software Stack] --> B[Arduino Framework]
    B --> C[ESP32 Core 3.x]
    C --> D[FreeRTOS]
    
    A --> E[Libraries]
    E --> F[ESPAsyncWebServer]
    E --> G[ESPAsyncWiFiManager]
    E --> H[ArduinoJson]
    E --> I[ESPmDNS]
    
    A --> J[Build System]
    J --> K[PlatformIO]
    K --> L[platformio.ini]
    
    style B fill:#90EE90
    style K fill:#90EE90
```

## 2.2 Organizational Constraints

### Development Constraints

| Constraint | Description | Impact |
|------------|-------------|--------|
| **Open Source** | GPL-3.0 License | All code must be open, derivatives must share-alike |
| **GitHub Repository** | Version control and collaboration | Public development, issue tracking required |
| **Documentation** | English language, Markdown format | Accessible to international contributors |
| **Code Style** | Arduino/C++ conventions | Consistency, readability for Arduino users |
| **No Commercial Support** | Community-driven project | Reliance on volunteer contributions |

```mermaid
graph TD
    A[Organizational Structure] --> B[Open Source Model]
    B --> C[GPL-3.0 License]
    B --> D[Public Repository]
    B --> E[Community Support]
    
    A --> F[Development Process]
    F --> G[Git Version Control]
    F --> H[Issue Tracking]
    F --> I[Pull Requests]
    
    A --> J[Documentation]
    J --> K[Markdown Docs]
    J --> L[Code Comments]
    J --> M[arc42 Architecture]
    
    style B fill:#87CEEB
    style F fill:#87CEEB
    style J fill:#87CEEB
```

### Team Constraints

- **Team Size**: Small core team + community contributors
- **Time Availability**: Part-time development, volunteer basis
- **Skill Levels**: Mixed experience (beginners to experts)
- **Communication**: Asynchronous, GitHub-based
- **Testing**: Limited dedicated testing resources

## 2.3 Conventions

### Coding Conventions

```cpp
// File naming: lowercase with underscores
// main.cpp, wifi_manager.cpp

// Class naming: PascalCase
class AccessoryController {
    // Member variables: camelCase with m_ prefix
    int m_pinNumber;
    bool m_isActive;
    
    // Methods: camelCase
    void turnOn();
    void turnOff();
    bool getStatus();
};

// Constants: UPPER_CASE
#define MAX_ACCESSORIES 16
const int DEFAULT_TIMEOUT = 5000;

// Function naming: camelCase
void setupWiFiConnection();
void handleWebRequest();
```

### Documentation Conventions

- **Code Comments**: Inline for complex logic, function headers for public APIs
- **Markdown**: All documentation in Markdown format
- **Diagrams**: Mermaid for architecture diagrams
- **README**: User-focused, quick start guide
- **arc42**: Detailed architecture documentation

```mermaid
graph LR
    A[Documentation Levels] --> B[Code Comments]
    A --> C[README.md]
    A --> D[arc42 Docs]
    A --> E[API Reference]
    
    B --> B1[Inline Comments]
    B --> B2[Function Headers]
    
    C --> C1[Quick Start]
    C --> C2[Features]
    C --> C3[Examples]
    
    D --> D1[Architecture]
    D --> D2[Decisions]
    D --> D3[Quality Goals]
    
    E --> E1[HTTP Endpoints]
    E --> E2[Request/Response]
    
    style A fill:#DDA0DD
```

## 2.4 Legal Constraints

### Licensing

- **Firmware**: GPL-3.0 License
- **Hardware Design**: Open Hardware (if applicable)
- **Third-party Libraries**: Compatible licenses (MIT, Apache 2.0, LGPL)
- **Documentation**: Creative Commons CC-BY-SA 4.0

### Compliance

```mermaid
graph TB
    A[Legal Compliance] --> B[Software Licenses]
    B --> C[GPL-3.0 Firmware]
    B --> D[MIT Libraries]
    B --> E[Apache 2.0 Libraries]
    
    A --> F[Hardware Compliance]
    F --> G[CE Marking]
    F --> H[FCC Compliance]
    F --> I[RoHS Directive]
    
    A --> J[WiFi Regulations]
    J --> K[2.4 GHz ISM Band]
    J --> L[Regional Restrictions]
    
    A --> M[Safety Standards]
    M --> N[Low Voltage Directive]
    M --> O[Electrical Safety]
    
    style C fill:#FFE4B5
    style G fill:#FFE4B5
    style K fill:#FFE4B5
    style N fill:#FFE4B5
```

### Regional Constraints

- **WiFi Channels**: Compliance with regional channel restrictions
- **Transmission Power**: Within legal limits for 2.4 GHz band
- **Certification**: End users responsible for commercial certification
- **Export Control**: No export restrictions on open-source software

## 2.5 Infrastructure Constraints

### Network Requirements

| Requirement | Specification | Constraint |
|-------------|---------------|------------|
| **WiFi Standard** | 802.11 b/g/n | 2.4 GHz only, no 5 GHz |
| **Security** | WPA2/WPA3 | Open networks supported but discouraged |
| **IP Address** | DHCP or Static | Must support DHCP for ease of use |
| **DNS** | mDNS (Bonjour) | Requires mDNS-capable network |
| **Firewall** | HTTP port 80 | Must allow incoming HTTP connections |

```mermaid
graph TB
    subgraph "Network Infrastructure"
        A[WiFi Router] --> B[2.4 GHz Network]
        B --> C[DHCP Server]
        B --> D[mDNS Responder]
        
        C --> E[IP Assignment]
        D --> F[hostname.local]
        
        B --> G[ESP32 Controller]
        G --> H[Web Server :80]
        G --> I[mDNS Service]
    end
    
    subgraph "Client Devices"
        J[Browser] --> K[HTTP Client]
        K --> L[mDNS Discovery]
    end
    
    L -.Discovers.-> I
    K -.HTTP Request.-> H
    
    style B fill:#87CEEB
    style G fill:#90EE90
```

### Development Environment

- **IDE**: VS Code with PlatformIO extension
- **Build Tools**: PlatformIO CLI, espressif32 platform
- **Version Control**: Git 2.x+
- **Programming Tools**: USB-to-Serial adapter, ESP32 flash tool
- **Testing**: Web browsers (Chrome, Firefox, Safari)

### Deployment Constraints

- **Flash Method**: USB serial or OTA updates
- **Configuration**: Web interface or WiFi Manager
- **Persistent Storage**: SPIFFS/LittleFS filesystem
- **Backup**: No automatic backup, manual export required
- **Updates**: Manual OTA upload via web interface

```mermaid
sequenceDiagram
    participant Dev as Developer
    participant Git as GitHub Repo
    participant PIO as PlatformIO
    participant ESP as ESP32 Device
    participant User as End User
    
    Dev->>Git: Push Code
    Dev->>PIO: Build Firmware
    PIO->>PIO: Compile & Link
    PIO-->>Dev: firmware.bin
    
    alt Initial Flash
        Dev->>ESP: USB Serial Upload
    else OTA Update
        Dev->>ESP: HTTP Upload
        ESP->>ESP: Flash & Reboot
    end
    
    ESP->>User: Updated System
    
    Note over ESP,User: User can also perform<br/>OTA updates independently
```

## 2.6 Development Tool Constraints

### Required Tools

- **PlatformIO Core**: 6.1.0+
- **ESP32 Platform**: espressif32 @ 6.12.0
- **Arduino Framework**: arduino-esp32 @ 3.20017.241212
- **Python**: 3.7+ (for PlatformIO)
- **Git**: Any recent version

### Optional Tools

- **Serial Monitor**: PlatformIO, Arduino IDE, PuTTY
- **HTTP Testing**: Postman, curl, browser DevTools
- **Network Analysis**: Wireshark, nmap
- **Documentation**: Mermaid CLI, Markdown editors

```mermaid
graph TD
    A[Development Tools] --> B[Required]
    A --> C[Optional]
    
    B --> B1[VS Code + PlatformIO]
    B --> B2[Git]
    B --> B3[Python 3.7+]
    B --> B4[USB Driver]
    
    C --> C1[Serial Monitor]
    C --> C2[HTTP Testing]
    C --> C3[Network Tools]
    C --> C4[Documentation Tools]
    
    B1 --> D[Build & Flash]
    B2 --> E[Version Control]
    B3 --> F[Build System]
    B4 --> G[Device Communication]
    
    style B fill:#90EE90
    style C fill:#FFE4B5
```
