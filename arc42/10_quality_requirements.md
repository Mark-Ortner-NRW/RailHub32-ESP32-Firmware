# 10. Quality Requirements

## 10.1 Quality Tree

```mermaid
graph TB
    Q[Quality Requirements] --> Q1[Reliability]
    Q --> Q2[Usability]
    Q --> Q3[Performance]
    Q --> Q4[Maintainability]
    Q --> Q5[Security]
    Q --> Q6[Portability]
    
    Q1 --> Q1A[Continuous Operation]
    Q1 --> Q1B[Error Recovery]
    Q1 --> Q1C[Data Persistence]
    
    Q2 --> Q2A[Easy Setup]
    Q2 --> Q2B[Intuitive Interface]
    Q2 --> Q2C[Clear Feedback]
    
    Q3 --> Q3A[Low Latency]
    Q3 --> Q3B[Fast Boot]
    Q3 --> Q3C[Efficient Resources]
    
    Q4 --> Q4A[Code Quality]
    Q4 --> Q4B[Documentation]
    Q4 --> Q4C[Modularity]
    
    Q5 --> Q5A[Network Security]
    Q5 --> Q5B[Input Validation]
    Q5 --> Q5C[Safe Defaults]
    
    Q6 --> Q6A[Platform Support]
    Q6 --> Q6B[Browser Compat]
    Q6 --> Q6C[Hardware Variants]
    
    style Q fill:#87CEEB
    style Q1 fill:#90EE90
    style Q2 fill:#FFE4B5
    style Q3 fill:#DDA0DD
    style Q4 fill:#FFB6C6
```

## 10.2 Quality Scenarios

### Reliability Scenarios

#### R1: 24/7 Operation

```mermaid
sequenceDiagram
    participant User
    participant System as ESP32 System
    participant Monitor as Watchdog
    
    User->>System: Start Operation
    
    loop Continuous Operation
        System->>System: Normal Operation
        System->>Monitor: Heartbeat (every 30s)
        Monitor->>Monitor: Check Health
    end
    
    Note over System,Monitor: System runs 720+ hours
    
    System->>User: Stable Operation
    
    alt System Hang Detected
        Monitor->>System: Watchdog Timeout
        System->>System: Automatic Restart
        System->>System: Restore Last State
        System->>User: Service Resumed
    end
```

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | System running continuously |
| **Response** | No crashes or hangs |
| **Measure** | MTBF > 720 hours (30 days) |
| **Priority** | 🔴 Critical |

#### R2: Power Failure Recovery

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Unexpected power loss |
| **Response** | Restore previous state on reboot |
| **Measure** | State restored in < 5 seconds |
| **Priority** | 🟡 High |

```cpp
// State persistence ensures recovery
void setup() {
    stateManager.loadStates();  // Load from NVS
    gpioManager.restoreStates();  // Apply to hardware
    // System resumes where it left off
}
```

#### R3: Network Disconnection

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | WiFi connection lost |
| **Response** | Automatic reconnection attempts |
| **Measure** | Reconnect within 30 seconds, max 3 retries |
| **Priority** | 🟡 High |

```mermaid
stateDiagram-v2
    [*] --> Connected
    Connected --> Disconnected: Connection Lost
    Disconnected --> Reconnecting: Attempt 1
    Reconnecting --> Connected: Success
    Reconnecting --> Reconnecting: Retry (5s delay)
    Reconnecting --> FallbackAP: 3 Failures
    FallbackAP --> Reconnecting: New Credentials
```

### Usability Scenarios

#### U1: First-Time Setup

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | New user powers on device |
| **Response** | Guided WiFi configuration |
| **Measure** | Complete setup in < 5 minutes |
| **Priority** | 🔴 Critical |

**Success Criteria**:
- ✅ Captive portal opens automatically
- ✅ WiFi networks listed clearly
- ✅ Password entry straightforward
- ✅ Success/failure clearly indicated

#### U2: Accessory Control

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | User clicks accessory button |
| **Response** | Immediate visual and physical feedback |
| **Measure** | Response time < 100ms, visual update < 200ms |
| **Priority** | 🟡 High |

```mermaid
sequenceDiagram
    participant User
    participant UI as Web Interface
    participant API
    participant GPIO
    
    User->>UI: Click "Turnout 1 ON"
    Note over UI: T=0ms
    UI->>UI: Optimistic UI Update
    Note over UI: T=10ms (feels instant)
    
    UI->>API: POST /control
    API->>GPIO: Set Pin HIGH
    GPIO-->>API: Success
    Note over GPIO: T=30ms
    
    API-->>UI: Confirm Success
    UI->>UI: Final State Update
    Note over UI: T=80ms
    
    UI-->>User: Action Complete
```

#### U3: Error Recovery

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Invalid user input |
| **Response** | Clear error message with guidance |
| **Measure** | Error message appears < 1 second |
| **Priority** | 🟢 Medium |

**Error Message Guidelines**:
- ✅ Clear, non-technical language
- ✅ Explain what went wrong
- ✅ Suggest corrective action
- ✅ Visual distinction (color, icon)

### Performance Scenarios

#### P1: Command Latency

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | User sends control command |
| **Response** | GPIO state changes |
| **Measure** | End-to-end latency < 100ms |
| **Priority** | 🟡 High |

```mermaid
gantt
    title Command Processing Timeline
    dateFormat SSS
    axisFormat %L ms
    
    section Network
    WiFi RTT         :net, 000, 10ms
    
    section Server
    Route Lookup     :route, after net, 1ms
    JSON Parse       :parse, after route, 5ms
    Validate         :valid, after parse, 2ms
    
    section Hardware
    GPIO Command     :gpio, after valid, 1ms
    
    section Storage
    State Persist    :state, after gpio, 15ms
    
    section Response
    JSON Build       :json, after state, 3ms
    Send Response    :resp, after json, 5ms
    
    Total: 42ms (well within 100ms target)
```

#### P2: Boot Time

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Power on or reset |
| **Response** | System fully operational |
| **Measure** | Boot time < 5 seconds |
| **Priority** | 🟢 Medium |

**Boot Sequence Breakdown**:
- Hardware boot: 100ms
- WiFi connection: 2000ms (variable)
- State restoration: 100ms
- Web server start: 50ms
- mDNS registration: 100ms
- **Total**: ~2.5 seconds typical

#### P3: Concurrent Users

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | 5 users access system simultaneously |
| **Response** | All requests handled without blocking |
| **Measure** | Response time < 200ms for all users |
| **Priority** | 🟢 Medium |

```mermaid
graph TB
    subgraph "Async Web Server"
        S[Main Thread]
        S --> H1[Handler 1]
        S --> H2[Handler 2]
        S --> H3[Handler 3]
        S --> H4[Handler 4]
        S --> H5[Handler 5]
    end
    
    H1 --> R1[Response 1]
    H2 --> R2[Response 2]
    H3 --> R3[Response 3]
    H4 --> R4[Response 4]
    H5 --> R5[Response 5]
    
    style S fill:#87CEEB
    style R1 fill:#90EE90
    style R2 fill:#90EE90
    style R3 fill:#90EE90
    style R4 fill:#90EE90
    style R5 fill:#90EE90
```

### Maintainability Scenarios

#### M1: Code Comprehension

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | New developer reads codebase |
| **Response** | Understands architecture and can contribute |
| **Measure** | Comprehension in < 2 hours |
| **Priority** | 🟡 High |

**Supporting Elements**:
- ✅ arc42 architecture documentation
- ✅ Inline code comments
- ✅ README with quick start
- ✅ Mermaid diagrams for visualization
- ✅ Clear naming conventions

#### M2: Feature Addition

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Add new API endpoint |
| **Response** | Implementation without breaking existing features |
| **Measure** | < 4 hours for simple endpoint |
| **Priority** | 🟢 Medium |

```cpp
// Adding new endpoint is straightforward
server.on("/new-feature", HTTP_GET, [](AsyncWebServerRequest *request){
    // Implementation here
    request->send(200, "application/json", "{\"status\":\"ok\"}");
});
```

#### M3: Bug Fix

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Bug reported with stack trace |
| **Response** | Locate, fix, and test bug |
| **Measure** | < 2 hours for typical bug |
| **Priority** | 🟡 High |

**Debugging Tools**:
- Serial console logging
- HTTP request inspection
- State dump endpoints
- GPIO state monitoring

### Security Scenarios

#### S1: Local Network Attack

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Malicious device on local network |
| **Response** | Limit attack surface |
| **Measure** | No unauthorized control possible |
| **Priority** | 🟡 High |

**Mitigations**:
- ✅ Input validation on all endpoints
- ✅ No authentication bypass
- ✅ Rate limiting (planned)
- ✅ No command injection vulnerabilities

#### S2: Malformed Request

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Invalid JSON or out-of-range parameters |
| **Response** | Reject request without crash |
| **Measure** | 400 Bad Request returned |
| **Priority** | 🔴 Critical |

```cpp
// Input validation example
if (!request->hasParam("accessory") || !request->hasParam("state")) {
    request->send(400, "application/json", 
        "{\"error\":\"Missing required parameters\"}");
    return;
}

int id = request->getParam("accessory")->value().toInt();
if (id < 1 || id > MAX_ACCESSORIES) {
    request->send(400, "application/json", 
        "{\"error\":\"Accessory ID out of range\"}");
    return;
}
```

#### S3: WiFi Security

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | Network traffic interception attempt |
| **Response** | WiFi encryption protects traffic |
| **Measure** | WPA2/WPA3 encryption enforced |
| **Priority** | 🔴 Critical |

### Portability Scenarios

#### P1: Browser Compatibility

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | User accesses from different browsers |
| **Response** | Consistent functionality and appearance |
| **Measure** | Works on 95% of modern browsers |
| **Priority** | 🟡 High |

**Supported Browsers**:
- ✅ Chrome/Edge (Chromium) 90+
- ✅ Firefox 88+
- ✅ Safari 14+
- ✅ Mobile Chrome (Android)
- ✅ Mobile Safari (iOS)

#### P2: ESP32 Board Variants

| Aspect | Specification |
|--------|---------------|
| **Stimulus** | User has different ESP32 board variant |
| **Response** | Code compiles and runs with minimal changes |
| **Measure** | < 5 configuration changes required |
| **Priority** | 🟢 Medium |

**Compatible Boards**:
- ESP32-WROOM-32
- ESP32-WROVER
- ESP32 DevKitC
- NodeMCU-32S
- TTGO T-Display (with pin adjustments)

## 10.3 Quality Measures

### Reliability Metrics

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| **MTBF** | > 720 hours | Extended runtime testing |
| **Crash Recovery** | < 5 seconds | Watchdog reset timing |
| **State Persistence** | 100% | Power cycle testing |
| **WiFi Reconnection** | > 95% | Network disruption tests |

### Usability Metrics

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| **Setup Time** | < 5 minutes | User testing with novices |
| **Error Rate** | < 5% | Task completion studies |
| **User Satisfaction** | > 4/5 | Post-use surveys |
| **Learning Curve** | < 30 minutes | Time to first successful control |

### Performance Metrics

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| **Command Latency** | < 100ms | HTTP timing measurements |
| **Boot Time** | < 5 seconds | Serial console timestamps |
| **Memory Usage** | < 200KB RAM | ESP.getFreeHeap() |
| **CPU Utilization** | < 30% average | Task profiling |
| **Throughput** | > 25 req/s | Load testing |

### Maintainability Metrics

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| **Code Comments** | > 20% | Static analysis |
| **Function Length** | < 50 lines avg | Code metrics |
| **Cyclomatic Complexity** | < 10 per function | Static analysis |
| **Documentation Coverage** | > 90% | Manual review |

### Security Metrics

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| **Input Validation** | 100% endpoints | Code review |
| **Vulnerability Scan** | 0 high/critical | Security scanning |
| **WiFi Encryption** | WPA2+ only | Connection testing |
| **Attack Surface** | Minimal | Threat modeling |

## 10.4 Quality Assurance Activities

### Testing Strategy

```mermaid
graph TB
    A[Testing Activities] --> B[Unit Testing]
    A --> C[Integration Testing]
    A --> D[System Testing]
    A --> E[Acceptance Testing]
    
    B --> B1[GPIO Functions]
    B --> B2[State Management]
    B --> B3[JSON Parsing]
    
    C --> C1[WiFi + WebServer]
    C --> C2[API + Control]
    C --> C3[Storage + Persistence]
    
    D --> D1[End-to-End Scenarios]
    D --> D2[Performance Testing]
    D --> D3[Security Testing]
    
    E --> E1[User Acceptance]
    E --> E2[Field Testing]
    E --> E3[Beta Program]
    
    style A fill:#87CEEB
    style D fill:#90EE90
```

### Code Quality Checks

- **Static Analysis**: PlatformIO check
- **Code Review**: All PRs reviewed by maintainer
- **Style Guide**: Consistent formatting (clang-format)
- **Memory Profiling**: Heap usage monitoring
- **Performance Profiling**: Execution time analysis

### Continuous Improvement

```mermaid
graph LR
    A[User Feedback] --> B[Issue Tracking]
    B --> C[Prioritization]
    C --> D[Implementation]
    D --> E[Testing]
    E --> F[Release]
    F --> G[Monitoring]
    G --> A
    
    style A fill:#FFE4B5
    style D fill:#90EE90
    style F fill:#87CEEB
```

**Feedback Channels**:
- GitHub Issues
- Community Forum
- User Surveys
- Beta Testing Program
- Serial Console Logs
