#ifndef CONFIG_H
#define CONFIG_H

// WiFi Access Point Configuration
#define AP_SSID "RailHub32-AP"           // Access Point name
#define AP_PASSWORD "RailHub32Pass"      // Access Point password (min 8 characters)
#define AP_CHANNEL 6                     // WiFi channel (1-13)
#define AP_HIDDEN false                  // Set to true to hide SSID
#define AP_MAX_CONNECTIONS 4             // Maximum number of simultaneous connections

// Access Point IP Configuration
#define AP_LOCAL_IP "192.168.4.1"        // ESP32 IP address
#define AP_GATEWAY "192.168.4.1"         // Gateway IP
#define AP_SUBNET "255.255.255.0"        // Subnet mask

// MQTT Configuration
#define MQTT_BROKER "192.168.4.100"      // IP of your MQTT broker (adjust if broker is on AP network)
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "RailHub32-ESP32-"

// Device Configuration
#define DEVICE_NAME "ESP32-Controller-01"
#define MAX_OUTPUTS 16

// Pin Definitions for different output types
#define LED_PINS {2, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 12, 13, 14}

// Status LED
#define STATUS_LED_PIN 2

// Update intervals
#define STATUS_UPDATE_INTERVAL 30000  // 30 seconds
#define MQTT_RECONNECT_INTERVAL 5000  // 5 seconds

#endif