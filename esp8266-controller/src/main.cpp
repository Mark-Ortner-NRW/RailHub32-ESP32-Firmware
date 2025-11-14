#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include "config.h"

// Forward declarations
void initializeOutputs();
void initializeWiFi();
void initializeWiFiManager();
void checkConfigPortalTrigger();
void initializeWebServer();
void executeOutputCommand(int pin, bool active, int brightnessPercent);
void saveOutputState(int index);
void loadOutputStates();
void saveAllOutputStates();
void saveCustomParameters();
void loadCustomParameters();

// Global variables
// Web Server
ESP8266WebServer* server = nullptr;
WiFiManager wifiManager;

// EEPROM structure for ESP8266
struct EEPROMData {
    char deviceName[40];
    bool outputStates[8];
    uint8_t outputBrightness[8];
    char outputNames[8][21]; // 20 chars + null terminator
    uint8_t checksum;
};
EEPROMData eepromData;

String macAddress;
char customDeviceName[40] = DEVICE_NAME; // Custom device name from WiFiManager
bool portalRunning = false;
unsigned long portalButtonPressTime = 0;
bool wifiConnected = false;

// Output pin configuration
int outputPins[MAX_OUTPUTS] = LED_PINS;
bool outputStates[MAX_OUTPUTS] = {false};
int outputBrightness[MAX_OUTPUTS] = {255}; // 0-255 for PWM
String outputNames[MAX_OUTPUTS]; // Custom names for outputs

// Timing variables

void setup() {
    Serial.begin(115200);
    delay(100);
    
    // Initialize EEPROM for ESP8266
    EEPROM.begin(EEPROM_SIZE);
    
    Serial.println("\n\n========================================");
    Serial.println("  RailHub8266 ESP8266 Controller v1.0");
    Serial.println("========================================");
    Serial.println("[BOOT] Chip ID: " + String(ESP.getChipId(), HEX));
    Serial.println("[BOOT] CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("[BOOT] Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB");
    Serial.println("[BOOT] Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    
    // Get MAC address for unique identification
    macAddress = WiFi.macAddress();
    Serial.println("[INIT] MAC Address: " + macAddress);
    
    // Initialize portal trigger pin
    Serial.println("[INIT] Configuring portal trigger pin (GPIO " + String(PORTAL_TRIGGER_PIN) + ")");
    pinMode(PORTAL_TRIGGER_PIN, INPUT_PULLUP);
    
    // Initialize output pins
    Serial.println("[INIT] Initializing " + String(MAX_OUTPUTS) + " output pins...");
    initializeOutputs();
    
    // Load custom parameters from preferences
    Serial.println("[INIT] Loading custom parameters from NVRAM...");
    loadCustomParameters();
    
    // Load saved output states from NVRAM
    Serial.println("[INIT] Loading saved output states...");
    loadOutputStates();
    
    // Initialize WiFi with WiFiManager
    Serial.println("[INIT] Initializing WiFi Manager...");
    initializeWiFiManager();
    
    // Initialize web server after WiFi is connected
    if (wifiConnected) {
        Serial.println("[INIT] Starting web server on port 80...");
        server = new ESP8266WebServer(80);
        initializeWebServer();
        Serial.println("[WEB] Web server initialized successfully");
    } else {
        Serial.println("[WARN] WiFi not connected - web server not started");
    }
    
    Serial.println("\n========================================");
    Serial.println("  Setup Complete!");
    Serial.println("========================================");
    Serial.println("[INFO] Device Name: " + String(customDeviceName));
    Serial.println("[INFO] Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("[INFO] System ready for operation\n");
}

void loop() {
    // Check for config portal trigger button
    checkConfigPortalTrigger();
    
    // Handle web server requests
    if (server) {
        server->handleClient();
    }
    
    // Update mDNS responder
    MDNS.update();
    
    // Handle any other tasks
    yield();
}

// Periodic status logging (called every 60 seconds via timer)
void logSystemStatus() {
    static unsigned long lastStatusLog = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastStatusLog >= 60000) {
        lastStatusLog = currentMillis;
        Serial.println("\n[STATUS] === System Status Report ===");
        Serial.println("[STATUS] Uptime: " + String(currentMillis / 1000) + " seconds");
        Serial.println("[STATUS] Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("[STATUS] WiFi Status: " + String(WiFi.isConnected() ? "Connected" : "Disconnected"));
        if (WiFi.isConnected()) {
            Serial.println("[STATUS] IP Address: " + WiFi.localIP().toString());
            Serial.println("[STATUS] RSSI: " + String(WiFi.RSSI()) + " dBm");
        }
        
        // Count active outputs
        int activeCount = 0;
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            if (outputStates[i]) activeCount++;
        }
        Serial.println("[STATUS] Active Outputs: " + String(activeCount) + "/" + String(MAX_OUTPUTS));
        Serial.println("[STATUS] ========================\n");
    }
}

void initializeOutputs() {
    Serial.println("[OUTPUT] Initializing outputs...");
    
    // Set PWM range for ESP8266 (0-1023 by default, we'll use 0-255 range)
    analogWriteRange(255);
    analogWriteFreq(1000); // 1kHz PWM frequency
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        Serial.print("[OUTPUT] Configuring Output " + String(i) + " on GPIO " + String(outputPins[i]));
        pinMode(outputPins[i], OUTPUT);
        analogWrite(outputPins[i], 0);
        Serial.println(" - OK (PWM 1kHz, 8-bit)");
    }
    
    // Status LED (active LOW on ESP8266)
    Serial.println("[OUTPUT] Initializing status LED on GPIO " + String(STATUS_LED_PIN));
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW); // Turn on status LED (active LOW)
    Serial.println("[OUTPUT] All outputs initialized successfully");
}

void initializeWiFi() {
    Serial.println("Configuring Access Point...");
    
    // Disconnect from any existing WiFi connection
    WiFi.disconnect();
    delay(100);
    
    // Configure Access Point IP address
    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    
    local_IP.fromString(AP_LOCAL_IP);
    gateway.fromString(AP_GATEWAY);
    subnet.fromString(AP_SUBNET);
    
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println("AP Config Failed!");
    }
    
    // Start Access Point
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTIONS);
    
    if (apStarted) {
        Serial.println();
        Serial.println("Access Point started successfully!");
        Serial.print("AP SSID: ");
        Serial.println(AP_SSID);
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("AP MAC address: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.print("Max connections: ");
        Serial.println(AP_MAX_CONNECTIONS);
        
        // Blink status LED to indicate AP started (active LOW)
        for (int i = 0; i < 5; i++) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(150);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(150);
        }
    } else {
        Serial.println();
        Serial.println("Access Point failed to start!");
    }
}

void initializeWiFiManager() {
    Serial.println("[WIFI] Initializing WiFiManager...");
    Serial.println("[WIFI] Configuration Portal SSID: " + String(WIFIMANAGER_AP_SSID));
    Serial.println("[WIFI] Portal Trigger Pin: GPIO " + String(PORTAL_TRIGGER_PIN));
    
    // Ensure WiFi is in correct mode
    WiFi.mode(WIFI_STA);
    delay(100);
    
    // WiFiManager already initialized globally
    
    // Set custom parameters
    WiFiManagerParameter custom_device_name("device_name", "Device Name", customDeviceName, 40);
    
    // Add parameters to WiFiManager
    wifiManager.addParameter(&custom_device_name);
    
    // Minimal configuration for ESP8266 to save RAM
    wifiManager.setMinimumSignalQuality(20);  // Higher = fewer networks shown = less RAM
    wifiManager.setRemoveDuplicateAPs(true);
    
    // Disable features to save memory
    wifiManager.setShowInfoUpdate(false);  // Don't show info in update mode
    wifiManager.setShowInfoErase(false);   // Don't show erase button
    
    // Set save config callback
    wifiManager.setSaveConfigCallback([]() {
        Serial.println("[WIFI] Configuration saved!");
        Serial.print("[WIFI] Device Name: ");
        Serial.println(customDeviceName);
        Serial.println("[WIFI] WiFi credentials will be used on next boot");
        Serial.println("[WIFI] Restarting ESP8266 to apply new configuration...");
        delay(2000);
        ESP.restart();
    });
    
    // Set short timeout to free memory after config
    wifiManager.setConfigPortalTimeout(300); // 5 minutes timeout
    
    // Disable debug output to save RAM
    wifiManager.setDebugOutput(false);
    

    
    // Set AP callback
    wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
        Serial.println("\n========================================");
        Serial.println("     CONFIGURATION MODE ACTIVE");
        Serial.println("========================================");
        Serial.println("[WIFI] AP Mode Started");
        Serial.println("[WIFI] AP SSID: " + String(WIFIMANAGER_AP_SSID));
        Serial.println("[WIFI] AP Password: " + String(WIFIMANAGER_AP_PASSWORD));
        Serial.println("[WIFI] AP IP Address: " + WiFi.softAPIP().toString());
        Serial.println("[WIFI] Configuration Portal: http://192.168.4.1");
        Serial.println("[INFO] Connect your device to the AP above");
        Serial.println("[INFO] Portal running on port 80");
        Serial.println("========================================\n");
        
        // Blink LED to indicate config mode (active LOW)
        for (int i = 0; i < 10; i++) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(100);
        }
        digitalWrite(STATUS_LED_PIN, LOW); // On (active LOW)
    });
    
    // Configure static IP for portal
    IPAddress portal_ip(192, 168, 4, 1);
    IPAddress portal_gateway(192, 168, 4, 1);
    IPAddress portal_subnet(255, 255, 255, 0);
    wifiManager.setAPStaticIPConfig(portal_ip, portal_gateway, portal_subnet);
    
    // Try to connect with saved credentials or start portal
    Serial.println("[WIFI] Attempting to connect to WiFi...");
    Serial.print("[WIFI] Config AP SSID: ");
    Serial.println(WIFIMANAGER_AP_SSID);
    
    // Use NULL for open AP if password is empty, otherwise use the password
    const char* apPassword = (strlen(WIFIMANAGER_AP_PASSWORD) == 0) ? NULL : WIFIMANAGER_AP_PASSWORD;
    
    unsigned long connectStart = millis();
    if (wifiManager.autoConnect(WIFIMANAGER_AP_SSID, apPassword)) {
        // Connected to WiFi successfully
        unsigned long connectDuration = millis() - connectStart;
        wifiConnected = true;
        
        Serial.println("\n========================================");
        Serial.println("     WIFI CONNECTION SUCCESSFUL");
        Serial.println("========================================");
        Serial.print("[WIFI] IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WIFI] SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("[WIFI] Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.print("[WIFI] MAC Address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("[WIFI] Connection Time: ");
        Serial.print(connectDuration);
        Serial.println("ms");
        Serial.println("========================================\n");
        
        // Get custom parameters
        strncpy(customDeviceName, custom_device_name.getValue(), 40);
        saveCustomParameters();
        
        // Start mDNS service
        String hostname = String(customDeviceName);
        hostname.toLowerCase();
        hostname.replace(" ", "-");
        if (MDNS.begin(hostname.c_str())) {
            Serial.print("[MDNS] mDNS responder started: ");
            Serial.print(hostname);
            Serial.println(".local");
            MDNS.addService("http", "tcp", 80);
            Serial.println("[MDNS] HTTP service added");
        } else {
            Serial.println("[ERROR] mDNS failed to start");
        }
        
        // Solid LED to indicate connected (active LOW)
        digitalWrite(STATUS_LED_PIN, LOW);
    } else {
        // Failed to connect - fallback to AP mode
        Serial.println("[ERROR] Failed to connect - starting fallback AP mode");
        wifiConnected = false;
        initializeWiFi();
    }
}

void checkConfigPortalTrigger() {
    static bool warningShown = false;
    
    // Check if portal trigger button is pressed
    if (digitalRead(PORTAL_TRIGGER_PIN) == LOW) {
        if (portalButtonPressTime == 0) {
            portalButtonPressTime = millis();
            warningShown = false;
            Serial.println("[PORTAL] Config button pressed (hold for 3s to trigger)");
        } else {
            unsigned long holdDuration = millis() - portalButtonPressTime;
            
            // Warning at 2.5 seconds - only show once
            if (holdDuration > 2500 && !warningShown && !portalRunning) {
                Serial.println("[PORTAL] Warning: Portal trigger in 0.5s...");
                warningShown = true;
            }
            
            if (holdDuration > PORTAL_TRIGGER_DURATION && !portalRunning) {
                Serial.println("[PORTAL] Portal trigger detected! Resetting WiFi and restarting...");
                Serial.print("[PORTAL] Free heap before reset: ");
                Serial.print(ESP.getFreeHeap());
                Serial.println(" bytes");
                portalRunning = true;
                
                // Blink LED rapidly (active LOW)
                Serial.println("[PORTAL] Blinking status LED (confirmation)");
                for (int i = 0; i < 20; i++) {
                    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
                    delay(50);
                }
                
                // Clear WiFi settings (ESP8266 stores WiFi creds in flash)
                Serial.println("[PORTAL] Disconnecting WiFi and clearing saved networks...");
                WiFi.disconnect(true); // true = also erase stored credentials
                delay(1000);
                
                // Restart to trigger portal
                Serial.println("[PORTAL] Restarting ESP8266 in 1s...");
                Serial.flush();
                delay(1000);
                ESP.restart();
            }
        }
    } else {
        if (portalButtonPressTime > 0) {
            unsigned long pressDuration = millis() - portalButtonPressTime;
            Serial.print("[PORTAL] Config button released after ");
            Serial.print(pressDuration);
            Serial.println("ms (trigger requires 3000ms)");
        }
        portalButtonPressTime = 0;
        portalRunning = false;
        warningShown = false;
    }
}

void saveCustomParameters() {
    Serial.println("[EEPROM] Saving custom parameters...");
    
    // Read current EEPROM data
    EEPROM.get(0, eepromData);
    
    // Update device name
    strncpy(eepromData.deviceName, customDeviceName, 39);
    eepromData.deviceName[39] = '\0';
    
    // Write back to EEPROM
    EEPROM.put(0, eepromData);
    EEPROM.commit();
    
    Serial.print("[EEPROM] Custom parameters saved: Device Name = '");
    Serial.print(customDeviceName);
    Serial.println("'");
}

void loadCustomParameters() {
    Serial.println("[EEPROM] Loading custom parameters...");
    
    // Read EEPROM data
    EEPROM.get(0, eepromData);
    
    // Check if data is valid (simple check - not empty)
    if (eepromData.deviceName[0] != '\0' && eepromData.deviceName[0] != 0xFF) {
        strncpy(customDeviceName, eepromData.deviceName, 39);
        customDeviceName[39] = '\0';
        Serial.print("[EEPROM] Loaded custom device name: '");
        Serial.print(customDeviceName);
        Serial.println("'");
    } else {
        strncpy(customDeviceName, DEVICE_NAME, 39);
        customDeviceName[39] = '\0';
        Serial.print("[EEPROM] No custom device name found, using default: '");
        Serial.print(customDeviceName);
        Serial.println("'");
    }
}

void executeOutputCommand(int pin, bool active, int brightnessPercent) {
    unsigned long startTime = millis();
    
    // Find the output index for the given pin
    int outputIndex = -1;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (outputPins[i] == pin) {
            outputIndex = i;
            break;
        }
    }
    
    if (outputIndex == -1) {
        Serial.println("[ERROR] Invalid GPIO pin: " + String(pin));
        return;
    }
    
    // Validate brightness range
    if (brightnessPercent < 0 || brightnessPercent > 100) {
        Serial.println("[ERROR] Invalid brightness: " + String(brightnessPercent) + "% (must be 0-100)");
        brightnessPercent = constrain(brightnessPercent, 0, 100);
    }
    
    // Update state
    outputStates[outputIndex] = active;
    outputBrightness[outputIndex] = map(brightnessPercent, 0, 100, 0, 255);
    
    // Apply the command
    if (active) {
        analogWrite(outputPins[outputIndex], outputBrightness[outputIndex]);
    } else {
        analogWrite(outputPins[outputIndex], 0);
    }
    
    // Save the state to persistent storage
    saveOutputState(outputIndex);
    
    unsigned long duration = millis() - startTime;
    String nameStr = outputNames[outputIndex].length() > 0 ? " [" + outputNames[outputIndex] + "]" : "";
    Serial.println("[CMD] Output " + String(outputIndex) + " (GPIO " + String(pin) + ")" + nameStr + ": " + 
                   (active ? "ON" : "OFF") + " @ " + String(brightnessPercent) + "% (" + String(duration) + "ms)");
}

void saveOutputState(int index) {
    if (index < 0 || index >= MAX_OUTPUTS) {
        Serial.print("[ERROR] Invalid output index for state save: ");
        Serial.println(index);
        return;
    }
    
    // Read current EEPROM data
    EEPROM.get(0, eepromData);
    
    // Update specific output
    eepromData.outputStates[index] = outputStates[index];
    eepromData.outputBrightness[index] = outputBrightness[index];
    
    // Write back to EEPROM
    EEPROM.put(0, eepromData);
    EEPROM.commit();
    
    Serial.print("[EEPROM] Saved state for Output ");
    Serial.print(index);
    Serial.print(" (GPIO ");
    Serial.print(outputPins[index]);
    Serial.print("): ");
    Serial.print(outputStates[index] ? "ON" : "OFF");
    Serial.print(" @ ");
    Serial.print(outputBrightness[index]);
    Serial.println(" PWM");
}

void saveOutputName(int index, String name) {
    if (index < 0 || index >= MAX_OUTPUTS) {
        Serial.println("[ERROR] Invalid output index for name save: " + String(index));
        return;
    }
    
    // Read current EEPROM data
    EEPROM.get(0, eepromData);
    
    // If name is empty or whitespace-only, clear the name
    name.trim(); // Trim modifies in place
    if (name.length() == 0) {
        eepromData.outputNames[index][0] = '\0';
        outputNames[index] = "";
        EEPROM.put(0, eepromData);
        EEPROM.commit();
        Serial.println("[EEPROM] Removed custom name for Output " + String(index) + " (GPIO " + String(outputPins[index]) + ") - using default");
        return;
    }
    
    // Copy name to EEPROM structure (max 20 chars + null)
    strncpy(eepromData.outputNames[index], name.c_str(), 20);
    eepromData.outputNames[index][20] = '\0';
    
    // Write back to EEPROM
    EEPROM.put(0, eepromData);
    EEPROM.commit();
    
    outputNames[index] = name;
    Serial.println("[EEPROM] Saved name for Output " + String(index) + " (GPIO " + String(outputPins[index]) + "): '" + name + "'");
}

void loadOutputStates() {
    Serial.println("[EEPROM] Loading saved output states...");
    
    // Read EEPROM data
    EEPROM.get(0, eepromData);
    
    // Check if data is valid (check for uninitialized EEPROM)
    bool validData = true;
    
    // Check device name for 0xFF pattern (uninitialized)
    if (eepromData.deviceName[0] == 0xFF) {
        validData = false;
    }
    
    // Check brightness values
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (eepromData.outputBrightness[i] > 255) {
            validData = false;
            break;
        }
    }
    
    // Initialize with defaults if invalid
    if (!validData) {
        Serial.println("[EEPROM] No valid data found, initializing defaults");
        
        // Clear entire structure
        memset(&eepromData, 0, sizeof(eepromData));
        
        // Set defaults
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            eepromData.outputStates[i] = false;
            eepromData.outputBrightness[i] = 255;
            eepromData.outputNames[i][0] = '\0';
        }
        strncpy(eepromData.deviceName, DEVICE_NAME, 39);
        eepromData.deviceName[39] = '\0';
        
        EEPROM.put(0, eepromData);
        EEPROM.commit();
        Serial.println("[EEPROM] Defaults saved to EEPROM");
    }
    
    int loadedCount = 0;
    int namedCount = 0;
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        // Load state and brightness from EEPROM
        outputStates[i] = eepromData.outputStates[i];
        outputBrightness[i] = eepromData.outputBrightness[i];
        
        // Load custom name - validate it's printable ASCII
        if (eepromData.outputNames[i][0] != '\0' && 
            eepromData.outputNames[i][0] != 0xFF &&
            eepromData.outputNames[i][0] >= 32 && 
            eepromData.outputNames[i][0] <= 126) {
            // Ensure null termination
            eepromData.outputNames[i][20] = '\0';
            outputNames[i] = String(eepromData.outputNames[i]);
            namedCount++;
        } else {
            outputNames[i] = "";
        }
        
        // Apply the loaded state to the output
        if (outputStates[i]) {
            analogWrite(outputPins[i], outputBrightness[i]);
            int brightPercent = map(outputBrightness[i], 0, 255, 0, 100);
            Serial.print("[EEPROM] Output " + String(i) + " (GPIO " + String(outputPins[i]) + "): ON @ " + String(brightPercent) + "%");
            if (outputNames[i].length() > 0) {
                Serial.println(" [Name: " + outputNames[i] + "]");
            } else {
                Serial.println("");
            }
            loadedCount++;
        } else {
            analogWrite(outputPins[i], 0);
        }
    }
    
    Serial.println("[EEPROM] Loaded " + String(loadedCount) + " active outputs, " + String(namedCount) + " custom names");
}

void saveAllOutputStates() {
    unsigned long startTime = millis();
    Serial.println("[EEPROM] Saving all output states (batch operation)...");
    
    // Read current EEPROM data
    EEPROM.get(0, eepromData);
    
    // Update all output states and brightness
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        eepromData.outputStates[i] = outputStates[i];
        eepromData.outputBrightness[i] = outputBrightness[i];
    }
    
    // Write back to EEPROM
    EEPROM.put(0, eepromData);
    EEPROM.commit();
    
    unsigned long duration = millis() - startTime;
    Serial.print("[EEPROM] Batch save complete: ");
    Serial.print(MAX_OUTPUTS);
    Serial.print(" outputs saved (");
    Serial.print(duration);
    Serial.println("ms)");
}

void initializeWebServer() {
    if (!server) return;
    
    // Serve minimal HTML page optimized for ESP8266 low RAM - send in chunks
    server->on("/", HTTP_GET, []() {
        server->setContentLength(CONTENT_LENGTH_UNKNOWN);
        server->send(200, "text/html", "");
        
        // Header chunk
        server->sendContent(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>RailHub8266</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;background:#1a1a1a;color:#e0e0e0;padding:20px}"
        ".card{background:#2a2a2a;border:1px solid #3a3a3a;padding:15px;margin-bottom:15px}h1{font-size:1.5rem;margin-bottom:10px}h2{font-size:1.2rem;margin-bottom:10px}"
        ".status{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:20px}.stat{background:#333;padding:10px;text-align:center}"
        ".value{font-size:1.5rem;color:#6c9bcf}.label{font-size:0.8rem;color:#999;margin-top:5px}"
        ".outputs{display:grid;gap:10px}.output{background:#333;padding:10px;display:flex;justify-content:space-between;align-items:center}"
        ".output.on{border-left:3px solid #4a9b6f}.toggle{width:60px;height:30px;background:#555;cursor:pointer;position:relative}"
        ".toggle.on{background:#4a9b6f}.toggle::after{content:'';position:absolute;width:26px;height:26px;background:#fff;top:2px;left:2px;transition:0.2s}"
        ".toggle.on::after{left:32px}.brightness{display:flex;align-items:center;gap:10px;margin-top:8px}"
        ".brightness input{flex:1;height:6px;border-radius:3px;background:#555;outline:none;-webkit-appearance:none}"
        ".brightness input::-webkit-slider-thumb{-webkit-appearance:none;width:16px;height:16px;border-radius:50%;background:#6c9bcf;cursor:pointer}"
        ".brightness input::-moz-range-thumb{width:16px;height:16px;border-radius:50%;background:#6c9bcf;cursor:pointer;border:none}"
        ".brightness span{min-width:40px;text-align:right;font-size:0.9rem;color:#999}"
        "button{background:#6c9bcf;color:#fff;border:none;padding:10px 20px;cursor:pointer;margin-right:10px}"
        "button:hover{background:#5a8bc0}.info{font-size:0.9rem;color:#999}</style></head><body>"));
        
        // Body start
        server->sendContent(F("<div class='card'><h1>🚂 RailHub8266</h1><p class='info'>"));
        server->sendContent(String(customDeviceName));
        server->sendContent(F("</p></div><div class='card'><h2>Status</h2><div class='status'>"
        "<div class='stat'><div class='value' id='heap'>-</div><div class='label'>Free RAM</div></div>"
        "<div class='stat'><div class='value' id='uptime'>-</div><div class='label'>Uptime</div></div>"
        "</div></div><div class='card'><h2>Controls</h2>"
        "<button onclick='allOn()'>All ON</button><button onclick='allOff()'>All OFF</button><button onclick='refresh()'>Refresh</button>"
        "</div><div class='card'><h2>Outputs</h2><div class='outputs' id='outputs'></div></div>"));
        
        // JavaScript chunk
        server->sendContent(F("<script>async function load(){try{const r=await fetch('/api/status');const d=await r.json();"
        "document.getElementById('heap').textContent=(d.freeHeap/1024).toFixed(1)+'KB';"
        "const s=Math.floor(d.uptime/1000);document.getElementById('uptime').textContent=s+'s';"
        "const o=document.getElementById('outputs');o.innerHTML='';"
        "d.outputs.forEach((out,i)=>{"
        "const div=document.createElement('div');div.className='output'+(out.active?' on':'');"
        "div.innerHTML=`<div><span>GPIO ${out.pin}</span>"
        "<div class='brightness'><input type='range' min='0' max='100' value='${out.brightness}' "
        "oninput='this.nextElementSibling.textContent=this.value+\"%\"' onchange='setBright(${out.pin},this.value)'>"
        "<span>${out.brightness}%</span></div></div>"
        "<div class='toggle ${out.active?'on':''}' onclick='tog(${out.pin})'></div>`;"
        "o.appendChild(div);});}catch(e){console.error(e);}}"));
        
        server->sendContent(F("async function tog(pin){try{const r=await fetch('/api/status');const d=await r.json();"
        "const out=d.outputs.find(o=>o.pin===pin);await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({pin:pin,active:!out.active,brightness:out.brightness})});load();}catch(e){console.error(e);}}"));
        
        server->sendContent(F("async function setBright(pin,val){try{const r=await fetch('/api/status');const d=await r.json();"
        "const out=d.outputs.find(o=>o.pin===pin);await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({pin:pin,active:out.active,brightness:parseInt(val)})});}catch(e){console.error(e);}}"));
        
        server->sendContent(F("async function allOn(){try{const r=await fetch('/api/status');const d=await r.json();"
        "for(const o of d.outputs){await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({pin:o.pin,active:true,brightness:100})});}load();}catch(e){console.error(e);}}"));
        
        server->sendContent(F("async function allOff(){try{const r=await fetch('/api/status');const d=await r.json();"
        "for(const o of d.outputs){await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({pin:o.pin,active:false,brightness:0})});}load();}catch(e){console.error(e);}}"));
        
        server->sendContent(F("function refresh(){load();}load();setInterval(load,2000);</script></body></html>"));
        server->sendContent("");  // End chunked transfer
    });
    
    // API endpoint for status
    server->on("/api/status", HTTP_GET, []() {
        unsigned long startTime = millis();
        IPAddress clientIP = server->client().remoteIP();
        Serial.print("[WEB] GET /api/status from ");
        Serial.println(clientIP.toString());
        
        DynamicJsonDocument doc(2048);
        doc["macAddress"] = macAddress;
        doc["name"] = customDeviceName;
        doc["wifiMode"] = WiFi.getMode() == WIFI_AP ? "AP" : "STA";
        doc["ip"] = WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
        doc["ssid"] = WiFi.getMode() == WIFI_AP ? String(AP_SSID) : WiFi.SSID();
        doc["apClients"] = WiFi.softAPgetStationNum();
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["uptime"] = millis();
        
        JsonArray outputs = doc.createNestedArray("outputs");
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            JsonObject output = outputs.createNestedObject();
            output["pin"] = outputPins[i];
            output["active"] = outputStates[i];
            output["brightness"] = map(outputBrightness[i], 0, 255, 0, 100);
            output["name"] = outputNames[i];
        }
        
        String response;
        serializeJson(doc, response);
        
        unsigned long duration = millis() - startTime;
        Serial.print("[WEB] Status response: ");
        Serial.print(response.length());
        Serial.print(" bytes, ");
        Serial.print(duration);
        Serial.println("ms");
        
        server->send(200, "application/json", response);
    });
    
    // API endpoint for updating output name
    server->on("/api/name", HTTP_POST, []() {
        unsigned long startTime = millis();
        IPAddress clientIP = server->client().remoteIP();
        String body = server->arg("plain");
        Serial.print("[WEB] POST /api/name from ");
        Serial.print(clientIP.toString());
        Serial.print(" (");
        Serial.print(body.length());
        Serial.println(" bytes)");
        
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            Serial.print("[ERROR] JSON deserialization failed: ");
            Serial.println(error.c_str());
            server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        int pin = doc["pin"];
        String name = doc["name"].as<String>();
        
        Serial.print("[WEB] Name update request: GPIO ");
        Serial.print(pin);
        Serial.print(" -> '");
        Serial.print(name);
        Serial.println("'");
        
        // Find output index by pin
        int outputIndex = -1;
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            if (outputPins[i] == pin) {
                outputIndex = i;
                break;
            }
        }
        
        if (outputIndex >= 0) {
            saveOutputName(outputIndex, name);
            unsigned long duration = millis() - startTime;
            Serial.print("[WEB] Name update complete (");
            Serial.print(duration);
            Serial.println("ms)");
            server->send(200, "application/json", "{\"success\":true}");
        } else {
            Serial.print("[ERROR] GPIO pin not found: ");
            Serial.println(pin);
            server->send(404, "application/json", "{\"error\":\"Output not found\"}");
        }
    });
    
    // API endpoint for control
    server->on("/api/control", HTTP_POST, []() {
        unsigned long startTime = millis();
        IPAddress clientIP = server->client().remoteIP();
        String body = server->arg("plain");
        Serial.print("[WEB] POST /api/control from ");
        Serial.print(clientIP.toString());
        Serial.print(" (");
        Serial.print(body.length());
        Serial.println(" bytes)");
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            Serial.print("[ERROR] JSON deserialization failed: ");
            Serial.println(error.c_str());
            server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        int pin = doc["pin"];
        bool active = doc["active"];
        int brightness = doc["brightness"] | 100;
        
        Serial.print("[WEB] Control request: GPIO ");
        Serial.print(pin);
        Serial.print(" -> ");
        Serial.print(active ? "ON" : "OFF");
        Serial.print(" @ ");
        Serial.print(brightness);
        Serial.println("%");
        
        executeOutputCommand(pin, active, brightness);
        
        unsigned long duration = millis() - startTime;
        Serial.print("[WEB] Control complete (");
        Serial.print(duration);
        Serial.println("ms)");
        
        server->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    
    // API endpoint to reset saved states
    server->on("/api/reset", HTTP_POST, []() {
        IPAddress clientIP = server->client().remoteIP();
        Serial.print("[WEB] POST /api/reset from ");
        Serial.println(clientIP.toString());
        Serial.println("[EEPROM] Resetting all saved states...");
        Serial.print("[EEPROM] Free heap before reset: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        
        // Clear EEPROM data
        for (int i = 0; i < EEPROM_SIZE; i++) {
            EEPROM.write(i, 0xFF);
        }
        EEPROM.commit();
        
        Serial.println("[EEPROM] All saved states cleared!");
        Serial.print("[EEPROM] Free heap after reset: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        
        server->send(200, "application/json", "{\"status\":\"reset_complete\"}");
    });
    
    server->begin();
    Serial.println("[WEB] Web server started on port 80");
    Serial.println("[WEB] Available endpoints:");
    Serial.println("[WEB]   GET  /              - Main control interface");
    Serial.println("[WEB]   GET  /api/status    - System and output status");
    Serial.println("[WEB]   POST /api/control   - Control output state/brightness");
    Serial.println("[WEB]   POST /api/name      - Update output name");
    Serial.println("[WEB]   POST /api/reset     - Reset all saved preferences");
}