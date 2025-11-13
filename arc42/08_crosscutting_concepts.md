# 8. Cross-cutting Concepts

## 8.1 Domain Model

### Core Domain Entities

```mermaid
classDiagram
    class Accessory {
        +int id
        +String name
        +bool state
        +int pin
        +uint32_t lastChanged
        +turnOn()
        +turnOff()
        +toggle()
        +getState()
    }
    
    class Controller {
        +String hostname
        +String ipAddress
        +uint32_t uptime
        +int freeHeap
        +boot()
        +handleRequests()
        +getStatus()
    }
    
    class Configuration {
        +String wifiSSID
        +String wifiPassword
        +String hostname
        +PinMapping[] pins
        +load()
        +save()
        +reset()
    }
    
    class StateManager {
        +AccessoryState[] states
        +loadStates()
        +saveState()
        +updateState()
        +getAllStates()
    }
    
    class GPIOManager {
        +PinConfig[] pins
        +setPin()
        +getPin()
        +initialize()
    }
    
    Controller "1" --> "0..16" Accessory : controls
    Controller "1" --> "1" Configuration : uses
    Controller "1" --> "1" StateManager : uses
    Controller "1" --> "1" GPIOManager : uses
    StateManager "1" --> "0..16" Accessory : tracks
    GPIOManager "1" --> "0..16" Accessory : drives
```

### Value Objects and Entities

| Concept | Type | Description |
|---------|------|-------------|
| **Accessory** | Entity | Model railway component (turnout, signal, light) |
| **Controller** | Entity | ESP32 hardware instance |
| **Configuration** | Value Object | System settings (WiFi, hostname, pins) |
| **AccessoryState** | Value Object | Current state of accessory (ON/OFF) |
| **PinMapping** | Value Object | Accessory ID to GPIO pin mapping |
| **Command** | Value Object | User control command (id, state) |
| **Status** | Value Object | System status snapshot |

## 8.2 Security and Safety

### Security Architecture

```mermaid
graph TB
    subgraph "Security Layers"
        A[Network Security]
        B[Application Security]
        C[Data Security]
        D[Physical Security]
    end
    
    A --> A1[WPA2/WPA3 WiFi Encryption]
    A --> A2[Local Network Only]
    A --> A3[No Internet Exposure]
    
    B --> B1[Input Validation]
    B --> B2[Request Sanitization]
    B --> B3[Error Handling]
    
    C --> C1[NVS Encryption Optional]
    C --> C2[No Sensitive Data Storage]
    C --> C3[Config File Protection]
    
    D --> D1[Physical Access Required]
    D --> D2[USB Programming Lock]
    D --> D3[Enclosure Protection]
    
    style A fill:#FFB6C6
    style B fill:#87CEEB
    style C fill:#90EE90
    style D fill:#FFE4B5
```

### Security Principles

#### 1. Network Isolation

```mermaid
graph LR
    Internet[Internet] -.Blocked.-> Firewall[Router Firewall]
    Firewall -->|Allowed| Local[Local Network]
    Local <-->|Encrypted WiFi| ESP[ESP32 Controller]
    
    style Firewall fill:#FFB6C6
    style ESP fill:#90EE90
```

**Measures:**
- No port forwarding from internet
- WiFi WPA2/WPA3 encryption mandatory
- mDNS local network only
- No cloud connectivity required

#### 2. Input Validation

```cpp
// Example input validation
bool AccessoryController::validateCommand(JsonObject& cmd) {
    // Validate accessory ID
    if (!cmd.containsKey("accessory")) {
        return false;
    }
    int id = cmd["accessory"];
    if (id < 1 || id > MAX_ACCESSORIES) {
        return false;
    }
    
    // Validate state
    if (!cmd.containsKey("state")) {
        return false;
    }
    if (!cmd["state"].is<bool>()) {
        return false;
    }
    
    return true;
}
```

#### 3. Authentication Model

```mermaid
flowchart TD
    A[Authentication Model] --> B{Environment}
    
    B -->|Home Network| C[Trust Model]
    C --> C1[No Authentication]
    C --> C2[Physical Security]
    C --> C3[Network Security]
    
    B -->|Future: Public/Shared| D[Auth Required]
    D --> D1[Username/Password]
    D --> D2[API Keys]
    D --> D3[OAuth/SSO]
    
    style C fill:#90EE90
    style D fill:#FFE4B5
```

**Current (v1):**
- ❌ No authentication required
- ✅ Trust local network
- ✅ Physical access implies authorization
- ⚠️ Not suitable for shared networks

**Future (v2+):**
- Basic authentication (username/password)
- API key support for integrations
- Optional OAuth for home automation systems

### Safety Mechanisms

#### Hardware Protection

```mermaid
graph TB
    A[Hardware Safety] --> B[Overcurrent Protection]
    A --> C[Voltage Regulation]
    A --> D[ESD Protection]
    A --> E[Thermal Management]
    
    B --> B1[Current Limiting Resistors]
    B --> B2[GPIO Max 40mA per pin]
    B --> B3[Fused Power Supply]
    
    C --> C1[3.3V LDO Regulator]
    C --> C2[5V Input Protection]
    
    D --> D1[Input Diodes]
    D --> D2[Pull-up/Pull-down]
    
    E --> E1[Passive Cooling]
    E --> E2[Thermal Shutdown]
    
    style A fill:#FFB6C6
```

#### Software Safety

- **Watchdog Timer**: Automatic restart on hang (30s timeout)
- **Error Recovery**: Graceful degradation on component failure
- **State Persistence**: Prevents unexpected state on reboot
- **Input Bounds Checking**: Prevent out-of-range GPIO access

## 8.3 Error and Exception Handling

### Error Handling Strategy

```mermaid
graph TB
    A[Error Detection] --> B{Error Type}
    
    B --> C[Network Errors]
    C --> C1[WiFi Connection Lost]
    C --> C2[DHCP Failure]
    C --> C3[HTTP Server Error]
    
    B --> D[Hardware Errors]
    D --> D1[GPIO Failure]
    D --> D2[Flash Write Error]
    D --> D3[Memory Exhaustion]
    
    B --> E[Application Errors]
    E --> E1[Invalid Command]
    E --> E2[JSON Parse Error]
    E --> E3[Out of Range]
    
    C1 --> R1[Retry Connection]
    C2 --> R2[Fallback to AP Mode]
    C3 --> R3[Restart Server]
    
    D1 --> R4[Log & Continue]
    D2 --> R5[Retry Write]
    D3 --> R6[Garbage Collection]
    
    E1 --> R7[Return 400 Error]
    E2 --> R8[Return 400 Error]
    E3 --> R9[Return 400 Error]
    
    style C1 fill:#FFB6C6
    style D1 fill:#FFB6C6
    style E1 fill:#FFE4B5
```

### Error Response Format

```json
{
  "success": false,
  "error": {
    "code": "INVALID_ACCESSORY",
    "message": "Accessory ID must be between 1 and 16",
    "details": {
      "received": 99,
      "min": 1,
      "max": 16
    }
  }
}
```

### Exception Categories

| Category | Severity | Handling Strategy | User Impact |
|----------|----------|-------------------|-------------|
| **Critical** | System halt | Restart ESP32 | Service interruption |
| **Error** | Operation failure | Log & return error | Operation denied |
| **Warning** | Degraded function | Log & continue | Reduced functionality |
| **Info** | Normal operation | Log only | None |

### Logging Levels

```cpp
// Logging configuration
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  #define LOG_ERROR(msg) Serial.println("[ERROR] " + String(msg))
#else
  #define LOG_ERROR(msg)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
  #define LOG_WARNING(msg) Serial.println("[WARN] " + String(msg))
#else
  #define LOG_WARNING(msg)
#endif
```

## 8.4 Configuration Management

### Configuration Layers

```mermaid
graph TB
    subgraph "Configuration Hierarchy"
        A[Compile-Time Config]
        B[Flash Config Files]
        C[NVS Persistent Config]
        D[Runtime Config]
    end
    
    A --> A1[platformio.ini]
    A --> A2[config.h defines]
    A --> A3[Pin mappings]
    
    B --> B1[Web assets]
    B --> B2[Default settings]
    
    C --> C1[WiFi credentials]
    C --> C2[Hostname]
    C --> C3[Accessory states]
    
    D --> D1[Current IP]
    D --> D2[Connection status]
    D --> D3[Uptime]
    
    A -->|Compiled into| B
    B -->|Defaults for| C
    C -->|Loaded into| D
    
    style A fill:#87CEEB
    style C fill:#90EE90
    style D fill:#FFE4B5
```

### Configuration Sources

#### 1. Build-Time Configuration (platformio.ini)

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Build flags
build_flags = 
    -DCORE_DEBUG_LEVEL=0
    -DMAX_ACCESSORIES=16
    -DWIFI_TIMEOUT=10000
    
; Upload settings
upload_speed = 921600
monitor_speed = 115200
```

#### 2. Source Code Configuration (config.h)

```cpp
// config.h
#ifndef CONFIG_H
#define CONFIG_H

// GPIO Pin Mapping
const int GPIO_PINS[MAX_ACCESSORIES] = {
    2, 4, 5, 12, 13, 14, 15, 16,
    17, 18, 19, 21, 22, 23, 25, 26
};

// Network Configuration
#define DEFAULT_HOSTNAME "railhub32"
#define AP_SSID_PREFIX "RailHub32-Setup"
#define MDNS_ENABLED true

// Timeouts
#define WIFI_CONNECT_TIMEOUT 10000
#define HTTP_REQUEST_TIMEOUT 5000
#define STATE_SAVE_INTERVAL 1000

#endif
```

#### 3. Runtime Configuration (NVS)

```cpp
class ConfigManager {
private:
    Preferences prefs;
    
public:
    void saveWiFiCredentials(String ssid, String password) {
        prefs.begin("wifi", false);
        prefs.putString("ssid", ssid);
        prefs.putString("password", password);
        prefs.end();
    }
    
    void loadWiFiCredentials(String& ssid, String& password) {
        prefs.begin("wifi", true);
        ssid = prefs.getString("ssid", "");
        password = prefs.getString("password", "");
        prefs.end();
    }
    
    void setHostname(String hostname) {
        prefs.begin("config", false);
        prefs.putString("hostname", hostname);
        prefs.end();
    }
};
```

## 8.5 Observability and Monitoring

### Monitoring Strategy

```mermaid
graph TB
    A[Observability] --> B[Logging]
    A --> C[Metrics]
    A --> D[Health Checks]
    
    B --> B1[Serial Console]
    B --> B2[System Events]
    B --> B3[Error Logs]
    
    C --> C1[Uptime]
    C --> C2[Free Heap]
    C --> C3[Request Count]
    C --> C4[WiFi RSSI]
    
    D --> D1[WiFi Connected]
    D --> D2[Web Server Running]
    D --> D3[GPIO Functional]
    
    style A fill:#87CEEB
```

### System Metrics

```cpp
struct SystemMetrics {
    uint32_t uptime;          // Milliseconds since boot
    uint32_t freeHeap;        // Free RAM in bytes
    int8_t wifiRSSI;          // WiFi signal strength (dBm)
    uint32_t requestCount;    // Total HTTP requests
    uint32_t errorCount;      // Total errors
    float cpuTemp;            // CPU temperature (°C)
};

SystemMetrics getMetrics() {
    SystemMetrics metrics;
    metrics.uptime = millis();
    metrics.freeHeap = ESP.getFreeHeap();
    metrics.wifiRSSI = WiFi.RSSI();
    metrics.cpuTemp = temperatureRead();
    return metrics;
}
```

### Health Check Endpoint

```json
GET /health

{
  "status": "healthy",
  "checks": {
    "wifi": {
      "status": "connected",
      "rssi": -45,
      "ip": "192.168.1.100"
    },
    "web_server": {
      "status": "running",
      "requests": 1234
    },
    "memory": {
      "status": "ok",
      "free_heap": 180000,
      "heap_fragmentation": 5
    },
    "storage": {
      "status": "ok",
      "flash_used": 60
    }
  },
  "uptime": 3600
}
```

## 8.6 Testability

### Testing Strategies

```mermaid
graph TB
    A[Testing Levels] --> B[Unit Testing]
    A --> C[Integration Testing]
    A --> D[System Testing]
    A --> E[Manual Testing]
    
    B --> B1[GPIO Functions]
    B --> B2[State Management]
    B --> B3[JSON Parsing]
    
    C --> C1[WiFi + Web Server]
    C --> C2[API + Control Logic]
    C --> C3[Storage + State]
    
    D --> D1[Full Boot Sequence]
    D --> D2[End-to-End Scenarios]
    D --> D3[Error Recovery]
    
    E --> E1[Hardware Testing]
    E --> E2[User Acceptance]
    E --> E3[Network Scenarios]
    
    style B fill:#90EE90
    style C fill:#87CEEB
    style D fill:#FFE4B5
    style E fill:#DDA0DD
```

### Testability Features

- **Modular Design**: Independent components can be tested in isolation
- **Dependency Injection**: Mock dependencies for unit tests
- **Serial Debugging**: Rich debug output via serial console
- **Test Endpoints**: Special HTTP endpoints for testing (disabled in production)
- **State Inspection**: Ability to dump and inspect internal state

### Mock Infrastructure

```cpp
// Mock GPIO for testing
class MockGPIO : public IGPIOManager {
private:
    bool pinStates[34];
    
public:
    bool setPin(uint8_t pin, bool state) override {
        if (pin >= 34) return false;
        pinStates[pin] = state;
        return true;
    }
    
    bool getPin(uint8_t pin) override {
        if (pin >= 34) return false;
        return pinStates[pin];
    }
    
    // Test helper
    bool getPinState(uint8_t pin) {
        return pinStates[pin];
    }
};
```

## 8.7 Performance and Scalability

### Performance Characteristics

```mermaid
graph TB
    A[Performance Metrics] --> B[Response Time]
    A --> C[Throughput]
    A --> D[Resource Usage]
    
    B --> B1[HTTP Request: <20ms]
    B --> B2[GPIO Control: <1ms]
    B --> B3[State Save: <15ms]
    
    C --> C1[Max 50 req/s GET]
    C --> C2[Max 25 req/s POST]
    C --> C3[Concurrent: 10 clients]
    
    D --> D1[RAM: <100KB used]
    D --> D2[CPU: <30% average]
    D --> D3[Flash: <1.5MB firmware]
    
    style B1 fill:#90EE90
    style C1 fill:#90EE90
    style D1 fill:#90EE90
```

### Scalability Limits

| Resource | Limit | Constraint |
|----------|-------|------------|
| **Accessories** | 16 | GPIO pin availability |
| **Concurrent Clients** | 10-15 | RAM + CPU |
| **Requests/Second** | 25-50 | Async server capacity |
| **State Updates/Sec** | 100 | NVS write speed |
| **WiFi Range** | 30-50m | Hardware limitation |
| **Firmware Size** | 1.5 MB | Flash partition |

### Optimization Techniques

```cpp
// 1. Minimize dynamic allocation
static char buffer[256];  // Reusable buffer

// 2. Use const for string literals
const char* const ERROR_MSG = "Invalid request";

// 3. Efficient JSON serialization
StaticJsonDocument<512> doc;  // Stack allocation, fixed size

// 4. Async operations
server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request){
    // Non-blocking handler
});

// 5. Batch state saves
#define STATE_SAVE_INTERVAL 1000  // Save at most once per second
```

## 8.8 Development and Build Process

### Build Pipeline

```mermaid
flowchart LR
    A[Source Code] --> B[PlatformIO Build]
    B --> C[Compile C++]
    C --> D[Link Libraries]
    D --> E[Generate Binary]
    E --> F{Target}
    
    F -->|USB| G[Serial Flash]
    F -->|OTA| H[HTTP Upload]
    
    G --> I[Running System]
    H --> I
    
    style B fill:#87CEEB
    style I fill:#90EE90
```

### Dependency Management

```ini
; platformio.ini
[env:esp32dev]
lib_deps = 
    ESP Async WebServer @ 3.6.0
    ESPAsyncWiFiManager @ 0.31.0
    ArduinoJson @ 7.3.0
    ESP32 Arduino Core @ 3.x
```

### Versioning Strategy

```cpp
// Version information
#define FIRMWARE_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

String getVersionInfo() {
    return "RailHub32 v" + String(FIRMWARE_VERSION) + 
           " (" + String(BUILD_DATE) + " " + String(BUILD_TIME) + ")";
}
```
