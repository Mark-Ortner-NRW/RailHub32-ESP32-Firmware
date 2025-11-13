# RailHub32 ESP32 Firmware

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)

Advanced firmware for ESP32-based model railway control system with Access Point mode, persistent storage, and multi-language web interface.

## 🚂 Features

### Core Functionality
- **16 PWM Output Channels** - Control lighting, signals, and other railway accessories
- **WiFi Configuration Portal** - Easy WiFi setup with captive portal interface
- **Station & Access Point Modes** - Connect to existing WiFi or create standalone network
- **Web-Based Interface** - Modern, responsive control panel accessible from any browser
- **Persistent Storage** - Output states, brightness levels, and custom names saved to NVRAM
- **Real-time Control** - Instant response to commands via web interface
- **Custom Output Names** - Editable, persistent names for each output

### Web Interface Features
- **Master Brightness Control** - Adjust all outputs simultaneously
- **Individual Output Control** - Fine-tune each output independently
- **Editable Output Names** - Click any output name to customize it (persists across reboots)
- **Multi-Language Support** - Available in English, German, French, Italian, Chinese, and Hindi
- **Persistent Preferences** - Language and tab selection saved in browser
- **Status Monitoring** - Real-time display of IP address, WiFi status, uptime, and memory usage
- **Dark Theme** - Professional, easy-on-the-eyes interface design
- **Auto-Redirect** - Automatic redirect to control panel after WiFi configuration
- **Train Emoji Favicon** - Easy identification in browser tabs (🚂)

### Technical Highlights
- **Asynchronous Web Server** - Non-blocking operation for smooth performance
- **WiFiManager Integration** - ESPAsyncWiFiManager for easy configuration
- **JSON API** - RESTful endpoints for programmatic control
- **PWM Control** - 8-bit brightness resolution (0-255) for smooth dimming
- **Low Memory Footprint** - Efficient resource usage (~14% RAM, ~67.6% Flash)
- **Optimized Performance** - Debug output disabled for faster web interface response
- **Configuration Portal** - Password-protected setup at 192.168.4.1

## 📋 Hardware Requirements

- **ESP32 Development Board** (ESP32-D0WD or compatible)
- **Power Supply** - 5V USB or appropriate voltage for your board
- **Output Devices** - LEDs, relays, or other accessories (up to 16)

### Recommended Boards
- ESP32 DevKit v1
- ESP32-WROOM-32
- ESP32-WROVER

## 🔧 Pin Configuration

Default GPIO pins for outputs (configurable in `include/config.h`):

### Pin Assignment Table

| Output # | GPIO Pin | PWM Channel | Default Function | Notes |
|----------|----------|-------------|------------------|-------|
| 1 | GPIO 2 | 0 | Status LED / Output 1 | Built-in LED on most boards |
| 2 | GPIO 4 | 1 | Output 2 | General purpose |
| 3 | GPIO 5 | 2 | Output 3 | General purpose |
| 4 | GPIO 18 | 3 | Output 4 | General purpose |
| 5 | GPIO 19 | 4 | Output 5 | General purpose |
| 6 | GPIO 21 | 5 | Output 6 | General purpose |
| 7 | GPIO 22 | 6 | Output 7 | General purpose |
| 8 | GPIO 23 | 7 | Output 8 | General purpose |
| 9 | GPIO 25 | 8 | Output 9 | DAC1 capable |
| 10 | GPIO 26 | 9 | Output 10 | DAC2 capable |
| 11 | GPIO 27 | 10 | Output 11 | General purpose |
| 12 | GPIO 32 | 11 | Output 12 | ADC1_CH4 capable |
| 13 | GPIO 33 | 12 | Output 13 | ADC1_CH5 capable |
| 14 | GPIO 12 | 13 | Output 14 | ⚠️ Strapping pin |
| 15 | GPIO 13 | 14 | Output 15 | General purpose |
| 16 | GPIO 14 | 15 | Output 16 | General purpose |

### Pin Characteristics

#### ⚡ PWM Capable Pins
All configured pins support 8-bit PWM (0-255) at 5kHz frequency for smooth brightness control.

#### 🔌 Safe Pins for General Use
- **Best choice**: GPIO 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
- These pins are safe for general I/O without boot issues

#### ⚠️ Pins with Special Considerations
- **GPIO 2**: Built-in LED, must be floating or LOW during boot
- **GPIO 12**: Strapping pin - controls flash voltage. Keep LOW during boot
- **GPIO 15**: Strapping pin - affects boot message verbosity

#### 🚫 Pins to Avoid (Not Used in Default Config)
- **GPIO 0**: Boot mode selection (strapping pin)
- **GPIO 1 & 3**: TX/RX - Serial communication
- **GPIO 6-11**: Connected to internal flash (DO NOT USE)
- **GPIO 34-39**: Input only, no PWM support

### Connection Diagram

```
ESP32 DevKit                     External Devices
┌─────────────┐
│             │
│   GPIO 2  ──┼───────────────► LED / Relay 1
│   GPIO 4  ──┼───────────────► LED / Relay 2
│   GPIO 5  ──┼───────────────► LED / Relay 3
│   GPIO 18 ──┼───────────────► LED / Relay 4
│   GPIO 19 ──┼───────────────► LED / Relay 5
│   GPIO 21 ──┼───────────────► LED / Relay 6
│   GPIO 22 ──┼───────────────► LED / Relay 7
│   GPIO 23 ──┼───────────────► LED / Relay 8
│   GPIO 25 ──┼───────────────► LED / Relay 9
│   GPIO 26 ──┼───────────────► LED / Relay 10
│   GPIO 27 ──┼───────────────► LED / Relay 11
│   GPIO 32 ──┼───────────────► LED / Relay 12
│   GPIO 33 ──┼───────────────► LED / Relay 13
│   GPIO 12 ──┼───────────────► LED / Relay 14
│   GPIO 13 ──┼───────────────► LED / Relay 15
│   GPIO 14 ──┼───────────────► LED / Relay 16
│             │
│    GND    ──┼───────────────► Common Ground
│    3V3    ──┼───────────────► Power (low current)
│    VIN    ──┼───────────────► Power Input (5V)
│             │
└─────────────┘
```

### Wiring Notes

**For LEDs:**
```
GPIO Pin → 220Ω Resistor → LED (+) → LED (-) → GND
```

**For Relays:**
```
GPIO Pin → Relay Module Input
GND → Relay Module GND
VIN (5V) → Relay Module VCC
```

**⚠️ Important:**
- Use appropriate current-limiting resistors for LEDs
- For high-current loads, use relay modules or MOSFETs
- ESP32 GPIO pins: 3.3V logic, max 40mA per pin
- Total current for all pins should not exceed 400mA

## 🚀 Quick Start

### 1. Installation

#### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- USB cable for ESP32
- Git (optional)

#### Clone or Download
```bash
git clone https://github.com/Mark-Ortner-NRW/RailWays.git
cd RailWays/firmware/esp32-controller
```

### 2. Configuration

Edit `include/config.h` to customize:

```cpp
// WiFi Configuration Portal
#define WIFIMANAGER_AP_SSID "RailHub32-Setup"     // Configuration portal SSID
#define WIFIMANAGER_AP_PASSWORD "12345678"        // Portal password (min 8 chars)
#define PORTAL_TRIGGER_PIN 0                      // Button to trigger config portal

// Device Configuration
#define DEVICE_NAME "ESP32-Controller-01"
#define MAX_OUTPUTS 16
#define STATUS_LED_PIN 2

// Pin Definitions
#define LED_PINS {2, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 12, 13, 14}
```

### 3. Build and Upload

Using PlatformIO CLI:
```bash
platformio run --target upload
```

Using PlatformIO IDE:
- Open project in VS Code
- Click "Upload" button in PlatformIO toolbar

### 4. First-Time WiFi Setup

On first boot, the ESP32 automatically enters configuration mode:

1. **Find the Network**: Look for WiFi network `RailHub32-Setup`
2. **Connect**: Use password `12345678`
3. **Configure WiFi**: 
   - Browser should automatically open to http://192.168.4.1
   - If not, manually navigate to http://192.168.4.1
   - Select your WiFi network from the list
   - Enter your WiFi password
   - Click Save
4. **Auto-Redirect**: After successful connection, you'll be automatically redirected to the control panel

### 5. Using the Control Panel

Once connected to your WiFi network:
- The ESP32 will display its IP address on serial monitor
- Navigate to the displayed IP address in your browser
- Start controlling your railway outputs!

**Reconfiguration**: Hold the configuration button (GPIO 0) for 3 seconds to re-enter setup mode

**Note**: The ESP32 runs an HTTP server on port 80 for maximum compatibility with all devices and browsers.

## 📱 Web Interface

### Screenshots

![Status Tab](images/sc1.png)

*Status tab showing system information and connected clients*

![Outputs Control](images/sc2.png)

*Outputs tab with master brightness control and individual output controls*

### Status Tab
- AP IP Address
- Connected Clients Count
- System Uptime
- Free Memory
- MAC Address
- AP SSID

### Outputs Tab
- **Master Brightness Control**: Adjust all active outputs at once
- **Individual Controls**: Toggle and adjust each output separately
- **Editable Names**: Click any output name to customize it
  - Custom names persist across reboots
  - Shows default name in selected language if not customized
  - Max 20 characters per name
- **Real-time Updates**: Automatic status refresh every 5 seconds
- **Bulk Controls**: Turn all outputs ON or OFF at once

### Language Support
Select your preferred language from the header:
- 🇬🇧 English (EN) - with "Edit Name", "Save", "Cancel" buttons
- 🇩🇪 German (DE) - mit "Name bearbeiten", "Speichern", "Abbrechen"
- 🇫🇷 French (FR) - avec "Modifier le nom", "Enregistrer", "Annuler"
- 🇮🇹 Italian (IT) - con "Modifica nome", "Salva", "Annulla"
- 🇨🇳 Chinese (中文) - 带有 "编辑名称"、"保存"、"取消"
- 🇮🇳 Hindi (हिं) - "नाम संपादित करें", "सहेजें", "रद्द करें" के साथ

## 🔌 API Reference

### REST Endpoints

#### Get Status
```http
GET /api/status
```

**Response:**
```json
{
  "macAddress": "9C:9C:1F:18:9E:BC",
  "name": "ESP32-Controller-01",
  "wifiMode": "STA",
  "ip": "192.168.1.100",
  "ssid": "YourWiFiNetwork",
  "apClients": 0,
  "freeHeap": 248576,
  "uptime": 123456,
  "outputs": [
    {
      "pin": 2,
      "active": true,
      "brightness": 75,
      "name": "Station Light"
    },
    {
      "pin": 4,
      "active": false,
      "brightness": 0,
      "name": ""
    }
  ]
}
```

**Note**: The `name` field in outputs shows the custom name if set, or empty string for default names.

#### Control Output
```http
POST /api/control
Content-Type: application/json

{
  "pin": 2,
  "active": true,
  "brightness": 100
}
```

**Response:**
```json
{
  "status": "success",
  "pin": 2,
  "active": true,
  "brightness": 100
}
```

#### Update Output Name
```http
POST /api/name
Content-Type: application/json

{
  "pin": 2,
  "name": "Station Light"
}
```

**Response:**
```json
{
  "success": true
}
```

Updates the custom name for the specified output. Name is stored in NVRAM and persists across reboots.

#### Reset Saved States
```http
POST /api/reset
```

**Response:**
```json
{
  "status": "reset_complete"
}
```

Clears all saved output states from persistent storage (NVRAM).

### Configuration Portal

When in configuration mode, the ESP32 hosts a captive portal:

**Access Point Details:**
- **SSID**: RailHub32-Setup
- **Password**: 12345678
- **IP Address**: 192.168.4.1
- **Features**: 
  - WiFi network scanning
  - Password-protected setup
  - Custom device name configuration
  - Automatic redirect after successful connection
  - Train emoji favicon for easy identification

## 🛠️ Development

### Project Structure
```
firmware/esp32-controller/
├── include/
│   └── config.h           # Configuration settings
├── src/
│   └── main.cpp           # Main application code
├── platformio.ini         # PlatformIO configuration
└── README.md              # This file
```

### Dependencies
- `ArduinoJson` @ 7.4.2 - JSON parsing and serialization
- `ESPAsyncWebServer` @ 3.6.0 - Asynchronous web server
- `AsyncTCP` @ 3.3.2 - Asynchronous TCP library
- `ESPAsyncWiFiManager` @ 0.31.0 - WiFi configuration manager
- `Preferences` @ 2.0.0 - NVRAM persistent storage
- `WiFi` @ 2.0.0 - WiFi management

### Building
```bash
# Clean build
platformio run --target clean

# Build only
platformio run

# Upload
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### Memory Usage
- **RAM**: ~46 KB (14.0% of 320 KB)
- **Flash**: ~887 KB (67.6% of 1310 KB)

**Storage Breakdown:**
- Application code and web interface
- ESPAsyncWiFiManager with captive portal
- Multi-language translations (6 languages)
- Custom output names storage
- Output states and brightness persistence

## 🐛 Troubleshooting

### Cannot Connect to Configuration Portal
- Ensure ESP32 is powered on (status LED should be lit)
- Look for WiFi network named `RailHub32-Setup`
- Password is `12345678` (minimum 8 characters required)
- If portal doesn't auto-open, manually navigate to http://192.168.4.1
- Try forgetting the network and reconnecting

### Configuration Portal Not Appearing
- Hold the configuration button (GPIO 0) for 3 seconds to trigger portal
- Serial monitor will show: "=== Entered Config Mode ==="
- Check that no other device is connected to the configuration AP
- Power cycle the ESP32 if it's stuck

### Web Interface Not Loading
- Check serial monitor for the assigned IP address
- Ensure you're connected to the same WiFi network as the ESP32
- If in config mode, use http://192.168.4.1
- Clear browser cache
- Try a different browser
- Verify WiFi connection was successful (check serial output)

### Outputs Not Working
- Verify correct GPIO pin connections
- Check power supply to outputs
- Review serial monitor for error messages
- Ensure output states are properly configured
- Check if custom names are saving (indicates NVRAM is working)

### Custom Names Not Saving
- Check serial monitor for "Saved name for output X: [name]" messages
- NVRAM errors on first boot are normal (keys don't exist yet)
- If names don't persist, NVRAM may be full (rare)
- Try clearing all saved states via `/api/reset`

### Page Performance Issues
- Debug output is disabled by default for optimal performance
- WiFi scanning is optimized (removed duplicate APs)
- If slow, check serial monitor for unusual activity
- Clear browser cache and refresh

### Upload Fails
- Check USB cable connection
- Verify correct COM port in `platformio.ini`
- Press BOOT button on ESP32 during upload
- Stop serial monitor before uploading
- Try reducing upload speed
- Use `Stop-Process -Name "pio"` on Windows if port is locked

## 📊 Performance

- **Web Response Time**: < 50ms (optimized with debug output disabled)
- **Command Latency**: < 10ms
- **PWM Frequency**: 5 kHz
- **PWM Resolution**: 8-bit (0-255)
- **UI Refresh Rate**: 5 seconds (auto-refresh)
- **WiFi Scan**: Optimized (duplicates removed)
- **Configuration Portal**: Password-protected, responsive interface

## 🔐 Security

- **Default Credentials**: Change `WIFIMANAGER_AP_PASSWORD` in `config.h` before deployment (minimum 8 characters)
- **Configuration Portal**: Password-protected setup mode (default: "12345678")
- **WiFi Station Mode**: Connects to your existing WiFi network with your credentials
- **HTTP Protocol**: Uses standard HTTP on port 80 for maximum device compatibility
- **No Internet Required**: Fully functional offline system
- **Local Storage**: All data (states, brightness, names) stored securely in device NVRAM
- **No Cloud Dependencies**: Complete privacy - no data sent to external servers

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- **Mark Ortner** - *Initial work* - [Mark-Ortner-NRW](https://github.com/Mark-Ortner-NRW)

## 🙏 Acknowledgments

- ESP32 Arduino Framework
- PlatformIO Team
- AsyncWebServer Library
- Model Railway Community

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Mark-Ortner-NRW/RailWays/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Mark-Ortner-NRW/RailWays/discussions)

## 🗺️ Roadmap

- [x] WiFi Configuration Portal with captive portal
- [x] Custom editable output names (persistent)
- [x] Multi-language support (6 languages)
- [x] Performance optimization (debug output disabled)
- [x] Auto-redirect after WiFi configuration
- [x] Train emoji favicon
- [ ] OTA (Over-The-Air) Updates
- [ ] Scenario/Scene Management  
- [ ] Mobile App Integration
- [ ] Advanced Scheduling
- [ ] Multi-Device Synchronization
- [ ] MQTT Integration (removed in current version)
- [ ] Integration with DCC Systems
- [ ] Backup/Restore Configuration
- [ ] Output Groups/Zones

---

**Made with ❤️ for model railway enthusiasts**
