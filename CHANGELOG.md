# Changelog

All notable changes to the RailHub32/8266 Firmware project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2025-11-14

### Added - Major Feature Release

#### ESP32 (RailHub32) and ESP8266 (RailHub8266)
- **WebSocket Server**: Real-time bidirectional communication on port 81
  - Automatic status broadcasts every 500ms
  - Push updates on any output or configuration change
  - Support for multiple simultaneous clients
  - Automatic reconnection handling
  - Event-driven architecture
  - JSON format matching `/api/status` endpoint

- **Blink Interval Control**: Per-output blinking at configurable intervals
  - Configurable from 0ms (solid) to 65535ms (~65 seconds)
  - Non-blocking implementation using `millis()`
  - Independent timing for each output
  - NVRAM/EEPROM persistence
  - Visual indicators in web UI (orange border for blinking outputs)
  - API endpoint: `POST /api/interval`

- **Enhanced Web Interface**:
  - Real-time updates via WebSocket (no page refresh needed)
  - Live status monitoring with 500ms broadcast interval
  - Blink interval controls per output
  - Visual feedback for blinking outputs
  - Improved responsiveness and user experience

#### ESP8266 (RailHub8266) Exclusive
- **Chasing Light Groups**: Sequential output activation for dynamic effects
  - Support for up to 4 independent chasing groups
  - Assign up to 8 outputs per group
  - Configurable step interval (10-65535ms)
  - Custom group names (up to 20 characters)
  - EEPROM persistence for all group data
  - Dedicated "Chasing" tab in web UI
  - Non-blocking sequencer implementation
  - Three new API endpoints:
    - `POST /api/chasing/create` - Create new chasing group
    - `POST /api/chasing/delete` - Delete chasing group
    - `POST /api/chasing/name` - Rename chasing group

### Changed
- **ESP32**: Updated main.cpp from 1,868 lines to 2,301 lines (+433 lines)
- **ESP8266**: Updated main.cpp from 1,833 lines to 1,769 lines (+custom chasing logic)
- **Memory Usage**:
  - ESP32 Flash: 905 KB → 949 KB (WebSocket library addition)
  - ESP32 RAM: 48 KB → 52 KB (WebSocket buffers)
  - ESP8266 Flash: 408 KB → 429 KB
  - ESP8266 RAM: 77 KB → 76.8 KB (optimized)
- **Web Interface**: Enhanced with WebSocket integration, ~1,200 → ~1,500 lines
- **Dependencies**: Added `WebSockets @ 2.4.1` library for both platforms

### Improved
- **Real-time Synchronization**: All clients see changes instantly via WebSocket
- **Network Efficiency**: Reduced bandwidth compared to HTTP polling
- **User Experience**: Eliminated 500ms polling delay, instant UI updates
- **Documentation**: Added comprehensive feature documentation:
  - `WEBSOCKET_FEATURE.md` - Complete WebSocket guide
  - `CHASING_LIGHTS_FEATURE.md` - ESP8266 chasing lights documentation
  - Updated `BLINK_INTERVAL_FEATURE.md` with v2.0 enhancements
  - Updated all README files with new features

### Technical Details
- **WebSocket Port**: 81 (both platforms)
- **Broadcast Interval**: 500ms (configurable via `BROADCAST_INTERVAL`)
- **Client Support**: 4-6 clients (ESP32), 2-3 clients (ESP8266)
- **Message Format**: JSON matching `/api/status` endpoint
- **Latency**: <50ms from state change to client update

### Documentation Updates
- Updated `README.md` with v2.0 features and WebSocket documentation
- Updated `ESP32_vs_ESP8266_Comparison.md` with feature parity table
- Updated `ESTIMATION.md` with v2.0 development effort (20-28 person-days)
- Updated `ESP32Flasher/README.md` for v2.0 compatibility
- Created comprehensive WebSocket, Chasing Lights, and Blink Interval guides

---

## [1.0.0] - 2025-11-13

### Added - Initial Release

#### Core Features (Both Platforms)
- **WiFi Configuration Portal**: ESPAsyncWiFiManager integration
  - Captive portal for easy WiFi setup
  - Password-protected configuration (8+ characters)
  - Network scanning and selection
  - Automatic reconnection
  - Portal trigger button (GPIO 0, 3-second press)

- **PWM Output Control**:
  - ESP32: 16 independent PWM outputs
  - ESP8266: 8 independent PWM outputs
  - 8-bit brightness control (0-255 / 0-100%)
  - 5kHz PWM frequency for flicker-free operation
  - Individual on/off state per output

- **Web-Based Interface**:
  - Responsive design (desktop, tablet, mobile)
  - Dark theme with Volvo-inspired aesthetics
  - Multi-language support (6 languages):
    - English, German, French, Italian, Chinese, Hindi
  - Tab-based navigation
  - Master brightness control
  - Individual output controls with sliders
  - Real-time status display (500ms polling)

- **Custom Output Names**:
  - Editable names for each output
  - Click-to-edit inline functionality
  - Persistent storage (NVRAM/EEPROM)
  - Up to 20 characters per name

- **mDNS Hostname Support**:
  - Access via `railhub32.local` or `railhub8266.local`
  - Automatic hostname registration
  - Cross-platform compatibility (macOS, Linux, Windows with Bonjour)

- **Persistent Storage**:
  - ESP32: Preferences library (NVRAM)
  - ESP8266: EEPROM with structured data
  - Saves output states, brightness, and names
  - Automatic restoration on boot
  - Checksum validation (ESP8266)

- **RESTful API**:
  - `GET /` - Web interface
  - `GET /api/status` - System and output status (JSON)
  - `POST /api/control` - Control output state and brightness
  - `POST /api/name` - Update output name
  - `POST /api/reset` - Clear all saved settings

#### ESP32 Specific
- 16 GPIO outputs: 2, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 12, 13, 14
- Dual-core processor support
- Larger RAM (327 KB) and Flash (1.3 MB)
- ESPAsyncWebServer for high performance

#### ESP8266 Specific
- 8 GPIO outputs: 4, 5, 12, 13, 14, 16, 0, 2
- Optimized for limited RAM (80 KB)
- Standard ESP8266WebServer
- EEPROM structure with checksum

#### Testing & Quality
- **Comprehensive Unit Tests**: 33 automated tests
  - test_config (11 tests): Configuration validation
  - test_gpio (5 tests): GPIO and PWM control
  - test_json (8 tests): JSON API parsing
  - test_utils (9 tests): Helper functions
- **Test Environments**:
  - ESP32 hardware testing
  - Native x86 testing (no hardware required)
- **100% Test Pass Rate**

#### Documentation
- **arc42 Architecture Documentation**: 12 comprehensive documents
  - 50+ Mermaid diagrams
  - Complete system architecture
  - Deployment and runtime views
  - Quality requirements
  - Architecture decisions

- **Project Documentation**:
  - Comprehensive README with feature list
  - Hardware requirements and pin configuration
  - API reference with examples
  - Troubleshooting guide
  - Development guide

- **Development Effort Analysis**:
  - Detailed estimation document
  - 16-22 person-days total effort
  - Monetary value assessment
  - ROI analysis

#### Desktop Application
- **ESP32Flasher** (Windows):
  - One-click firmware flashing
  - Automatic ESP32 detection
  - Volvo-inspired UI
  - Real-time progress tracking
  - .NET 8.0 Windows Forms

### Technical Specifications (v1.0)
- **Platform**: Espressif ESP32 / ESP8266
- **Framework**: Arduino
- **Build System**: PlatformIO
- **Memory Usage**:
  - ESP32: 48 KB RAM (14.7%), 905 KB Flash (69.1%)
  - ESP8266: 77 KB RAM (95%), 408 KB Flash (39%)
- **Dependencies**:
  - ArduinoJson 7.4.2
  - ESPAsyncWebServer 3.6.0 (ESP32)
  - WiFiManager 2.0.17 (ESP8266)
  - ESPmDNS / ESP8266mDNS

---

## Version Comparison

| Feature | v1.0 | v2.0 |
|---------|------|------|
| **PWM Outputs** | 16 (ESP32) / 8 (ESP8266) | ✓ Same |
| **Web Interface** | HTTP polling (500ms) | ✓ + WebSocket real-time |
| **Blink Intervals** | ❌ | ✓ Per-output (0-65535ms) |
| **Chasing Groups** | ❌ | ✓ ESP8266 only (up to 4) |
| **WebSocket** | ❌ | ✓ Port 81, 500ms broadcasts |
| **Multi-language** | ✓ 6 languages | ✓ Same |
| **Custom Names** | ✓ Persistent | ✓ Same |
| **mDNS Support** | ✓ .local domains | ✓ Same |
| **Unit Tests** | ✓ 33 tests | ✓ Same |
| **Lines of Code** | ESP32: 1,868 / ESP8266: 1,833 | ESP32: 2,301 / ESP8266: 1,769 |

---

## Upgrade Guide

### From v1.0 to v2.0

#### For End Users:
1. **Backup**: Note down current output names and settings
2. **Flash**: Upload v2.0 firmware via ESP32Flasher or PlatformIO
3. **Reconfigure**: Re-enter WiFi credentials if needed
4. **Restore**: Re-apply output names and brightness settings
5. **Enjoy**: Benefit from real-time WebSocket updates and new features

#### For Developers:
1. **Update Dependencies**: Add `WebSockets @ 2.4.1` to `platformio.ini`
2. **Code Changes**:
   - WebSocket server initialization in `setup()`
   - `ws->loop()` in main `loop()`
   - `broadcastStatus()` function for real-time updates
   - Blink interval arrays and timing logic
   - Chasing group structures (ESP8266)
3. **Web Interface**: Update JavaScript to use WebSocket instead of polling
4. **Testing**: Verify WebSocket connectivity and real-time updates

#### API Compatibility:
- ✅ All v1.0 endpoints remain functional
- ✅ New endpoints are additive (no breaking changes)
- ✅ Status JSON structure expanded (backward compatible)
- ✅ Existing integrations continue to work

---

## Roadmap

### Planned for v3.0
- [ ] OTA (Over-The-Air) firmware updates
- [ ] MQTT support for home automation
- [ ] Advanced scheduling with cron-like syntax
- [ ] Scene management (save/restore presets)
- [ ] Mobile native apps (iOS/Android)
- [ ] WebSocket authentication
- [ ] SSL/TLS support (wss://)

### Under Consideration
- [ ] DCC (Digital Command Control) integration
- [ ] Voice control (Alexa, Google Home)
- [ ] Bluetooth LE support (ESP32)
- [ ] Cloud backup (optional, privacy-focused)
- [ ] Advanced animation effects
- [ ] Multi-device synchronization

---

## Contributors

- **Mark Ortner** - [Mark-Ortner-NRW](https://github.com/Mark-Ortner-NRW)
  - Lead Developer
  - Architecture & Design
  - v1.0 and v2.0 development

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

**Repository**: [Mark-Ortner-NRW/RailHub32-ESP32-Firmware](https://github.com/Mark-Ortner-NRW/RailHub32-ESP32-Firmware)  
**Last Updated**: November 14, 2025  
**Current Version**: 2.0.0
