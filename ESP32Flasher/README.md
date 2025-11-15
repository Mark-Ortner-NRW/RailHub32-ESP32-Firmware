<div align="center">

# ⚡ RailHub32 Flasher

### Windows Firmware Flashing Tool for ESP32

[![Version](https://img.shields.io/badge/version-2.0.0-brightgreen.svg)](https://github.com/Mark-Ortner-NRW/RailHub32-ESP32-Firmware/releases)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](https://dotnet.microsoft.com/)
[![.NET](https://img.shields.io/badge/.NET-8.0-purple.svg)](https://dotnet.microsoft.com/download/dotnet/8.0)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](../LICENSE)

**One-click ESP32 firmware flashing with Volvo-inspired minimalist design**

[Features](#-features) •
[Quick Start](#-running) •
[Usage](#-usage) •
[Troubleshooting](#-troubleshooting)

![Flasher Banner](https://img.shields.io/badge/⚡-ESP32%20Flasher-gold?style=for-the-badge)

</div>

---

## 📋 Overview

Minimalistic **Windows Forms application** for flashing RailHub32 v2.0 firmware to ESP32 devices. Supports the latest firmware with WebSocket support, blink intervals, and real-time updates.

> **🎨 Volvo Design Language** - Clean, refined, and user-friendly interface

## ✨ Features

<div align="center">

| Feature | Description |
|---------|-------------|
| 🔍 **Auto-Detection** | Scans serial ports and detects ESP32 automatically |
| 👆 **One-Click Flash** | No manual configuration - just click "FLASH FIRMWARE" |
| 🆕 **v2.0 Support** | Compatible with WebSocket, blink intervals, real-time updates |
| 🎨 **Volvo Design** | Clean, minimalistic UI with refined aesthetics |
| 📊 **Real-time Progress** | Visual progress bar and status updates |
| ⚠️ **Error Handling** | Clear error messages and recovery instructions |

</div>

## 💻 Requirements

<table>
<tr>
<td>

### Software Dependencies

- ✅ **.NET 8.0** Runtime or SDK
- ✅ **Python 3.x** (for esptool.py)
- ✅ **PlatformIO** (for esptool.py)

</td>
<td>

### Hardware Requirements

- ✅ **Windows PC** (Windows 10+)
- ✅ **ESP32 Board** (any variant)
- ✅ **USB Cable** (data-capable)

</td>
</tr>
</table>

### Firmware Files

📁 Built firmware must exist in: `../esp32-controller/.pio/build/esp32dev/`

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

## 🚀 Usage

### Step-by-Step Guide

<table>
<tr>
<td align="center" width="20%">

**1️⃣**

🔌

**Connect ESP32**

Connect your ESP32 board via USB

</td>
<td align="center" width="20%">

**2️⃣**

🚀

**Launch App**

App auto-detects ESP32

</td>
<td align="center" width="20%">

**3️⃣**

⚡

**Flash Firmware**

Click "FLASH FIRMWARE" button

</td>
<td align="center" width="20%">

**4️⃣**

⏳

**Wait**

Real-time progress shown

</td>
<td align="center" width="20%">

**5️⃣**

✅

**Done**

ESP32 auto-reboots

</td>
</tr>
</table>

## 🎨 Design Philosophy

<div align="center">

**Following Volvo's Design Language**

[![Design](https://img.shields.io/badge/Design-Volvo%20Inspired-gold?style=for-the-badge)](https://www.volvocars.com/)

</div>

<table>
<tr>
<td align="center">

### ✨ Minimalism

Clean interface with only essential elements

</td>
<td align="center">

### 🌟 Refinement

Sophisticated color palette

</td>
<td align="center">

### 💡 Clarity

Clear typography and status messaging

</td>
<td align="center">

### 👍 Functionality

User-friendly with minimal interaction

</td>
</tr>
</table>

### 🎨 Color Palette

```css
/* Volvo-Inspired Colors */
--volvo-black:   #141414  /* Primary background */
--volvo-gold:    #CCA666  /* Accent color */
--light-gray:    #F0F0F0  /* Primary text */
--medium-gray:   #646464  /* Secondary text */
--dark-gray:     #2D2D2D  /* UI elements */
```

## 🔧 Troubleshooting

<details>
<summary><b>❌ No ESP32 detected</b></summary>
<br>

**Symptoms:** Application shows "No ESP32 detected"

**Solutions:**
- ✅ Ensure USB cable is connected properly
- ✅ Check that drivers are installed (CP210x or CH340)
- ✅ Try a different USB port
- ✅ Restart the application
- ✅ Check Device Manager for COM ports

</details>

<details>
<summary><b>⚠️ Flashing failed</b></summary>
<br>

**Symptoms:** Flash process fails or times out

**Solutions:**
- ✅ Ensure firmware is built: `pio run` in esp32-controller folder
- ✅ Check that no serial monitor is open (Arduino IDE, PlatformIO, etc.)
- ✅ Verify Python and PlatformIO are installed
- ✅ Try holding BOOT button on ESP32 during flash
- ✅ Check USB cable quality (use data-capable cable)

</details>

<details>
<summary><b>🐍 esptool.py not found</b></summary>
<br>

**Symptoms:** Error message about missing esptool.py

**Solutions:**
- ✅ Install PlatformIO: `pip install platformio`
- ✅ Or install esptool directly: `pip install esptool`
- ✅ Ensure Python is in system PATH
- ✅ Restart terminal/application after install

</details>

---

<div align="center">

## 🔗 Additional Resources

[📖 Main README](../README.md) •
[🐛 Report Issue](https://github.com/Mark-Ortner-NRW/RailHub32-ESP32-Firmware/issues) •
[📝 Changelog](../CHANGELOG.md)

---

**Version 2.0.0** • **Windows Platform** • **.NET 8.0**

**Last Updated:** November 15, 2025

Made with ❤️ for the RailHub32 project

</div>
