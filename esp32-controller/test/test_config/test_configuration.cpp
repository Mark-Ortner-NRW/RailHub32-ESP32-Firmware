/**
 * @file test_configuration.cpp
 * @brief Unit tests for configuration management
 * 
 * Tests configuration constants, WiFi settings, and device parameters.
 */

#include <Arduino.h>
#include <unity.h>
#include <cstring>
#include "../../include/config.h"

// Test: WiFi AP configuration
void test_wifi_ap_configuration(void) {
    TEST_ASSERT_EQUAL_STRING("RailHub32-AP", AP_SSID);
    TEST_ASSERT_EQUAL_STRING("RailHub32Pass", AP_PASSWORD);
    TEST_ASSERT_EQUAL(6, AP_CHANNEL);
    TEST_ASSERT_FALSE(AP_HIDDEN);
    TEST_ASSERT_EQUAL(4, AP_MAX_CONNECTIONS);
}

// Test: WiFi AP password length
void test_wifi_ap_password_length(void) {
    // WPA2 requires minimum 8 characters
    size_t passwordLen = strlen(AP_PASSWORD);
    TEST_ASSERT_TRUE(passwordLen >= 8);
    TEST_ASSERT_TRUE(passwordLen <= 63); // WPA2 maximum
}

// Test: IP address configuration
void test_ip_configuration(void) {
    TEST_ASSERT_EQUAL_STRING("192.168.4.1", AP_LOCAL_IP);
    TEST_ASSERT_EQUAL_STRING("192.168.4.1", AP_GATEWAY);
    TEST_ASSERT_EQUAL_STRING("255.255.255.0", AP_SUBNET);
}

// Test: WiFiManager configuration
void test_wifimanager_configuration(void) {
    TEST_ASSERT_EQUAL_STRING("RailHub32-Setup", WIFIMANAGER_AP_SSID);
    TEST_ASSERT_EQUAL_STRING("12345678", WIFIMANAGER_AP_PASSWORD);
    TEST_ASSERT_EQUAL(180, WIFIMANAGER_TIMEOUT);
    TEST_ASSERT_EQUAL(0, PORTAL_TRIGGER_PIN);
    TEST_ASSERT_EQUAL(3000, PORTAL_TRIGGER_DURATION);
}

// Test: WiFiManager password length
void test_wifimanager_password_length(void) {
    size_t passwordLen = strlen(WIFIMANAGER_AP_PASSWORD);
    TEST_ASSERT_TRUE(passwordLen >= 8);
}

// Test: Device name configuration
void test_device_name(void) {
    TEST_ASSERT_EQUAL_STRING("ESP32-Controller-01", DEVICE_NAME);
    
    // Device name should not be empty
    TEST_ASSERT_TRUE(strlen(DEVICE_NAME) > 0);
    TEST_ASSERT_TRUE(strlen(DEVICE_NAME) < 40); // Reasonable length limit
}

// Test: Maximum outputs configuration
void test_max_outputs_configuration(void) {
    TEST_ASSERT_EQUAL(16, MAX_OUTPUTS);
    
    // Should not exceed ESP32 PWM channel limit
    TEST_ASSERT_TRUE(MAX_OUTPUTS <= 16);
}

// Test: WiFi channel validity
void test_wifi_channel_validity(void) {
    // WiFi channels 1-13 are valid for most regions
    TEST_ASSERT_TRUE(AP_CHANNEL >= 1 && AP_CHANNEL <= 13);
}

// Test: Portal trigger pin validity
void test_portal_trigger_pin_validity(void) {
    // GPIO 0 is the boot button on most ESP32 boards
    TEST_ASSERT_EQUAL(0, PORTAL_TRIGGER_PIN);
    
    // Should be a valid GPIO pin
    TEST_ASSERT_TRUE(PORTAL_TRIGGER_PIN >= 0 && PORTAL_TRIGGER_PIN < 40);
}

// Test: Portal trigger duration
void test_portal_trigger_duration(void) {
    // Should be reasonable (not too short, not too long)
    TEST_ASSERT_TRUE(PORTAL_TRIGGER_DURATION >= 1000); // At least 1 second
    TEST_ASSERT_TRUE(PORTAL_TRIGGER_DURATION <= 10000); // Max 10 seconds
}

// Test: Max connections limit
void test_max_connections_limit(void) {
    // ESP32 supports up to 4 simultaneous SoftAP connections
    TEST_ASSERT_TRUE(AP_MAX_CONNECTIONS > 0);
    TEST_ASSERT_TRUE(AP_MAX_CONNECTIONS <= 4);
}

void setUp(void) {}
void tearDown(void) {}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_wifi_ap_configuration);
    RUN_TEST(test_wifi_ap_password_length);
    RUN_TEST(test_ip_configuration);
    RUN_TEST(test_wifimanager_configuration);
    RUN_TEST(test_wifimanager_password_length);
    RUN_TEST(test_device_name);
    RUN_TEST(test_max_outputs_configuration);
    RUN_TEST(test_wifi_channel_validity);
    RUN_TEST(test_portal_trigger_pin_validity);
    RUN_TEST(test_portal_trigger_duration);
    RUN_TEST(test_max_connections_limit);
    
    UNITY_END();
}

void loop() {}
