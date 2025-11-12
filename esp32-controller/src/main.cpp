#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "config.h"

// Forward declarations
void initializeOutputs();
void initializeWiFi();
void initializeMQTT();
void reconnectMQTT();
void initializeWebServer();
void onMqttMessage(char* topic, byte* payload, unsigned int length);
void executeOutputCommand(int pin, bool active, int brightnessPercent);
void sendStatusUpdate();
void sendDiscoveryMessage();
void saveOutputState(int index);
void loadOutputStates();
void saveAllOutputStates();

// Global variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);
// Web Server
AsyncWebServer server(80);
Preferences preferences;

String macAddress;
String deviceId;

// Output pin configuration
int outputPins[MAX_OUTPUTS] = LED_PINS;
bool outputStates[MAX_OUTPUTS] = {false};
int outputBrightness[MAX_OUTPUTS] = {255}; // 0-255 for PWM

// Timing variables
unsigned long lastStatusUpdate = 0;
unsigned long lastMqttReconnect = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("RailHub32 ESP32 Controller Starting...");
    
    // Get MAC address for unique identification
    macAddress = WiFi.macAddress();
    deviceId = MQTT_CLIENT_ID_PREFIX + macAddress;
    deviceId.replace(":", "");
    
    Serial.println("Device ID: " + deviceId);
    Serial.println("MAC Address: " + macAddress);
    
    // Initialize output pins
    initializeOutputs();
    
    // Load saved output states from NVRAM
    loadOutputStates();
    
    // Initialize WiFi
    initializeWiFi();
    
    // Initialize MQTT
    initializeMQTT();
    
    // Initialize web server
    initializeWebServer();
    
    Serial.println("Setup complete!");
}

void loop() {
    // Handle MQTT connection
    if (!mqttClient.connected()) {
        if (millis() - lastMqttReconnect > MQTT_RECONNECT_INTERVAL) {
            reconnectMQTT();
            lastMqttReconnect = millis();
        }
    }
    mqttClient.loop();
    
    // Send periodic status updates
    if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
        sendStatusUpdate();
        lastStatusUpdate = millis();
    }
    
    // Handle any other tasks
    yield();
}

void initializeOutputs() {
    Serial.println("Initializing outputs...");
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        pinMode(outputPins[i], OUTPUT);
        digitalWrite(outputPins[i], LOW);
        
        // Configure PWM channel for brightness control
        ledcSetup(i, 5000, 8); // 5kHz frequency, 8-bit resolution
        ledcAttachPin(outputPins[i], i);
        ledcWrite(i, 0);
    }
    
    // Status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH); // Turn on status LED
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

void initializeMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(onMqttMessage);
    reconnectMQTT();
}

void reconnectMQTT() {
    // Check if Access Point is running
    if (WiFi.softAPgetStationNum() == 0 && !mqttClient.connected()) {
        // No clients connected, optionally skip MQTT connection
        // return;
    }
    
    if (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        
        if (mqttClient.connect(deviceId.c_str())) {
            Serial.println(" connected!");
            
            // Subscribe to command topic
            String commandTopic = "railhub32/device/" + macAddress + "/command";
            mqttClient.subscribe(commandTopic.c_str());
            Serial.println("Subscribed to: " + commandTopic);
            
            // Send discovery message
            sendDiscoveryMessage();
            
        } else {
            Serial.print(" failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" will try again later");
        }
    }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.println("Received MQTT message: " + String(topic) + " - " + message);
    
    // Parse JSON command
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("Failed to parse JSON command");
        return;
    }
    
    // Extract command parameters
    int pin = doc["pin"];
    bool active = doc["active"];
    int brightness = doc["brightness"] | 100; // Default to 100%
    
    // Execute command
    executeOutputCommand(pin, active, brightness);
}

void executeOutputCommand(int pin, bool active, int brightnessPercent) {
    // Find the output index for the given pin
    int outputIndex = -1;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (outputPins[i] == pin) {
            outputIndex = i;
            break;
        }
    }
    
    if (outputIndex == -1) {
        Serial.println("Invalid pin: " + String(pin));
        return;
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
    
    Serial.println("Output " + String(pin) + " set to " + (active ? "ON" : "OFF") + 
                  " with brightness " + String(brightnessPercent) + "% [SAVED]");
}

void sendStatusUpdate() {
    if (!mqttClient.connected()) {
        return;
    }
    
    DynamicJsonDocument doc(2048);
    doc["status"] = "online";
    doc["ip"] = WiFi.softAPIP().toString();
    doc["apClients"] = WiFi.softAPgetStationNum();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    
    JsonArray outputs = doc.createNestedArray("outputs");
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        JsonObject output = outputs.createNestedObject();
        output["pin"] = outputPins[i];
        output["active"] = outputStates[i];
        output["brightness"] = map(outputBrightness[i], 0, 255, 0, 100);
    }
    
    String statusJson;
    serializeJson(doc, statusJson);
    
    String statusTopic = "railhub32/device/" + macAddress + "/status";
    mqttClient.publish(statusTopic.c_str(), statusJson.c_str());
    
    Serial.println("Status update sent");
}

void sendDiscoveryMessage() {
    DynamicJsonDocument doc(1024);
    doc["deviceId"] = deviceId;
    doc["macAddress"] = macAddress;
    doc["name"] = DEVICE_NAME;
    doc["type"] = "LightController";
    doc["mode"] = "AccessPoint";
    doc["ip"] = WiFi.softAPIP().toString();
    doc["apSSID"] = AP_SSID;
    
    JsonArray pins = doc.createNestedArray("outputPins");
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        pins.add(outputPins[i]);
    }
    
    String discoveryJson;
    serializeJson(doc, discoveryJson);
    
    String discoveryTopic = "railhub32/device/" + macAddress + "/discovery";
    mqttClient.publish(discoveryTopic.c_str(), discoveryJson.c_str());
    
    Serial.println("Discovery message sent");
}

void saveOutputState(int index) {
    if (index < 0 || index >= MAX_OUTPUTS) {
        return;
    }
    
    preferences.begin("railhub32", false);
    
    // Create keys for state and brightness
    String stateKey = "out_" + String(index) + "_s";
    String brightKey = "out_" + String(index) + "_b";
    
    preferences.putBool(stateKey.c_str(), outputStates[index]);
    preferences.putUChar(brightKey.c_str(), outputBrightness[index]);
    
    preferences.end();
}

void loadOutputStates() {
    Serial.println("Loading saved output states from NVRAM...");
    preferences.begin("railhub32", true); // Read-only mode
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        String stateKey = "out_" + String(i) + "_s";
        String brightKey = "out_" + String(i) + "_b";
        
        // Load state (default to false if not found)
        outputStates[i] = preferences.getBool(stateKey.c_str(), false);
        
        // Load brightness (default to 255 if not found)
        outputBrightness[i] = preferences.getUChar(brightKey.c_str(), 255);
        
        // Apply the loaded state to the output
        if (outputStates[i]) {
            ledcWrite(i, outputBrightness[i]);
            Serial.println("  Output " + String(i) + " (Pin " + String(outputPins[i]) + "): ON, Brightness: " + String(map(outputBrightness[i], 0, 255, 0, 100)) + "%");
        } else {
            ledcWrite(i, 0);
            Serial.println("  Output " + String(i) + " (Pin " + String(outputPins[i]) + "): OFF");
        }
    }
    
    preferences.end();
    Serial.println("Output states loaded successfully!");
}

void saveAllOutputStates() {
    Serial.println("Saving all output states to NVRAM...");
    preferences.begin("railhub32", false);
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        String stateKey = "out_" + String(i) + "_s";
        String brightKey = "out_" + String(i) + "_b";
        
        preferences.putBool(stateKey.c_str(), outputStates[i]);
        preferences.putUChar(brightKey.c_str(), outputBrightness[i]);
    }
    
    preferences.end();
    Serial.println("All output states saved successfully!");
}

void initializeWebServer() {
    // Serve main HTML page with RailHub32 styling
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RailHub32 ESP32 - )rawliteral" + String(DEVICE_NAME) + R"rawliteral(</title>
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
                <p id="deviceName">)rawliteral" + String(DEVICE_NAME) + R"rawliteral(</p>
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
                outputs: { master: 'Master Brightness Control', masterDesc: 'Adjusts brightness for all active outputs simultaneously', individual: 'Individual Output Control', output: 'Output', pin: 'Pin', brightness: 'Brightness', all: 'ALL', on: 'ON', off: 'OFF' }
            },
            de: {
                nav: { status: 'Status', outputs: 'Ausgänge' },
                buttons: { refresh: '🔄 Aktualisieren', allOn: '💡 Alle Ein', allOff: '⚫ Alle Aus' },
                status: { deviceInfo: 'Geräteinformationen', apIp: 'AP IP-Adresse', clients: 'Verbundene Clients', uptime: 'Laufzeit', freeHeap: 'Freier Speicher', macAddr: 'MAC-Adresse', apSsid: 'AP SSID' },
                outputs: { master: 'Master-Helligkeitssteuerung', masterDesc: 'Passt die Helligkeit aller aktiven Ausgänge gleichzeitig an', individual: 'Individuelle Ausgangssteuerung', output: 'Ausgang', pin: 'Pin', brightness: 'Helligkeit', all: 'ALLE', on: 'EIN', off: 'AUS' }
            },
            fr: {
                nav: { status: 'Statut', outputs: 'Sorties' },
                buttons: { refresh: '🔄 Actualiser', allOn: '💡 Tous Allumés', allOff: '⚫ Tous Éteints' },
                status: { deviceInfo: 'Informations sur l\'appareil', apIp: 'Adresse IP AP', clients: 'Clients connectés', uptime: 'Temps de fonctionnement', freeHeap: 'Mémoire libre', macAddr: 'Adresse MAC', apSsid: 'AP SSID' },
                outputs: { master: 'Contrôle principal de la luminosité', masterDesc: 'Ajuste la luminosité de toutes les sorties actives simultanément', individual: 'Contrôle individuel des sorties', output: 'Sortie', pin: 'Broche', brightness: 'Luminosité', all: 'TOUS', on: 'ALLUMÉ', off: 'ÉTEINT' }
            },
            it: {
                nav: { status: 'Stato', outputs: 'Uscite' },
                buttons: { refresh: '🔄 Aggiorna', allOn: '💡 Tutti Accesi', allOff: '⚫ Tutti Spenti' },
                status: { deviceInfo: 'Informazioni dispositivo', apIp: 'Indirizzo IP AP', clients: 'Client connessi', uptime: 'Tempo di attività', freeHeap: 'Memoria libera', macAddr: 'Indirizzo MAC', apSsid: 'AP SSID' },
                outputs: { master: 'Controllo luminosità principale', masterDesc: 'Regola la luminosità di tutte le uscite attive simultaneamente', individual: 'Controllo uscite individuali', output: 'Uscita', pin: 'Pin', brightness: 'Luminosità', all: 'TUTTI', on: 'ACCESO', off: 'SPENTO' }
            },
            zh: {
                nav: { status: '状态', outputs: '输出' },
                buttons: { refresh: '🔄 刷新', allOn: '💡 全部开启', allOff: '⚫ 全部关闭' },
                status: { deviceInfo: '设备信息', apIp: 'AP IP地址', clients: '已连接客户端', uptime: '运行时间', freeHeap: '可用内存', macAddr: 'MAC地址', apSsid: 'AP SSID' },
                outputs: { master: '主亮度控制', masterDesc: '同时调整所有活动输出的亮度', individual: '单独输出控制', output: '输出', pin: '引脚', brightness: '亮度', all: '全部', on: '开启', off: '关闭' }
            },
            hi: {
                nav: { status: 'स्थिति', outputs: 'आउटपुट' },
                buttons: { refresh: '🔄 रिफ्रेश', allOn: '💡 सभी चालू', allOff: '⚫ सभी बंद' },
                status: { deviceInfo: 'डिवाइस जानकारी', apIp: 'AP IP पता', clients: 'कनेक्टेड क्लाइंट', uptime: 'अपटाइम', freeHeap: 'खाली मेमोरी', macAddr: 'MAC पता', apSsid: 'AP SSID' },
                outputs: { master: 'मास्टर चमक नियंत्रण', masterDesc: 'सभी सक्रिय आउटपुट की चमक एक साथ समायोजित करता है', individual: 'व्यक्तिगत आउटपुट नियंत्रण', output: 'आउटपुट', pin: 'पिन', brightness: 'चमक', all: 'सभी', on: 'चालू', off: 'बंद' }
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
                    const card = document.createElement('div');
                    card.className = 'output-card' + (output.active ? ' active' : '');
                    card.innerHTML = `
                        <div class="output-header">
                            <div class="output-name">${t.output} ${index + 1}</div>
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

        // Refresh buttons
        document.getElementById('refreshStatus').addEventListener('click', loadStatus);
        document.getElementById('refreshOutputs').addEventListener('click', loadOutputs);

        // Initial load
        loadStatus();
        if (savedTab === 'outputs') {
            loadOutputs();
        }

        // Auto-refresh every 5 seconds
        setInterval(() => {
            loadStatus();
            if (document.getElementById('outputsContent').classList.contains('active')) {
                loadOutputs();
            }
        }, 5000);
    </script>
</body>
</html>
)rawliteral";
        
        request->send(200, "text/html", html);
    });
    
    // API endpoint for status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(2048);
        doc["deviceId"] = deviceId;
        doc["macAddress"] = macAddress;
        doc["name"] = DEVICE_NAME;
        doc["ip"] = WiFi.softAPIP().toString();
        doc["apClients"] = WiFi.softAPgetStationNum();
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["uptime"] = millis();
        
        JsonArray outputs = doc.createNestedArray("outputs");
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            JsonObject output = outputs.createNestedObject();
            output["pin"] = outputPins[i];
            output["active"] = outputStates[i];
            output["brightness"] = map(outputBrightness[i], 0, 255, 0, 100);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API endpoint for control
    server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, (const char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        int pin = doc["pin"];
        bool active = doc["active"];
        int brightness = doc["brightness"] | 100;
        
        executeOutputCommand(pin, active, brightness);
        
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    
    // API endpoint to reset saved states
    server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println("Resetting all saved states...");
        preferences.begin("railhub32", false);
        preferences.clear(); // Clear all saved preferences
        preferences.end();
        Serial.println("All saved states cleared!");
        request->send(200, "application/json", "{\"status\":\"reset_complete\"}");
    });
    
    server.begin();
    Serial.println("Web server started on port 80");
}