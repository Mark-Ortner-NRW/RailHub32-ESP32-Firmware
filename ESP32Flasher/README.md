# RailHub32 Flasher v2.0

Minimalistic Windows Forms application for flashing RailHub32 v2.0 firmware to ESP32 devices. Supports the latest firmware with WebSocket support, blink intervals, and real-time updates.

## Features

- **Automatic ESP32 Detection**: Scans serial ports and automatically detects connected ESP32 devices
- **One-Click Flashing**: No manual configuration required - just click "FLASH FIRMWARE"
- **v2.0 Firmware Support**: Compatible with RailHub32 v2.0 features (WebSocket, blink intervals)
- **Volvo-Inspired Design**: Clean, minimalistic UI with refined aesthetics
- **Real-time Progress**: Visual progress bar and status updates during flashing
- **Error Handling**: Clear error messages and recovery instructions

## Requirements

- .NET 8.0 Runtime or SDK
- Python 3.x (for esptool.py)
- PlatformIO (must be installed for esptool.py)
- Built firmware files in `../esp32-controller/.pio/build/esp32dev/`

## Building

```bash
cd ESP32Flasher
dotnet build
```

## Running

```bash
dotnet run
```

Or build and run the executable:

```bash
dotnet publish -c Release -r win-x64 --self-contained
```

The executable will be in `bin/Release/net8.0-windows/win-x64/publish/`

## Usage

1. **Connect ESP32**: Connect your ESP32 board via USB
2. **Launch Application**: The app will automatically detect the ESP32
3. **Flash Firmware**: Click "FLASH FIRMWARE" button
4. **Wait**: Progress will be shown in real-time
5. **Done**: ESP32 will automatically reboot with new firmware

## Design Philosophy

The application follows Volvo's design language:

- **Minimalism**: Clean interface with only essential elements
- **Refinement**: Sophisticated color palette (blacks, grays, gold accents)
- **Clarity**: Clear typography and status messaging
- **Functionality**: User-friendly with minimal interaction required

## Color Palette

- **Volvo Black**: `#141414` - Primary background
- **Volvo Gold**: `#CCA666` - Accent color for status and progress
- **Light Gray**: `#F0F0F0` - Primary text
- **Medium Gray**: `#646464` - Secondary text
- **Dark Gray**: `#2D2D2D` - UI elements

## Troubleshooting

**No ESP32 detected:**
- Ensure USB cable is connected
- Check that drivers are installed
- Try a different USB port
- Restart the application

**Flashing failed:**
- Ensure firmware is built (`pio run` in esp32-controller folder)
- Check that no serial monitor is open
- Verify Python and PlatformIO are installed

**esptool.py not found:**
- Install PlatformIO: `pip install platformio`
- Or specify custom esptool path in code

## License

Part of the RailHub32 v2.0 project.

---

**Version**: 2.0 (compatible with RailHub32 v2.0 firmware)  
**Platform**: Windows (.NET 8.0)  
**Last Updated**: November 14, 2025
