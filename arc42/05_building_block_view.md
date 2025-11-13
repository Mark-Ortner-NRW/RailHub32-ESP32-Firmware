# 5. Building Block View

## 5.1 Whitebox Overall System

### System Overview

```mermaid
graph TB
    subgraph "RailHub32 ESP32 Controller"
        direction TB
        
        subgraph "Presentation Layer"
            UI[Web Interface Module]
            API[REST API Module]
        end
        
        subgraph "Application Layer"
            AC[Accessory Control Module]
            SM[State Management Module]
            CM[Configuration Module]
        end
        
        subgraph "Infrastructure Layer"
            WM[WiFi Management Module]
            WS[Web Server Module]
            MD[mDNS Module]
            ST[Storage Module]
        end
        
        subgraph "Hardware Layer"
            GPIO[GPIO Control Module]
            NVS[NVS Storage Module]
            WIFI[WiFi Radio Module]
        end
    end
    
    UI --> API
    API --> AC
    AC --> SM
    AC --> CM
    
    SM --> ST
    CM --> ST
    
    WM --> WIFI
    WS --> API
    MD --> WIFI
    ST --> NVS
    
    AC --> GPIO
    
    style UI fill:#87CEEB
    style AC fill:#90EE90
    style WM fill:#FFE4B5
    style GPIO fill:#DDA0DD
```

### Top-Level Building Blocks

| Block | Responsibility | Interfaces |
|-------|---------------|------------|
| **Web Interface Module** | Serve HTML/CSS/JS, user interaction | HTTP GET / |
| **REST API Module** | Handle control requests, return status | HTTP GET/POST /api/* |
| **Accessory Control Module** | Business logic for accessory operations | processCommand(), getStatus() |
| **State Management Module** | Track and persist accessory states | updateState(), loadState() |
| **Configuration Module** | Manage system settings | getConfig(), saveConfig() |
| **WiFi Management Module** | Network connectivity, captive portal | connect(), configureAP() |
| **Web Server Module** | HTTP request routing | onRequest(), begin() |
| **mDNS Module** | Service discovery | advertise(), respond() |
| **Storage Module** | File system and NVS operations | read(), write(), persist() |
| **GPIO Control Module** | Low-level pin manipulation | setPin(), readPin() |
| **NVS Storage Module** | Non-volatile storage access | get(), set(), commit() |
| **WiFi Radio Module** | Hardware WiFi operations | scan(), associate() |

### Important Interfaces

```mermaid
sequenceDiagram
    participant User
    participant WebUI as Web Interface
    participant API as REST API
    participant Control as Accessory Control
    participant GPIO as GPIO Module
    participant State as State Manager
    
    User->>WebUI: Load Page
    WebUI->>API: GET /status
    API->>Control: getStatus()
    Control->>State: getCurrentStates()
    State-->>Control: State Array
    Control-->>API: JSON Status
    API-->>WebUI: 200 OK + JSON
    WebUI-->>User: Display UI
    
    User->>WebUI: Click Accessory
    WebUI->>API: POST /control
    API->>Control: processCommand()
    Control->>GPIO: setPin(pin, value)
    GPIO-->>Control: Success
    Control->>State: updateState()
    State-->>Control: Saved
    Control-->>API: Success
    API-->>WebUI: 200 OK
    WebUI-->>User: Update UI
```

## 5.2 Level 2: Web Server Module

### Whitebox Web Server Module

```mermaid
graph TB
    subgraph "Web Server Module"
        direction TB
        
        HTTP[HTTP Request Handler]
        ROUTE[Route Manager]
        STATIC[Static File Handler]
        JSON[JSON Parser/Generator]
        
        HTTP --> ROUTE
        ROUTE --> STATIC
        ROUTE --> JSON
    end
    
    subgraph "External Interfaces"
        CLIENT[HTTP Client]
        FILES[File System]
        HANDLERS[Request Handlers]
    end
    
    CLIENT <--> HTTP
    STATIC <--> FILES
    JSON <--> HANDLERS
    
    style HTTP fill:#87CEEB
    style ROUTE fill:#90EE90
```

### Building Blocks

| Block | Responsibility | Implementation |
|-------|---------------|----------------|
| **HTTP Request Handler** | Accept and process HTTP connections | ESPAsyncWebServer |
| **Route Manager** | Map URLs to handlers | AsyncWebServer::on() |
| **Static File Handler** | Serve HTML/CSS/JS files | serveStatic() |
| **JSON Parser/Generator** | Serialize/deserialize JSON | ArduinoJson |

### Internal Interfaces

```cpp
// Route registration
server.on("/", HTTP_GET, handleRoot);
server.on("/status", HTTP_GET, handleStatus);
server.on("/control", HTTP_POST, handleControl);
server.on("/config", HTTP_GET, handleGetConfig);
server.on("/config", HTTP_POST, handleSetConfig);

// Static file serving
server.serveStatic("/", SPIFFS, "/www/");

// Error handling
server.onNotFound(handleNotFound);
```

## 5.3 Level 2: Accessory Control Module

### Whitebox Accessory Control Module

```mermaid
graph TB
    subgraph "Accessory Control Module"
        direction TB
        
        CMD[Command Processor]
        VAL[Input Validator]
        CTRL[Control Logic]
        STAT[Status Reporter]
        
        CMD --> VAL
        VAL --> CTRL
        CTRL --> STAT
    end
    
    subgraph "Dependencies"
        GPIO[GPIO Module]
        STATE[State Manager]
        CONFIG[Configuration]
    end
    
    CTRL --> GPIO
    CTRL --> STATE
    VAL --> CONFIG
    STAT --> STATE
    
    style CMD fill:#90EE90
    style CTRL fill:#FFE4B5
```

### Building Blocks

| Block | Responsibility | Key Functions |
|-------|---------------|---------------|
| **Command Processor** | Parse and route commands | processCommand(id, state) |
| **Input Validator** | Validate command parameters | validateAccessoryId(), validateState() |
| **Control Logic** | Execute accessory operations | turnOn(), turnOff(), toggle() |
| **Status Reporter** | Generate status information | getAccessoryStatus(), getSystemStatus() |

### Control Flow

```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> Validating: Command Received
    Validating --> Executing: Valid
    Validating --> Error: Invalid
    Executing --> Persisting: GPIO Success
    Executing --> Error: GPIO Failure
    Persisting --> Reporting: State Saved
    Persisting --> Error: Save Failed
    Reporting --> Idle: Response Sent
    Error --> Idle: Error Response
```

### Implementation Example

```cpp
class AccessoryController {
private:
    GPIOManager& gpio;
    StateManager& state;
    ConfigManager& config;
    
public:
    bool processCommand(int accessoryId, bool newState) {
        // Validate input
        if (!validateAccessoryId(accessoryId)) {
            return false;
        }
        
        // Get GPIO pin for accessory
        int pin = config.getPinForAccessory(accessoryId);
        
        // Execute control
        if (!gpio.setPin(pin, newState)) {
            return false;
        }
        
        // Update state
        state.updateState(accessoryId, newState);
        
        return true;
    }
    
    JsonObject getStatus() {
        JsonObject status;
        status["hostname"] = config.getHostname();
        status["uptime"] = millis() / 1000;
        status["accessories"] = state.getAllStates();
        return status;
    }
};
```

## 5.4 Level 2: WiFi Management Module

### Whitebox WiFi Management Module

```mermaid
graph TB
    subgraph "WiFi Management Module"
        direction TB
        
        CONN[Connection Manager]
        AP[Access Point Mode]
        CP[Captive Portal]
        SCAN[Network Scanner]
        
        CONN --> SCAN
        CONN --> AP
        AP --> CP
    end
    
    subgraph "Dependencies"
        WIFI[WiFi Radio]
        DNS[DNS Server]
        WEB[Web Server]
        STOR[Credential Storage]
    end
    
    CONN --> WIFI
    AP --> DNS
    CP --> WEB
    CONN --> STOR
    
    style CONN fill:#87CEEB
    style AP fill:#FFE4B5
```

### Building Blocks

| Block | Responsibility | Library |
|-------|---------------|---------|
| **Connection Manager** | Establish and maintain WiFi connection | WiFi.h |
| **Access Point Mode** | Create temporary WiFi network | WiFiAP.h |
| **Captive Portal** | Configuration web interface | ESPAsyncWiFiManager |
| **Network Scanner** | Scan for available networks | WiFi.scanNetworks() |

### Connection State Machine

```mermaid
stateDiagram-v2
    [*] --> CheckCredentials
    CheckCredentials --> ConnectSTA: Credentials Found
    CheckCredentials --> StartAP: No Credentials
    
    ConnectSTA --> Connected: Success
    ConnectSTA --> StartAP: Failed (3 attempts)
    
    Connected --> Monitoring: mDNS Started
    Monitoring --> Reconnecting: Connection Lost
    Reconnecting --> Connected: Success
    Reconnecting --> StartAP: Failed
    
    StartAP --> CaptivePortal: AP Active
    CaptivePortal --> SaveCredentials: User Config
    SaveCredentials --> ConnectSTA: Credentials Saved
    
    Connected --> [*]: Running
```

## 5.5 Level 2: State Management Module

### Whitebox State Management Module

```mermaid
graph TB
    subgraph "State Management Module"
        direction TB
        
        CACHE[State Cache]
        SYNC[Synchronizer]
        LOAD[State Loader]
        SAVE[State Saver]
        
        CACHE --> SYNC
        SYNC --> LOAD
        SYNC --> SAVE
    end
    
    subgraph "Storage Backend"
        NVS[NVS Storage]
        RAM[RAM Cache]
    end
    
    LOAD --> NVS
    SAVE --> NVS
    CACHE --> RAM
    
    style CACHE fill:#90EE90
    style NVS fill:#DDA0DD
```

### Building Blocks

| Block | Responsibility | Storage Type |
|-------|---------------|--------------|
| **State Cache** | In-memory state tracking | RAM (fast access) |
| **Synchronizer** | Keep cache and NVS in sync | Coordination logic |
| **State Loader** | Load states from NVS on boot | NVS read operations |
| **State Saver** | Persist states to NVS | NVS write operations |

### State Lifecycle

```mermaid
sequenceDiagram
    participant Boot
    participant Loader as State Loader
    participant Cache as State Cache
    participant Control as Accessory Control
    participant Saver as State Saver
    participant NVS
    
    Boot->>Loader: System Start
    Loader->>NVS: Load All States
    NVS-->>Loader: State Data
    Loader->>Cache: Initialize Cache
    
    Note over Cache: System Running
    
    Control->>Cache: Update State(id, value)
    Cache->>Cache: Update RAM
    Cache->>Saver: Trigger Persist
    Saver->>NVS: Write State
    NVS-->>Saver: Confirmed
    Saver-->>Control: Success
```

### State Data Structure

```cpp
struct AccessoryState {
    uint8_t id;           // Accessory ID (1-16)
    bool state;           // Current state (ON/OFF)
    uint32_t lastChanged; // Timestamp of last change
    uint8_t pin;          // GPIO pin number
    char name[32];        // Accessory name
};

class StateManager {
private:
    AccessoryState states[MAX_ACCESSORIES];
    Preferences prefs;
    
public:
    void loadStates() {
        prefs.begin("states", false);
        for (int i = 0; i < MAX_ACCESSORIES; i++) {
            String key = "acc_" + String(i);
            states[i].state = prefs.getBool(key.c_str(), false);
        }
        prefs.end();
    }
    
    void updateState(uint8_t id, bool newState) {
        states[id].state = newState;
        states[id].lastChanged = millis();
        
        // Persist to NVS
        prefs.begin("states", false);
        String key = "acc_" + String(id);
        prefs.putBool(key.c_str(), newState);
        prefs.end();
    }
    
    AccessoryState getState(uint8_t id) {
        return states[id];
    }
};
```

## 5.6 Level 2: GPIO Control Module

### Whitebox GPIO Control Module

```mermaid
graph TB
    subgraph "GPIO Control Module"
        direction TB
        
        API[GPIO API]
        PIN[Pin Manager]
        OUT[Output Driver]
        PROT[Protection Layer]
        
        API --> PIN
        PIN --> PROT
        PROT --> OUT
    end
    
    subgraph "Hardware"
        PINS[Physical GPIO Pins]
    end
    
    OUT --> PINS
    
    style API fill:#90EE90
    style OUT fill:#FFE4B5
    style PINS fill:#DDA0DD
```

### Building Blocks

| Block | Responsibility | Safety Features |
|-------|---------------|-----------------|
| **GPIO API** | High-level pin operations | Input validation |
| **Pin Manager** | Track pin assignments and modes | Conflict detection |
| **Protection Layer** | Prevent hardware damage | Current limiting, validation |
| **Output Driver** | Low-level hardware control | Direct register access |

### Pin Configuration

```cpp
class GPIOManager {
private:
    struct PinConfig {
        uint8_t pin;
        uint8_t mode;      // INPUT, OUTPUT, INPUT_PULLUP
        bool inUse;
        uint8_t accessoryId;
    };
    
    PinConfig pins[34];    // ESP32 has 34 GPIO pins
    
    const uint8_t ACCESSORY_PINS[16] = {
        2, 4, 5, 12, 13, 14, 15, 16,
        17, 18, 19, 21, 22, 23, 25, 26
    };
    
public:
    void initializePins() {
        for (int i = 0; i < 16; i++) {
            uint8_t pin = ACCESSORY_PINS[i];
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            
            pins[pin].pin = pin;
            pins[pin].mode = OUTPUT;
            pins[pin].inUse = true;
            pins[pin].accessoryId = i;
        }
    }
    
    bool setPin(uint8_t pin, bool state) {
        // Validate pin
        if (!isValidPin(pin)) {
            return false;
        }
        
        // Check if pin is in use
        if (!pins[pin].inUse) {
            return false;
        }
        
        // Set pin state
        digitalWrite(pin, state ? HIGH : LOW);
        return true;
    }
    
    bool getPin(uint8_t pin) {
        if (!isValidPin(pin)) {
            return false;
        }
        return digitalRead(pin);
    }
    
private:
    bool isValidPin(uint8_t pin) {
        // Check if pin number is valid for ESP32
        if (pin >= 34) return false;
        
        // Check reserved pins (6-11 used for flash)
        if (pin >= 6 && pin <= 11) return false;
        
        return true;
    }
};
```

### Pin Safety Matrix

```mermaid
graph TB
    A[Pin Request] --> B{Valid Pin?}
    B -->|No| E1[Reject: Invalid Pin]
    B -->|Yes| C{Pin In Use?}
    C -->|No| E2[Reject: Unallocated]
    C -->|Yes| D{Safe State?}
    D -->|No| E3[Reject: Unsafe]
    D -->|Yes| F[Execute Command]
    
    F --> G{Success?}
    G -->|Yes| H[Return Success]
    G -->|No| E4[Reject: Hardware Error]
    
    style F fill:#90EE90
    style H fill:#90EE90
    style E1 fill:#FFB6C6
    style E2 fill:#FFB6C6
    style E3 fill:#FFB6C6
    style E4 fill:#FFB6C6
```

## 5.7 Component Dependencies

### Dependency Graph

```mermaid
graph TD
    A[Web Interface] --> B[REST API]
    B --> C[Accessory Control]
    C --> D[GPIO Manager]
    C --> E[State Manager]
    
    F[WiFi Manager] --> G[WiFi Radio]
    F --> H[Captive Portal]
    H --> B
    
    I[mDNS Service] --> G
    
    E --> J[NVS Storage]
    K[Configuration Manager] --> J
    
    L[Main Loop] --> A
    L --> F
    L --> I
    L --> C
    
    style C fill:#90EE90
    style D fill:#FFE4B5
    style E fill:#87CEEB
    style J fill:#DDA0DD
```

### Cross-Module Communication

| Caller | Callee | Interface Type | Protocol |
|--------|--------|----------------|----------|
| REST API → Accessory Control | Function call | Synchronous | C++ method |
| Accessory Control → GPIO Manager | Function call | Synchronous | C++ method |
| Accessory Control → State Manager | Function call | Synchronous | C++ method |
| State Manager → NVS Storage | Library call | Synchronous | Preferences API |
| WiFi Manager → Captive Portal | Callback | Asynchronous | ESPAsyncWiFiManager |
| Web Interface → REST API | HTTP | Asynchronous | JSON over HTTP |
