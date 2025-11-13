#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Preferences.h>
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
AsyncWebServer* server = nullptr;
DNSServer dns;
Preferences preferences;

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
    Serial.println("\n\n========================================");
    Serial.println("  RailHub32 ESP32 Controller v1.0");
    Serial.println("========================================");
    Serial.println("[BOOT] Chip Model: " + String(ESP.getChipModel()));
    Serial.println("[BOOT] Chip Revision: " + String(ESP.getChipRevision()));
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
        server = new AsyncWebServer(80);
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
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        Serial.print("[OUTPUT] Configuring Output " + String(i) + " on GPIO " + String(outputPins[i]));
        pinMode(outputPins[i], OUTPUT);
        digitalWrite(outputPins[i], LOW);
        
        // Configure PWM channel for brightness control
        ledcSetup(i, 5000, 8); // 5kHz frequency, 8-bit resolution
        ledcAttachPin(outputPins[i], i);
        ledcWrite(i, 0);
        Serial.println(" - OK (PWM Ch" + String(i) + ", 5kHz, 8-bit)");
    }
    
    // Status LED
    Serial.println("[OUTPUT] Initializing status LED on GPIO " + String(STATUS_LED_PIN));
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH); // Turn on status LED
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
        
        // Blink status LED to indicate AP started
        for (int i = 0; i < 5; i++) {
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(150);
            digitalWrite(STATUS_LED_PIN, HIGH);
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
    
    // Create temporary server for WiFiManager portal
    AsyncWebServer portalServer(80);
    AsyncWiFiManager wifiManager(&portalServer, &dns);
    
    // Set custom parameters
    AsyncWiFiManagerParameter custom_device_name("device_name", "Device Name", customDeviceName, 40);
    
    // Add parameters to WiFiManager
    wifiManager.addParameter(&custom_device_name);
    
    // Custom HTML styling to match RailHub32 design
    const char* customHead = R"rawliteral(
<link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='0.9em' font-size='90'>🚂</text></svg>">
<style>
:root {
    --color-bg-primary: #0a0a0a;
    --color-bg-secondary: #141414;
    --color-bg-card: #1c1c1c;
    --color-border: #2a2a2a;
    --color-border-hover: #3a3a3a;
    --color-text-primary: #e8e8e8;
    --color-text-secondary: #a0a0a0;
    --color-accent: #6c9bcf;
    --color-accent-hover: #5a8bc0;
}
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
    font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, sans-serif;
    background: var(--color-bg-primary);
    color: var(--color-text-primary);
    padding: 20px;
    line-height: 1.6;
}
h1, h2, h3 { 
    font-weight: 300; 
    margin-bottom: 20px; 
    color: var(--color-text-primary);
    letter-spacing: 0.02em;
}
h1 { font-size: 2rem; margin-bottom: 30px; }
h3 { 
    font-size: 0.85rem; 
    text-transform: uppercase; 
    letter-spacing: 0.08em;
    color: var(--color-text-secondary);
    margin: 30px 0 15px 0;
}
.container {
    max-width: 600px;
    margin: 0 auto;
    background: var(--color-bg-card);
    padding: 40px;
    border: 1px solid var(--color-border);
}
form {
    display: flex;
    flex-direction: column;
    gap: 20px;
}
label {
    font-size: 0.9rem;
    color: var(--color-text-secondary);
    margin-bottom: 8px;
    display: block;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    font-size: 0.75rem;
}
input[type="text"], input[type="password"], select {
    width: 100%;
    padding: 12px 16px;
    background: var(--color-bg-primary);
    border: 1px solid var(--color-border);
    color: var(--color-text-primary);
    font-size: 1rem;
    font-family: inherit;
    transition: border-color 0.2s;
}
input[type="text"]:focus, input[type="password"]:focus, select:focus {
    outline: none;
    border-color: var(--color-accent);
}
button, .btn {
    padding: 12px 24px;
    background: var(--color-accent);
    border: 1px solid var(--color-accent);
    color: var(--color-text-primary);
    cursor: pointer;
    font-size: 0.85rem;
    font-weight: 400;
    letter-spacing: 0.05em;
    text-transform: uppercase;
    transition: all 0.2s;
    font-family: inherit;
    width: 100%;
    margin-top: 10px;
}
button:hover, .btn:hover {
    background: var(--color-accent-hover);
    border-color: var(--color-accent-hover);
}
.network-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
    margin: 20px 0;
}
.network-item {
    padding: 12px 16px;
    background: var(--color-bg-primary);
    border: 1px solid var(--color-border);
    cursor: pointer;
    transition: border-color 0.2s;
    display: flex;
    justify-content: space-between;
    align-items: center;
}
.network-item:hover {
    border-color: var(--color-border-hover);
}
.q {
    color: var(--color-accent);
    font-weight: 400;
}
a {
    color: var(--color-accent);
    text-decoration: none;
    transition: color 0.2s;
}
a:hover {
    color: var(--color-accent-hover);
}
.header {
    text-align: center;
    margin-bottom: 40px;
    padding-bottom: 25px;
    border-bottom: 1px solid var(--color-border);
}
.info {
    background: var(--color-bg-secondary);
    padding: 16px;
    border: 1px solid var(--color-border);
    margin: 20px 0;
    font-size: 0.85rem;
    color: var(--color-text-secondary);
}
.scanning {
    text-align: center;
    padding: 20px;
    color: var(--color-text-secondary);
}
</style>
<script>
// Auto-scan on page load and handle scan button
document.addEventListener('DOMContentLoaded', function() {
    var scanBtn = document.querySelector('button[onclick*="scan"]');
    if (scanBtn) {
        scanBtn.addEventListener('click', function(e) {
            e.preventDefault();
            var networksDiv = document.querySelector('.network-list') || document.querySelector('div:has(> a[href*="wifisave"])').parentElement;
            if (networksDiv) {
                networksDiv.innerHTML = '<div class="scanning">Scanning for networks...</div>';
            }
            // Reload page to trigger new scan
            setTimeout(function() {
                window.location.reload();
            }, 1000);
        });
    }
    
    // Auto-scroll to networks if they exist
    var firstNetwork = document.querySelector('a[href*="wifisave"]');
    if (firstNetwork) {
        firstNetwork.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    }
});
</script>
)rawliteral";

    wifiManager.setCustomHeadElement(customHead);
    
    // Set minimum signal quality to show networks (lower = show more networks)
    wifiManager.setMinimumSignalQuality(8);
    
    // Configure WiFi scan settings to improve performance
    wifiManager.setRemoveDuplicateAPs(true);
    
    // Set save config callback
    wifiManager.setSaveConfigCallback([]() {
        Serial.println("[WIFI] Configuration saved!");
        Serial.print("[WIFI] Device Name: ");
        Serial.println(customDeviceName);
        Serial.println("[WIFI] WiFi credentials will be used on next boot");
    });
    
    // Set configuration portal timeout (0 = no timeout for easier mobile config)
    wifiManager.setConfigPortalTimeout(0);
    
    // Disable debug output to improve web interface performance
    wifiManager.setDebugOutput(false);
    
    // Set AP callback
    wifiManager.setAPCallback([](AsyncWiFiManager *myWiFiManager) {
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
        
        // Blink LED to indicate config mode
        for (int i = 0; i < 10; i++) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(100);
        }
        digitalWrite(STATUS_LED_PIN, HIGH);
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
        
        // Solid LED to indicate connected
        digitalWrite(STATUS_LED_PIN, HIGH);
    } else {
        // Failed to connect - fallback to AP mode
        Serial.println("[ERROR] Failed to connect - starting fallback AP mode");
        wifiConnected = false;
        initializeWiFi();
    }
}

void checkConfigPortalTrigger() {
    // Check if portal trigger button is pressed
    if (digitalRead(PORTAL_TRIGGER_PIN) == LOW) {
        if (portalButtonPressTime == 0) {
            portalButtonPressTime = millis();
            Serial.println("[PORTAL] Config button pressed (hold for 3s to trigger)");
        } else {
            unsigned long holdDuration = millis() - portalButtonPressTime;
            
            // Warning at 2.5 seconds
            if (holdDuration > 2500 && holdDuration < 2600 && !portalRunning) {
                Serial.println("[PORTAL] Warning: Portal trigger in 0.5s...");
            }
            
            if (holdDuration > PORTAL_TRIGGER_DURATION && !portalRunning) {
                Serial.println("[PORTAL] Portal trigger detected! Resetting WiFi and restarting...");
                Serial.print("[PORTAL] Free heap before reset: ");
                Serial.print(ESP.getFreeHeap());
                Serial.println(" bytes");
                portalRunning = true;
                
                // Blink LED rapidly
                Serial.println("[PORTAL] Blinking status LED (confirmation)");
                for (int i = 0; i < 20; i++) {
                    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
                    delay(50);
                }
                
                // Clear WiFi credentials from preferences
                Serial.println("[PORTAL] Clearing WiFi credentials from NVRAM...");
                if (!preferences.begin("railhub32", false)) {
                    Serial.println("[ERROR] Failed to open preferences for credential removal");
                } else {
                    preferences.remove("wifi_ssid");
                    preferences.remove("wifi_pass");
                    preferences.end();
                    Serial.println("[PORTAL] WiFi credentials cleared");
                }
                
                // Clear ESP32 WiFi settings
                Serial.println("[PORTAL] Disconnecting WiFi and clearing saved networks...");
                WiFi.disconnect(true, true);
                delay(1000);
                
                // Restart to trigger portal
                Serial.println("[PORTAL] Restarting ESP32 in 1s...");
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
    }
}

void saveCustomParameters() {
    Serial.println("[NVRAM] Saving custom parameters...");
    
    if (!preferences.begin("railhub32", false)) {
        Serial.println("[ERROR] Failed to open preferences for saving custom parameters");
        return;
    }
    
    size_t written = preferences.putString("deviceName", customDeviceName);
    preferences.end();
    
    if (written > 0) {
        Serial.print("[NVRAM] Custom parameters saved: Device Name = '");
        Serial.print(customDeviceName);
        Serial.print("' (");
        Serial.print(written);
        Serial.println(" bytes)");
    } else {
        Serial.println("[ERROR] Failed to save custom parameters");
    }
}

void loadCustomParameters() {
    Serial.println("[NVRAM] Loading custom parameters...");
    
    if (!preferences.begin("railhub32", true)) {
        Serial.println("[ERROR] Failed to open preferences for loading custom parameters");
        strncpy(customDeviceName, DEVICE_NAME, 40);
        customDeviceName[39] = '\0';
        Serial.print("[NVRAM] Using default device name: '");
        Serial.print(customDeviceName);
        Serial.println("'");
        return;
    }
    
    String savedName = preferences.getString("deviceName", DEVICE_NAME);
    preferences.end();
    
    strncpy(customDeviceName, savedName.c_str(), 40);
    customDeviceName[39] = '\0'; // Ensure null termination
    
    if (savedName == DEVICE_NAME) {
        Serial.print("[NVRAM] No custom device name found, using default: '");
        Serial.print(customDeviceName);
        Serial.println("'");
    } else {
        Serial.print("[NVRAM] Loaded custom device name: '");
        Serial.print(customDeviceName);
        Serial.print("' (");
        Serial.print(savedName.length());
        Serial.println(" chars)");
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
        ledcWrite(outputIndex, outputBrightness[outputIndex]);
    } else {
        ledcWrite(outputIndex, 0);
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
    
    if (!preferences.begin("railhub32", false)) {
        Serial.print("[ERROR] Failed to open preferences for saving Output ");
        Serial.println(index);
        return;
    }
    
    // Create keys for state and brightness
    String stateKey = "out_" + String(index) + "_s";
    String brightKey = "out_" + String(index) + "_b";
    
    size_t stateWritten = preferences.putBool(stateKey.c_str(), outputStates[index]);
    size_t brightWritten = preferences.putUChar(brightKey.c_str(), outputBrightness[index]);
    
    preferences.end();
    
    if (stateWritten > 0 && brightWritten > 0) {
        Serial.print("[NVRAM] Saved state for Output ");
        Serial.print(index);
        Serial.print(" (GPIO ");
        Serial.print(outputPins[index]);
        Serial.print("): ");
        Serial.print(outputStates[index] ? "ON" : "OFF");
        Serial.print(" @ ");
        Serial.print(outputBrightness[index]);
        Serial.println(" PWM");
    } else {
        Serial.print("[ERROR] Failed to save state for Output ");
        Serial.println(index);
    }
}

void saveOutputName(int index, String name) {
    if (index < 0 || index >= MAX_OUTPUTS) {
        Serial.println("[ERROR] Invalid output index for name save: " + String(index));
        return;
    }
    
    if (!preferences.begin("railhub32", false)) {
        Serial.println("[ERROR] Failed to open preferences for name save");
        return;
    }
    
    String nameKey = "out_" + String(index) + "_n";
    
    // If name is empty, remove the preference key
    if (name.length() == 0 || name.trim().length() == 0) {
        bool removed = preferences.remove(nameKey.c_str());
        preferences.end();
        outputNames[index] = "";
        if (removed) {
            Serial.println("[NVRAM] Removed custom name for Output " + String(index) + " (GPIO " + String(outputPins[index]) + ") - using default");
        } else {
            Serial.println("[NVRAM] No custom name to remove for Output " + String(index));
        }
        return;
    }
    
    size_t written = preferences.putString(nameKey.c_str(), name);
    preferences.end();
    
    if (written > 0) {
        outputNames[index] = name;
        Serial.println("[NVRAM] Saved name for Output " + String(index) + " (GPIO " + String(outputPins[index]) + "): '" + name + "' (" + String(written) + " bytes)");
    } else {
        Serial.println("[ERROR] Failed to save name for output " + String(index));
    }
}

void loadOutputStates() {
    Serial.println("[NVRAM] Loading saved output states...");
    
    if (!preferences.begin("railhub32", true)) {
        Serial.println("[ERROR] Failed to open preferences in read-only mode");
        return;
    }
    
    int loadedCount = 0;
    int namedCount = 0;
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        String stateKey = "out_" + String(i) + "_s";
        String brightKey = "out_" + String(i) + "_b";
        String nameKey = "out_" + String(i) + "_n";
        
        // Load state (default to false if not found)
        outputStates[i] = preferences.getBool(stateKey.c_str(), false);
        
        // Load brightness (default to 255 if not found)
        outputBrightness[i] = preferences.getUChar(brightKey.c_str(), 255);
        
        // Load custom name (default to empty string)
        outputNames[i] = preferences.getString(nameKey.c_str(), "");
        if (outputNames[i].length() > 0) {
            namedCount++;
        }
        
        // Apply the loaded state to the output
        if (outputStates[i]) {
            ledcWrite(i, outputBrightness[i]);
            int brightPercent = map(outputBrightness[i], 0, 255, 0, 100);
            Serial.print("[NVRAM] Output " + String(i) + " (GPIO " + String(outputPins[i]) + "): ON @ " + String(brightPercent) + "%");
            if (outputNames[i].length() > 0) {
                Serial.println(" [Name: " + outputNames[i] + "]");
            } else {
                Serial.println("");
            }
            loadedCount++;
        } else {
            ledcWrite(i, 0);
        }
    }
    
    preferences.end();
    Serial.println("[NVRAM] Loaded " + String(loadedCount) + " active outputs, " + String(namedCount) + " custom names");
}

void saveAllOutputStates() {
    unsigned long startTime = millis();
    Serial.println("[NVRAM] Saving all output states (batch operation)...");
    
    if (!preferences.begin("railhub32", false)) {
        Serial.println("[ERROR] Failed to open preferences for batch save");
        return;
    }
    
    int savedCount = 0;
    int failedCount = 0;
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        String stateKey = "out_" + String(i) + "_s";
        String brightKey = "out_" + String(i) + "_b";
        
        size_t stateWritten = preferences.putBool(stateKey.c_str(), outputStates[i]);
        size_t brightWritten = preferences.putUChar(brightKey.c_str(), outputBrightness[i]);
        
        if (stateWritten > 0 && brightWritten > 0) {
            savedCount++;
        } else {
            failedCount++;
            Serial.print("[ERROR] Failed to save Output ");
            Serial.println(i);
        }
    }
    
    preferences.end();
    
    unsigned long duration = millis() - startTime;
    Serial.print("[NVRAM] Batch save complete: ");
    Serial.print(savedCount);
    Serial.print(" outputs saved, ");
    Serial.print(failedCount);
    Serial.print(" failed (");
    Serial.print(duration);
    Serial.println("ms)");
}

void initializeWebServer() {
    if (!server) return;
    
    // Serve main HTML page with RailHub32 styling
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RailHub32 ESP32 - )rawliteral" + String(customDeviceName) + R"rawliteral(</title>
    <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='0.9em' font-size='90'>🚂</text></svg>">
    <style>
        :root {
            --color-bg-primary: #0a0a0a;
            --color-bg-secondary: #141414;
            --color-bg-tertiary: #1a1a1a;
            --color-bg-card: #1c1c1c;
            --color-border: #2a2a2a;
            --color-border-hover: #3a3a3a;
            --color-text-primary: #e8e8e8;
            --color-text-secondary: #a0a0a0;
            --color-text-muted: #707070;
            --color-accent: #6c9bcf;
            --color-accent-hover: #5a8bc0;
            --color-success: #4a9b6f;
            --color-danger: #b85c5c;
            --color-warning: #c9a257;
            --font-primary: 'Segoe UI', -apple-system, BlinkMacSystemFont, 'Helvetica Neue', sans-serif;
        }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: var(--font-primary);
            background: var(--color-bg-primary);
            color: var(--color-text-primary);
            min-height: 100vh;
            font-size: 15px;
            line-height: 1.6;
            letter-spacing: 0.01em;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 30px 40px;
        }
        header {
            text-align: left;
            margin-bottom: 50px;
            padding-bottom: 25px;
            border-bottom: 1px solid var(--color-border);
        }
        .header-content {
            margin-bottom: 20px;
        }
        h1 {
            font-size: 2rem;
            margin-bottom: 8px;
            font-weight: 300;
            letter-spacing: 0.03em;
        }
        header h1 {
            font-size: 2rem;
            margin-bottom: 8px;
            font-weight: 300;
            letter-spacing: 0.03em;
        }
        header p {
            font-size: 0.95rem;
            color: var(--color-text-secondary);
            font-weight: 300;
        }
        .language-selector {
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
            margin-top: 16px;
        }
        .lang-btn {
            padding: 8px 12px;
            background: transparent;
            border: 1px solid var(--color-border);
            color: var(--color-text-secondary);
            cursor: pointer;
            font-size: 0.8rem;
            font-weight: 400;
            letter-spacing: 0.05em;
            transition: all 0.2s ease;
            text-transform: uppercase;
        }
        .lang-btn:hover {
            border-color: var(--color-border-hover);
            background: var(--color-bg-tertiary);
        }
        .lang-btn.active {
            background: var(--color-accent);
            border-color: var(--color-accent);
            color: var(--color-text-primary);
        }
        nav {
            display: flex;
            justify-content: flex-start;
            margin-bottom: 40px;
            border-bottom: 1px solid var(--color-border);
        }
        .tab-button {
            background: transparent;
            border: none;
            color: var(--color-text-secondary);
            padding: 14px 32px;
            border-radius: 0;
            cursor: pointer;
            font-size: 0.9rem;
            font-weight: 300;
            letter-spacing: 0.02em;
            transition: all 0.2s ease;
            border-bottom: 2px solid transparent;
            text-transform: uppercase;
        }
        .tab-button:hover {
            color: var(--color-text-primary);
        }
        .tab-button.active {
            font-weight: 400;
            color: var(--color-text-primary);
            border-bottom-color: var(--color-accent);
        }
        main { min-height: 500px; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        .toolbar {
            display: flex;
            gap: 12px;
            margin-bottom: 30px;
        }
        .btn {
            padding: 11px 24px;
            border: 1px solid var(--color-border);
            border-radius: 2px;
            cursor: pointer;
            font-size: 0.85rem;
            font-weight: 400;
            letter-spacing: 0.05em;
            transition: all 0.2s ease;
            text-transform: uppercase;
            background: transparent;
            color: var(--color-text-primary);
        }
        .btn:hover {
            border-color: var(--color-border-hover);
            background: var(--color-bg-tertiary);
        }
        .btn-primary {
            background: var(--color-accent);
            border-color: var(--color-accent);
        }
        .btn-primary:hover {
            background: var(--color-accent-hover);
            border-color: var(--color-accent-hover);
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 16px;
            margin-bottom: 40px;
        }
        .status-card {
            background: var(--color-bg-card);
            padding: 24px;
            border: 1px solid var(--color-border);
            transition: border-color 0.2s ease;
        }
        .status-card:hover {
            border-color: var(--color-border-hover);
        }
        .status-value {
            font-size: 2.2rem;
            font-weight: 300;
            color: var(--color-accent);
            margin-bottom: 8px;
            letter-spacing: -0.02em;
        }
        .status-label {
            color: var(--color-text-secondary);
            font-size: 0.85rem;
            font-weight: 300;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        .master-brightness-card {
            margin-bottom: 30px;
            max-width: 100%;
        }
        .outputs-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
            gap: 16px;
        }
        .output-card {
            background: var(--color-bg-card);
            border: 1px solid var(--color-border);
            padding: 24px;
            transition: all 0.2s ease;
        }
        .output-card:hover {
            border-color: var(--color-border-hover);
        }
        .output-card.active {
            border-left: 2px solid var(--color-success);
        }
        .output-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
            padding-bottom: 16px;
            border-bottom: 1px solid var(--color-border);
        }
        .output-name {
            font-size: 1.1rem;
            font-weight: 400;
            color: var(--color-text-primary);
            letter-spacing: 0.02em;
            cursor: pointer;
            padding: 4px 8px;
            border-radius: 4px;
            transition: background 0.2s;
        }
        .output-name:hover {
            background: var(--color-bg-tertiary);
        }
        .output-name-edit {
            display: flex;
            align-items: center;
        }
        .output-status {
            padding: 5px 14px;
            font-size: 0.7rem;
            font-weight: 400;
            letter-spacing: 0.08em;
            text-transform: uppercase;
            border: 1px solid;
            background: transparent;
        }
        .output-status.on {
            color: var(--color-success);
            border-color: var(--color-success);
        }
        .output-status.off {
            color: var(--color-text-muted);
            border-color: var(--color-border);
        }
        .output-info {
            font-size: 0.85rem;
            color: var(--color-text-secondary);
            margin-bottom: 20px;
        }
        .output-info strong {
            color: var(--color-text-muted);
            font-weight: 400;
            text-transform: uppercase;
            font-size: 0.75rem;
            letter-spacing: 0.05em;
        }
        .output-controls {
            display: flex;
            gap: 16px;
            align-items: center;
            justify-content: space-between;
            margin-top: 16px;
        }
        .toggle-switch {
            position: relative;
            width: 44px;
            height: 22px;
            background: var(--color-bg-tertiary);
            border: 1px solid var(--color-border);
            cursor: pointer;
            transition: all 0.2s ease;
        }
        .toggle-switch.active {
            background: var(--color-accent);
            border-color: var(--color-accent);
        }
        .toggle-switch::before {
            content: '';
            position: absolute;
            top: 2px;
            left: 2px;
            width: 16px;
            height: 16px;
            background: var(--color-text-primary);
            transition: transform 0.2s ease;
        }
        .toggle-switch.active::before {
            transform: translateX(22px);
        }
        .brightness-control {
            display: flex;
            align-items: center;
            gap: 12px;
            flex: 1;
        }
        .brightness-label {
            font-size: 0.75rem;
            color: var(--color-text-muted);
            text-transform: uppercase;
            letter-spacing: 0.05em;
            min-width: 40px;
        }
        .brightness-slider {
            flex: 1;
            height: 2px;
            background: var(--color-border);
            outline: none;
            cursor: pointer;
            -webkit-appearance: none;
            appearance: none;
        }
        .brightness-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 14px;
            height: 14px;
            background: var(--color-text-primary);
            cursor: pointer;
        }
        .brightness-slider::-moz-range-thumb {
            width: 14px;
            height: 14px;
            background: var(--color-text-primary);
            cursor: pointer;
            border: none;
        }
        .brightness-value {
            font-size: 0.85rem;
            color: var(--color-text-secondary);
            min-width: 35px;
            text-align: right;
        }
        .section-title {
            font-size: 0.75rem;
            font-weight: 400;
            text-transform: uppercase;
            letter-spacing: 0.08em;
            color: var(--color-text-muted);
            margin-bottom: 24px;
        }
        .loading {
            display: inline-block;
            width: 18px;
            height: 18px;
            border: 2px solid var(--color-border);
            border-top-color: var(--color-accent);
            animation: spin 1s linear infinite;
        }
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
        footer {
            text-align: center;
            padding: 30px 20px;
            margin-top: 60px;
            border-top: 1px solid var(--color-border);
            color: var(--color-text-muted);
            font-size: 0.85rem;
            font-weight: 300;
        }
        footer a {
            color: var(--color-accent);
            text-decoration: none;
            transition: color 0.2s ease;
        }
        footer a:hover {
            color: var(--color-accent-hover);
        }
        @media (max-width: 768px) {
            .container { padding: 20px; }
            header { margin-bottom: 30px; }
            header h1 { font-size: 1.6rem; }
            nav { overflow-x: auto; }
            .tab-button { padding: 14px 24px; white-space: nowrap; }
            .outputs-grid { grid-template-columns: 1fr; }
            .toolbar { flex-direction: column; }
            .toolbar .btn { width: 100%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="header-content">
                <h1>🚂 RailHub32 ESP32</h1>
                <p id="deviceName">)rawliteral" + String(customDeviceName) + R"rawliteral(</p>
                <div class="language-selector">
                    <button class="lang-btn active" data-lang="en">EN</button>
                    <button class="lang-btn" data-lang="de">DE</button>
                    <button class="lang-btn" data-lang="fr">FR</button>
                    <button class="lang-btn" data-lang="it">IT</button>
                    <button class="lang-btn" data-lang="zh">中文</button>
                    <button class="lang-btn" data-lang="hi">हिं</button>
                </div>
            </div>
        </header>

        <nav>
            <button id="statusTab" class="tab-button active" data-i18n="nav.status">Status</button>
            <button id="outputsTab" class="tab-button" data-i18n="nav.outputs">Outputs</button>
        </nav>

        <main>
            <!-- Status Tab -->
            <div id="statusContent" class="tab-content active">
                <div class="toolbar">
                    <button id="refreshStatus" class="btn btn-primary" data-i18n="buttons.refresh">🔄 Refresh</button>
                </div>
                <h3 class="section-title" data-i18n="status.deviceInfo">Device Information</h3>
                <div class="status-grid">
                    <div class="status-card">
                        <div class="status-value" id="deviceIp">Loading...</div>
                        <div class="status-label" data-i18n="status.apIp">AP IP Address</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="apClients">0</div>
                        <div class="status-label" data-i18n="status.clients">Connected Clients</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="uptime">0s</div>
                        <div class="status-label" data-i18n="status.uptime">Uptime</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="freeHeap">0 KB</div>
                        <div class="status-label" data-i18n="status.freeHeap">Free Heap</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="macAddress">)rawliteral" + macAddress + R"rawliteral(</div>
                        <div class="status-label" data-i18n="status.macAddr">MAC Address</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value">)rawliteral" + String(AP_SSID) + R"rawliteral(</div>
                        <div class="status-label" data-i18n="status.apSsid">AP SSID</div>
                    </div>
                </div>
            </div>

            <!-- Outputs Tab -->
            <div id="outputsContent" class="tab-content">
                <div class="toolbar">
                    <button id="refreshOutputs" class="btn btn-primary" data-i18n="buttons.refresh">🔄 Refresh</button>
                    <button id="allOn" class="btn" data-i18n="buttons.allOn">💡 All On</button>
                    <button id="allOff" class="btn" data-i18n="buttons.allOff">⚫ All Off</button>
                </div>
                
                <!-- Master Brightness Control -->
                <div class="output-card master-brightness-card">
                    <div class="output-header">
                        <div class="output-name" data-i18n="outputs.master">Master Brightness Control</div>
                        <div class="output-status on" data-i18n="outputs.all">ALL</div>
                    </div>
                    <div class="output-info" data-i18n="outputs.masterDesc">
                        Adjusts brightness for all active outputs simultaneously
                    </div>
                    <div class="brightness-control">
                        <span class="brightness-label" data-i18n="outputs.brightness">Brightness</span>
                        <input type="range" 
                               id="masterBrightness" 
                               class="brightness-slider" 
                               min="0" 
                               max="100" 
                               value="100">
                        <span id="masterBrightnessValue" class="brightness-value">100%</span>
                    </div>
                </div>
                
                <h3 class="section-title" data-i18n="outputs.individual">Individual Output Control</h3>
                <div id="outputsGrid" class="outputs-grid">
                    <!-- Outputs will be loaded here -->
                </div>
            </div>
        </main>
        
        <footer>
            Made with ❤️ by innoMO
        </footer>
    </div>

    <script>
        // Translations
        const translations = {
            en: {
                nav: { status: 'Status', outputs: 'Outputs' },
                buttons: { refresh: '🔄 Refresh', allOn: '💡 All On', allOff: '⚫ All Off' },
                status: { deviceInfo: 'Device Information', apIp: 'AP IP Address', clients: 'Connected Clients', uptime: 'Uptime', freeHeap: 'Free Heap', macAddr: 'MAC Address', apSsid: 'AP SSID' },
                outputs: { master: 'Master Brightness Control', masterDesc: 'Adjusts brightness for all active outputs simultaneously', individual: 'Individual Output Control', output: 'Output', pin: 'Pin', brightness: 'Brightness', all: 'ALL', on: 'ON', off: 'OFF', editName: 'Edit Name', saveName: 'Save', cancelEdit: 'Cancel' }
            },
            de: {
                nav: { status: 'Status', outputs: 'Ausgänge' },
                buttons: { refresh: '🔄 Aktualisieren', allOn: '💡 Alle Ein', allOff: '⚫ Alle Aus' },
                status: { deviceInfo: 'Geräteinformationen', apIp: 'AP IP-Adresse', clients: 'Verbundene Clients', uptime: 'Laufzeit', freeHeap: 'Freier Speicher', macAddr: 'MAC-Adresse', apSsid: 'AP SSID' },
                outputs: { master: 'Master-Helligkeitssteuerung', masterDesc: 'Passt die Helligkeit aller aktiven Ausgänge gleichzeitig an', individual: 'Individuelle Ausgangssteuerung', output: 'Ausgang', pin: 'Pin', brightness: 'Helligkeit', all: 'ALLE', on: 'EIN', off: 'AUS', editName: 'Name bearbeiten', saveName: 'Speichern', cancelEdit: 'Abbrechen' }
            },
            fr: {
                nav: { status: 'Statut', outputs: 'Sorties' },
                buttons: { refresh: '🔄 Actualiser', allOn: '💡 Tous Allumés', allOff: '⚫ Tous Éteints' },
                status: { deviceInfo: 'Informations sur l\'appareil', apIp: 'Adresse IP AP', clients: 'Clients connectés', uptime: 'Temps de fonctionnement', freeHeap: 'Mémoire libre', macAddr: 'Adresse MAC', apSsid: 'AP SSID' },
                outputs: { master: 'Contrôle principal de la luminosité', masterDesc: 'Ajuste la luminosité de toutes les sorties actives simultanément', individual: 'Contrôle individuel des sorties', output: 'Sortie', pin: 'Broche', brightness: 'Luminosité', all: 'TOUS', on: 'ALLUMÉ', off: 'ÉTEINT', editName: 'Modifier le nom', saveName: 'Enregistrer', cancelEdit: 'Annuler' }
            },
            it: {
                nav: { status: 'Stato', outputs: 'Uscite' },
                buttons: { refresh: '🔄 Aggiorna', allOn: '💡 Tutti Accesi', allOff: '⚫ Tutti Spenti' },
                status: { deviceInfo: 'Informazioni dispositivo', apIp: 'Indirizzo IP AP', clients: 'Client connessi', uptime: 'Tempo di attività', freeHeap: 'Memoria libera', macAddr: 'Indirizzo MAC', apSsid: 'AP SSID' },
                outputs: { master: 'Controllo luminosità principale', masterDesc: 'Regola la luminosità di tutte le uscite attive simultaneamente', individual: 'Controllo uscite individuali', output: 'Uscita', pin: 'Pin', brightness: 'Luminosità', all: 'TUTTI', on: 'ACCESO', off: 'SPENTO', editName: 'Modifica nome', saveName: 'Salva', cancelEdit: 'Annulla' }
            },
            zh: {
                nav: { status: '状态', outputs: '输出' },
                buttons: { refresh: '🔄 刷新', allOn: '💡 全部开启', allOff: '⚫ 全部关闭' },
                status: { deviceInfo: '设备信息', apIp: 'AP IP地址', clients: '已连接客户端', uptime: '运行时间', freeHeap: '可用内存', macAddr: 'MAC地址', apSsid: 'AP SSID' },
                outputs: { master: '主亮度控制', masterDesc: '同时调整所有活动输出的亮度', individual: '单独输出控制', output: '输出', pin: '引脚', brightness: '亮度', all: '全部', on: '开启', off: '关闭', editName: '编辑名称', saveName: '保存', cancelEdit: '取消' }
            },
            hi: {
                nav: { status: 'स्थिति', outputs: 'आउटपुट' },
                buttons: { refresh: '🔄 रिफ्रेश', allOn: '💡 सभी चालू', allOff: '⚫ सभी बंद' },
                status: { deviceInfo: 'डिवाइस जानकारी', apIp: 'AP IP पता', clients: 'कनेक्टेड क्लाइंट', uptime: 'अपटाइम', freeHeap: 'खाली मेमोरी', macAddr: 'MAC पता', apSsid: 'AP SSID' },
                outputs: { master: 'मास्टर चमक नियंत्रण', masterDesc: 'सभी सक्रिय आउटपुट की चमक एक साथ समायोजित करता है', individual: 'व्यक्तिगत आउटपुट नियंत्रण', output: 'आउटपुट', pin: 'पिन', brightness: 'चमक', all: 'सभी', on: 'चालू', off: 'बंद', editName: 'नाम संपादित करें', saveName: 'सहेजें', cancelEdit: 'रद्द करें' }
            }
        };

        // Language management
        let currentLang = localStorage.getItem('railhub32_lang') || 'en';

        function updateLanguage(lang) {
            currentLang = lang;
            localStorage.setItem('railhub32_lang', lang);
            
            // Update all elements with data-i18n
            document.querySelectorAll('[data-i18n]').forEach(elem => {
                const key = elem.getAttribute('data-i18n');
                const keys = key.split('.');
                let value = translations[lang];
                for (const k of keys) {
                    value = value[k];
                }
                elem.textContent = value;
            });
            
            // Update language buttons
            document.querySelectorAll('.lang-btn').forEach(btn => {
                btn.classList.toggle('active', btn.getAttribute('data-lang') === lang);
            });
            
            // Reload outputs to update labels
            if (document.getElementById('outputsContent').classList.contains('active')) {
                loadOutputs();
            }
        }

        // Language button handlers
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                updateLanguage(btn.getAttribute('data-lang'));
            });
        });

        // Initialize language on page load
        updateLanguage(currentLang);

        // Tab switching with persistence
        function switchTab(tabName) {
            document.querySelectorAll('.tab-button').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            document.getElementById(tabName + 'Tab').classList.add('active');
            document.getElementById(tabName + 'Content').classList.add('active');
            localStorage.setItem('railhub32_tab', tabName);
            
            // Load outputs when switching to outputs tab
            if (tabName === 'outputs') {
                loadOutputs();
            }
        }
        
        document.querySelectorAll('.tab-button').forEach(button => {
            button.addEventListener('click', function() {
                const tabName = this.id.replace('Tab', '');
                switchTab(tabName);
            });
        });
        
        // Restore last selected tab
        const savedTab = localStorage.getItem('railhub32_tab') || 'status';
        switchTab(savedTab);

        // Load status
        async function loadStatus() {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                document.getElementById('deviceIp').textContent = data.ip;
                document.getElementById('macAddress').textContent = data.macAddress;
                document.getElementById('apClients').textContent = data.apClients || 0;
                
                const uptimeSeconds = Math.floor(data.uptime / 1000);
                const hours = Math.floor(uptimeSeconds / 3600);
                const minutes = Math.floor((uptimeSeconds % 3600) / 60);
                const seconds = uptimeSeconds % 60;
                document.getElementById('uptime').textContent = 
                    hours > 0 ? `${hours}h ${minutes}m` : minutes > 0 ? `${minutes}m ${seconds}s` : `${seconds}s`;
                
                const heapKB = Math.floor(data.freeHeap / 1024);
                document.getElementById('freeHeap').textContent = heapKB + ' KB';
                
                return data;
            } catch (error) {
                console.error('Error loading status:', error);
            }
        }

        // Load outputs
        async function loadOutputs() {
            // Don't update if user is editing a name
            const activeElement = document.activeElement;
            if (activeElement && activeElement.id && activeElement.id.startsWith('name-input-')) {
                return;
            }
            
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                // Update master brightness to match first active output (if any)
                const activeOutputs = data.outputs.filter(o => o.active);
                if (activeOutputs.length > 0) {
                    const avgBrightness = Math.round(
                        activeOutputs.reduce((sum, o) => sum + o.brightness, 0) / activeOutputs.length
                    );
                    document.getElementById('masterBrightness').value = avgBrightness;
                    document.getElementById('masterBrightnessValue').textContent = avgBrightness + '%';
                }
                
                const grid = document.getElementById('outputsGrid');
                grid.innerHTML = '';
                
                data.outputs.forEach((output, index) => {
                    const t = translations[currentLang].outputs;
                    // Use default name from current language if output name is empty
                    const displayName = (output.name && output.name.trim() !== '') ? output.name : `${t.output} ${index + 1}`;
                    // Store the actual value from server (empty string if no custom name)
                    const inputValue = output.name || '';
                    const card = document.createElement('div');
                    card.className = 'output-card' + (output.active ? ' active' : '');
                    card.innerHTML = `
                        <div class="output-header">
                            <div class="output-name" id="name-display-${output.pin}" onclick="editOutputName(${output.pin}, '${inputValue}', ${index})">${displayName}</div>
                            <div class="output-name-edit" id="name-edit-${output.pin}" style="display: none;">
                                <input type="text" id="name-input-${output.pin}" value="${inputValue}" placeholder="${t.output} ${index + 1}" maxlength="20" style="width: 130px; padding: 4px; background: var(--color-bg-tertiary); border: 1px solid var(--color-border); color: var(--color-text-primary); border-radius: 4px;">
                                <button onclick="saveOutputName(${output.pin})" style="padding: 4px 8px; margin-left: 4px; background: var(--color-success); border: none; color: white; border-radius: 4px; cursor: pointer; font-size: 11px;">${t.saveName}</button>
                                <button onclick="cancelEditName(${output.pin})" style="padding: 4px 8px; margin-left: 2px; background: var(--color-danger); border: none; color: white; border-radius: 4px; cursor: pointer; font-size: 11px;">${t.cancelEdit}</button>
                            </div>
                            <div class="output-status ${output.active ? 'on' : 'off'}" data-pin="${output.pin}">
                                ${output.active ? t.on : t.off}
                            </div>
                        </div>
                        <div class="output-info">
                            <strong>${t.pin}:</strong> GPIO ${output.pin}
                        </div>
                        <div class="output-controls">
                            <div class="toggle-switch ${output.active ? 'active' : ''}" 
                                 data-pin="${output.pin}" 
                                 onclick="toggleOutput(${output.pin})">
                            </div>
                            <div class="brightness-control">
                                <span class="brightness-label">${t.brightness}</span>
                                <input type="range" 
                                       class="brightness-slider" 
                                       min="0" max="100" 
                                       value="${output.brightness}" 
                                       data-pin="${output.pin}"
                                       onchange="setBrightness(${output.pin}, this.value)">
                                <span class="brightness-value">${output.brightness}%</span>
                            </div>
                        </div>
                    `;
                    grid.appendChild(card);
                });
            } catch (error) {
                console.error('Error loading outputs:', error);
            }
        }

        // Toggle output
        async function toggleOutput(pin) {
            try {
                const toggle = document.querySelector(`.toggle-switch[data-pin="${pin}"]`);
                const isActive = toggle.classList.contains('active');
                const brightness = document.querySelector(`.brightness-slider[data-pin="${pin}"]`).value;
                
                const response = await fetch('/api/control', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        pin: pin,
                        active: !isActive,
                        brightness: parseInt(brightness)
                    })
                });
                
                if (response.ok) {
                    await loadOutputs();
                }
            } catch (error) {
                console.error('Error toggling output:', error);
            }
        }

        // Set brightness
        async function setBrightness(pin, brightness) {
            try {
                const toggle = document.querySelector(`.toggle-switch[data-pin="${pin}"]`);
                const isActive = toggle.classList.contains('active');
                
                const response = await fetch('/api/control', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        pin: pin,
                        active: isActive,
                        brightness: parseInt(brightness)
                    })
                });
                
                if (response.ok) {
                    await loadOutputs();
                }
            } catch (error) {
                console.error('Error setting brightness:', error);
            }
        }

        // All On
        document.getElementById('allOn').addEventListener('click', async () => {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                for (const output of data.outputs) {
                    await fetch('/api/control', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({
                            pin: output.pin,
                            active: true,
                            brightness: 100
                        })
                    });
                }
                await loadOutputs();
            } catch (error) {
                console.error('Error turning all on:', error);
            }
        });

        // All Off
        document.getElementById('allOff').addEventListener('click', async () => {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                for (const output of data.outputs) {
                    await fetch('/api/control', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({
                            pin: output.pin,
                            active: false,
                            brightness: 0
                        })
                    });
                }
                await loadOutputs();
            } catch (error) {
                console.error('Error turning all off:', error);
            }
        });

        // Master Brightness Control
        const masterBrightnessSlider = document.getElementById('masterBrightness');
        const masterBrightnessValue = document.getElementById('masterBrightnessValue');
        
        masterBrightnessSlider.addEventListener('input', function() {
            masterBrightnessValue.textContent = this.value + '%';
        });
        
        masterBrightnessSlider.addEventListener('change', async function() {
            const brightness = parseInt(this.value);
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                // Update all active outputs with new brightness
                for (const output of data.outputs) {
                    if (output.active) {
                        await fetch('/api/control', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({
                                pin: output.pin,
                                active: true,
                                brightness: brightness
                            })
                        });
                    }
                }
                await loadOutputs();
            } catch (error) {
                console.error('Error setting master brightness:', error);
            }
        });

        // Output name editing functions
        function editOutputName(pin, currentName, index) {
            document.getElementById(`name-display-${pin}`).style.display = 'none';
            document.getElementById(`name-edit-${pin}`).style.display = 'block';
            const inputField = document.getElementById(`name-input-${pin}`);
            inputField.focus();
            inputField.select();
        }

        function cancelEditName(pin) {
            document.getElementById(`name-display-${pin}`).style.display = 'block';
            document.getElementById(`name-edit-${pin}`).style.display = 'none';
        }

        async function saveOutputName(pin) {
            const newName = document.getElementById(`name-input-${pin}`).value.trim();
            try {
                const response = await fetch('/api/name', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        pin: pin,
                        name: newName
                    })
                });
                
                if (response.ok) {
                    await loadOutputs();
                } else {
                    alert('Failed to save name');
                    cancelEditName(pin);
                }
            } catch (error) {
                console.error('Error saving name:', error);
                alert('Error saving name');
                cancelEditName(pin);
            }
        }

        // Refresh buttons
        document.getElementById('refreshStatus').addEventListener('click', loadStatus);
        document.getElementById('refreshOutputs').addEventListener('click', loadOutputs);

        // Initial load
        loadStatus();
        if (savedTab === 'outputs') {
            loadOutputs();
        }

        // Auto-refresh every 0.5 seconds
        setInterval(() => {
            loadStatus();
            if (document.getElementById('outputsContent').classList.contains('active')) {
                loadOutputs();
            }
        }, 500);
    </script>
</body>
</html>
)rawliteral";
        
        request->send(200, "text/html", html);
    });
    
    // API endpoint for status
    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        unsigned long startTime = millis();
        IPAddress clientIP = request->client()->remoteIP();
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
        
        request->send(200, "application/json", response);
    });
    
    // API endpoint for updating output name
    server->on("/api/name", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        unsigned long startTime = millis();
        IPAddress clientIP = request->client()->remoteIP();
        Serial.print("[WEB] POST /api/name from ");
        Serial.print(clientIP.toString());
        Serial.print(" (");
        Serial.print(len);
        Serial.println(" bytes)");
        
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, (const char*)data);
        
        if (error) {
            Serial.print("[ERROR] JSON deserialization failed: ");
            Serial.println(error.c_str());
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
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
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            Serial.print("[ERROR] GPIO pin not found: ");
            Serial.println(pin);
            request->send(404, "application/json", "{\"error\":\"Output not found\"}");
        }
    });
    
    // API endpoint for control
    server->on("/api/control", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        unsigned long startTime = millis();
        IPAddress clientIP = request->client()->remoteIP();
        Serial.print("[WEB] POST /api/control from ");
        Serial.print(clientIP.toString());
        Serial.print(" (");
        Serial.print(len);
        Serial.println(" bytes)");
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, (const char*)data);
        
        if (error) {
            Serial.print("[ERROR] JSON deserialization failed: ");
            Serial.println(error.c_str());
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
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
        
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    
    // API endpoint to reset saved states
    server->on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        IPAddress clientIP = request->client()->remoteIP();
        Serial.print("[WEB] POST /api/reset from ");
        Serial.println(clientIP.toString());
        Serial.println("[NVRAM] Resetting all saved states...");
        Serial.print("[NVRAM] Free heap before reset: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        
        if (!preferences.begin("railhub32", false)) {
            Serial.println("[ERROR] Failed to open preferences for reset");
            request->send(500, "application/json", "{\"error\":\"Reset failed\"}");
            return;
        }
        
        preferences.clear(); // Clear all saved preferences
        preferences.end();
        
        Serial.println("[NVRAM] All saved states cleared!");
        Serial.print("[NVRAM] Free heap after reset: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        
        request->send(200, "application/json", "{\"status\":\"reset_complete\"}");
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