# RailHub8266 ESP8266 WiFi-Controlled Railway Controller

ESP8266-based WiFi-controlled PWM output controller for model railways and lighting control. This is a port of the RailHub32 ESP32 controller, adapted for the ESP8266 platform with 8 outputs instead of 16.

## Features

- **8 Independent PWM Outputs**: Control 8 devices with individual on/off states and brightness (0-100%)
- **WiFi Connectivity**: Connect via existing WiFi or create access point for standalone operation
- **Web Interface**: Responsive multilingual web control panel (6 languages)
- **Persistent Storage**: All settings saved to EEPROM and restored on boot
- **WiFiManager Integration**: Easy WiFi configuration via captive portal
- **mDNS Support**: Access controller via `railhub8266.local`
- **Custom Output Names**: Assign meaningful names to each output
- **MQTT Ready**: RESTful API for integration with home automation systems

## Hardware Requirements

- ESP8266 board (ESP-12E, NodeMCU, Wemos D1 Mini, etc.)
- 8 PWM-capable output devices (LEDs, motors, etc.)
- Optional: Push button on GPIO 0 for config portal trigger
- Built-in LED on GPIO 2 for status indication

## Pin Configuration

The controller uses the following GPIO pins for PWM outputs:

| Output | GPIO | NodeMCU Label | Notes |
|--------|------|---------------|-------|
| 1      | 4    | D2            | Safe for PWM |
| 2      | 5    | D1            | Safe for PWM |
| 3      | 12   | D6            | Safe for PWM |
| 4      | 13   | D7            | Safe for PWM |
| 5      | 14   | D5            | Safe for PWM |
| 6      | 16   | D0            | No internal pull-up |
| 7      | 0    | D3            | Boot mode pin |
| 8      | 2    | D4            | Boot mode pin, built-in LED |

**Status LED**: GPIO 2 (D4) - Active LOW (LED on when pin LOW)

**Config Portal Trigger**: GPIO 0 (D3) - Hold LOW for 3 seconds

## Platform Differences from ESP32 Version

### ESP8266-Specific Adaptations

1. **PWM Outputs**: 8 outputs vs 16 on ESP32
2. **PWM API**: Uses `analogWrite()` with `analogWriteRange(255)` instead of `ledcWrite()`
3. **Storage**: EEPROM instead of Preferences library
4. **WiFi Library**: `ESP8266WiFi.h` instead of `WiFi.h`
5. **mDNS Library**: `ESP8266mDNS.h` instead of `ESPmDNS.h`
6. **Status LED**: Active LOW on GPIO 2 vs active HIGH on ESP32
7. **Memory**: Limited RAM (80KB) requires careful memory management

### EEPROM Structure

```cpp
struct EEPROMData {
    char deviceName[40];           // Custom device name
    bool outputStates[8];          // On/off states for 8 outputs
    uint8_t outputBrightness[8];   // Brightness values (0-255)
    char outputNames[8][21];       // Custom names (20 chars + null)
    uint8_t checksum;              // Data integrity check
};
```

Total EEPROM usage: ~265 bytes (512 bytes allocated)

## Building and Flashing

### PlatformIO (Recommended)

```bash
# Build for ESP8266 (ESP-12E)
pio run -e esp12e

# Upload to device
pio run -e esp12e --target upload

# Monitor serial output
pio device monitor
```

### Environment Options

The `platformio.ini` defines three environments:

- **esp12e**: Hardware build for ESP-12E module (production)
- **native**: Native x86 build for testing (limited functionality)
- **esp12e_test**: Hardware build with Unity test framework

## Initial Setup

1. **Flash Firmware**: Upload firmware to ESP8266
2. **Connect to WiFi**:
   - On first boot, device creates AP: `RailHub8266-Setup` (password: `12345678`)
   - Connect to AP and configure your WiFi credentials
   - Device reboots and connects to configured network
3. **Access Web Interface**:
   - Via mDNS: `http://railhub8266.local`
   - Via IP: Check serial monitor for assigned IP address
4. **Configure Outputs**: Set custom names and test each output

## Web Interface

The web interface provides:

- **Status Tab**: Real-time system information (WiFi signal, IP, uptime, memory)
- **Control Tab**: Individual output control with brightness sliders
- **Settings Tab**: Device configuration and output naming
- **Language Selector**: English, German, French, Italian, Spanish, Portuguese

### API Endpoints

```
GET  /              - Main web interface
GET  /api/status    - JSON status of all outputs and system info
POST /api/control   - Control output (output, state, brightness)
POST /api/name      - Set custom output name (output, name)
POST /api/reset     - Clear all saved settings (EEPROM wipe)
```

### Example API Usage

```bash
# Turn on Output 1 at 75% brightness
curl -X POST http://railhub8266.local/api/control \
  -d "output=0&state=1&brightness=191"

# Set custom name for Output 1
curl -X POST http://railhub8266.local/api/name \
  -d "output=0&name=Platform%20Lights"

# Get status
curl http://railhub8266.local/api/status
```

## Configuration Portal

To re-enter WiFi configuration:

1. Press and hold the button on GPIO 0 for 3 seconds
2. Device creates `RailHub8266-Setup` access point
3. Connect and configure new WiFi credentials
4. Device reboots with new settings

Or trigger via web interface Settings tab.

## Memory Usage

- **RAM**: ~95% utilized (77KB/80KB)
- **Flash**: ~39% utilized (408KB/1MB)

The ESP8266's limited RAM requires careful string handling and minimal dynamic allocations.

## Technical Specifications

- **Platform**: Espressif ESP8266
- **Framework**: Arduino
- **Compiler**: GCC 10.3.0 (Xtensa)
- **Flash**: 4MB (1MB program space)
- **RAM**: 80KB
- **PWM Frequency**: 1kHz (default)
- **PWM Resolution**: 8-bit (0-255)

## Dependencies

All dependencies are managed via PlatformIO:

- **ArduinoJson** 7.4.2 - JSON parsing and serialization
- **ESPAsyncWebServer** - Asynchronous web server
- **ESPAsyncTCP** - Async TCP library for ESP8266
- **ESPAsyncWiFiManager** - WiFi configuration portal
- **ESP8266WiFi** - WiFi connectivity (built-in)
- **ESP8266mDNS** - Multicast DNS (built-in)
- **EEPROM** - Non-volatile storage (built-in)

## File Structure

```
esp8266-controller/
├── platformio.ini          # Build configuration
├── include/
│   ├── config.h           # Hardware configuration
│   └── certificates.h     # SSL certificates (optional)
├── src/
│   └── main.cpp           # Main application code
├── test/                  # Unit tests (optional)
└── README.md             # This file
```

## Troubleshooting

### Device won't connect to WiFi
- Hold GPIO 0 button for 3 seconds to reset WiFi credentials
- Check SSID/password in configuration portal
- Verify 2.4GHz WiFi network (ESP8266 doesn't support 5GHz)

### Web interface not accessible
- Check serial monitor for IP address
- Try `http://railhub8266.local` (mDNS)
- Ensure device and computer are on same network

### Outputs not responding
- Verify correct GPIO pin connections
- Check power supply (adequate current for loads)
- Monitor serial output for error messages

### High RAM usage warnings
- ESP8266 has only 80KB RAM
- Limit concurrent connections to 2-3 clients
- Avoid large POST requests

## Safety Notes

- **Boot Pins**: GPIO 0, 2, 15 have boot mode constraints. Avoid external pull-ups/pull-downs.
- **Current Limits**: Each GPIO can source/sink max 12mA. Use transistors/MOSFETs for high-current loads.
- **Voltage**: ESP8266 is 3.3V device. Do NOT apply 5V to GPIO pins.
- **Power Supply**: Ensure stable 3.3V supply with adequate current (>250mA).

## License

[Specify your license here]

## Version History

- **1.0.0** - Initial ESP8266 port from RailHub32 ESP32 controller
  - 8 PWM outputs adapted for ESP8266 GPIO constraints
  - EEPROM storage replacing ESP32 Preferences
  - Optimized for ESP8266 memory limitations
  - Full web interface and WiFiManager integration

## Credits

Ported from **RailHub32** ESP32 controller to ESP8266 platform.

## Support

For issues, questions, or contributions, please contact [your contact info].
