# RailHub32 ESP32 Firmware

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)

Advanced firmware for ESP32-based model railway control system with Access Point mode, persistent storage, and multi-language web interface.

## 🚂 Features

### Core Functionality
- **16 PWM Output Channels** - Control lighting, signals, and other railway accessories
- **Access Point Mode** - ESP32 creates its own WiFi network for direct device connection
- **Web-Based Interface** - Modern, responsive control panel accessible from any browser
- **MQTT Support** - Integration with home automation systems and external controllers
- **Persistent Storage** - Output states and brightness levels saved to NVRAM
- **Real-time Control** - Instant response to commands via web interface or MQTT

### Web Interface Features
- **Master Brightness Control** - Adjust all outputs simultaneously
- **Individual Output Control** - Fine-tune each output independently
- **Multi-Language Support** - Available in English, German, French, Italian, Chinese, and Hindi
- **Persistent Preferences** - Language and tab selection saved in browser
- **Status Monitoring** - Real-time display of IP address, connected clients, uptime, and memory usage
- **Dark Theme** - Professional, easy-on-the-eyes interface design

### Technical Highlights
- **Asynchronous Web Server** - Non-blocking operation for smooth performance
- **JSON API** - RESTful endpoints for programmatic control
- **PWM Control** - 8-bit brightness resolution (0-255) for smooth dimming
- **Low Memory Footprint** - Efficient resource usage (~14% RAM, ~66% Flash)
- **OTA Ready** - Architecture supports over-the-air updates

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
// WiFi Access Point Configuration
#define AP_SSID "RailHub32-AP"           // Your network name
#define AP_PASSWORD "RailHub32Pass"      // Your password (min 8 chars)
#define AP_LOCAL_IP "192.168.4.1"        // ESP32 IP address

// MQTT Configuration (optional)
#define MQTT_BROKER "192.168.4.100"      // MQTT broker IP
#define MQTT_PORT 1883

// Device Configuration
#define DEVICE_NAME "ESP32-Controller-01"
#define MAX_OUTPUTS 16

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

### 4. Connect

1. **Find the Network**: Look for WiFi network `RailHub32-AP`
2. **Connect**: Use password `RailHub32Pass` (minimum 8 characters)
3. **Open Browser**: Navigate to `http://192.168.4.1`
4. **Control**: Start controlling your railway!

**Note**: The ESP32 runs an HTTP server on port 80 for maximum compatibility with all devices and browsers.

## 📱 Web Interface

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
- **Real-time Updates**: Automatic status refresh every 5 seconds

### Language Support
Select your preferred language from the header:
- 🇬🇧 English (EN)
- 🇩🇪 German (DE)
- 🇫🇷 French (FR)
- 🇮🇹 Italian (IT)
- 🇨🇳 Chinese (中文)
- 🇮🇳 Hindi (हिं)

## 🔌 API Reference

### REST Endpoints

#### Get Status
```http
GET /api/status
```

**Response:**
```json
{
  "deviceId": "RailHub32-ESP32-9c9c1f189ebc",
  "macAddress": "9C:9C:1F:18:9E:BC",
  "name": "ESP32-Controller-01",
  "ip": "192.168.4.1",
  "apClients": 2,
  "freeHeap": 248576,
  "uptime": 123456,
  "outputs": [
    {
      "pin": 2,
      "active": true,
      "brightness": 75
    }
  ]
}
```

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

### MQTT Topics

#### Subscribe (Commands)
```
railhub32/device/[MAC_ADDRESS]/command
```

**Payload:**
```json
{
  "pin": 2,
  "active": true,
  "brightness": 80
}
```

#### Publish (Status)
```
railhub32/device/[MAC_ADDRESS]/status
railhub32/device/[MAC_ADDRESS]/discovery
```

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
- `PubSubClient` @ 2.8.0 - MQTT client
- `ArduinoJson` @ 7.4.2 - JSON parsing
- `ESPAsyncWebServer` @ 3.6.0 - Async web server
- `AsyncTCP` @ 3.3.2 - Async TCP library
- `Preferences` @ 2.0.0 - NVRAM storage
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
- **RAM**: ~45 KB (13.9% of 320 KB)
- **Flash**: ~865 KB (66% of 1310 KB)

## 🐛 Troubleshooting

### Cannot Connect to WiFi
- Ensure ESP32 is powered on (status LED should be lit)
- Check that WiFi is enabled on your device
- Verify you're using the correct password
- Try forgetting the network and reconnecting

### Web Interface Not Loading
- Verify IP address: `192.168.4.1`
- Check that you're connected to the ESP32's network
- Clear browser cache
- Try a different browser

### Outputs Not Working
- Verify correct GPIO pin connections
- Check power supply to outputs
- Review serial monitor for error messages
- Ensure output states are properly configured

### Upload Fails
- Check USB cable connection
- Verify correct COM port in `platformio.ini`
- Press BOOT button on ESP32 during upload
- Try reducing upload speed

## 📊 Performance

- **Web Response Time**: < 50ms
- **Command Latency**: < 10ms
- **PWM Frequency**: 5 kHz
- **Status Update Interval**: 30 seconds (MQTT)
- **UI Refresh Rate**: 5 seconds

## 🔐 Security

- **Default Credentials**: Change `AP_PASSWORD` in `config.h` before deployment (minimum 8 characters)
- **Network Isolation**: AP mode creates isolated network separate from your main WiFi
- **HTTP Protocol**: Uses standard HTTP on port 80 for maximum device compatibility
- **No Internet Required**: Fully functional offline system
- **Local Storage**: All data stored securely on device NVRAM
- **Password Protection**: WiFi access point is password-protected

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

- [ ] OTA (Over-The-Air) Updates
- [ ] Scenario/Scene Management
- [ ] Mobile App Integration
- [ ] Advanced Scheduling
- [ ] Multi-Device Synchronization
- [ ] Web-based Configuration Editor
- [ ] Integration with DCC Systems

---

**Made with ❤️ for model railway enthusiasts**
