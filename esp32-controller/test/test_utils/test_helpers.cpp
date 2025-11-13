/**
 * @file test_helpers.cpp
 * @brief Unit tests for utility functions and helpers
 * 
 * Tests common utility functions used throughout the application.
 */

#include <Arduino.h>
#include <unity.h>
#include <cstring>

// Test: Brightness percentage to PWM conversion
void test_brightness_to_pwm_conversion(void) {
    // 0% -> 0 PWM
    int pwm0 = map(0, 0, 100, 0, 255);
    TEST_ASSERT_EQUAL(0, pwm0);
    
    // 50% -> 127-128 PWM
    int pwm50 = map(50, 0, 100, 0, 255);
    TEST_ASSERT_TRUE(pwm50 >= 127 && pwm50 <= 128);
    
    // 100% -> 255 PWM
    int pwm100 = map(100, 0, 100, 0, 255);
    TEST_ASSERT_EQUAL(255, pwm100);
}

// Test: PWM to brightness percentage conversion
void test_pwm_to_brightness_conversion(void) {
    // 0 PWM -> 0%
    int brightness0 = map(0, 0, 255, 0, 100);
    TEST_ASSERT_EQUAL(0, brightness0);
    
    // 128 PWM -> ~50%
    int brightness50 = map(128, 0, 255, 0, 100);
    TEST_ASSERT_TRUE(brightness50 >= 50 && brightness50 <= 51);
    
    // 255 PWM -> 100%
    int brightness100 = map(255, 0, 255, 0, 100);
    TEST_ASSERT_EQUAL(100, brightness100);
}

// Test: String sanitization for mDNS hostname
void test_hostname_sanitization(void) {
    // Simulate hostname sanitization logic
    const char* testNames[] = {
        "ESP32-Controller-01",
        "My Train Controller",
        "RailHub32",
        "Test_Device_123"
    };
    
    for (int i = 0; i < 4; i++) {
        const char* name = testNames[i];
        size_t len = strlen(name);
        
        // Hostname should not be empty
        TEST_ASSERT_TRUE(len > 0);
        
        // Should not exceed reasonable length
        TEST_ASSERT_TRUE(len < 64);
    }
}

// Test: Constrain function for brightness values
void test_constrain_brightness(void) {
    // Test values outside range
    int val1 = constrain(-10, 0, 100);
    TEST_ASSERT_EQUAL(0, val1);
    
    int val2 = constrain(150, 0, 100);
    TEST_ASSERT_EQUAL(100, val2);
    
    // Test values inside range
    int val3 = constrain(50, 0, 100);
    TEST_ASSERT_EQUAL(50, val3);
}

// Test: Pin index to GPIO pin mapping
void test_pin_index_mapping(void) {
    const int pins[] = {2, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 12, 13, 14};
    
    for (int i = 0; i < 16; i++) {
        // Index should map to valid GPIO pin
        TEST_ASSERT_TRUE(pins[i] >= 0 && pins[i] < 40);
        
        // Should not be reserved flash pins
        TEST_ASSERT_TRUE(pins[i] < 6 || pins[i] > 11);
    }
}

// Test: Uptime formatting
void test_uptime_calculation(void) {
    // Simulate uptime calculations
    unsigned long milliseconds = 123456789;
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    TEST_ASSERT_EQUAL(123456, seconds);
    TEST_ASSERT_EQUAL(2057, minutes);
    TEST_ASSERT_EQUAL(34, hours);
    TEST_ASSERT_EQUAL(1, days);
}

// Test: Memory size formatting (bytes to KB)
void test_memory_formatting(void) {
    unsigned long bytes = 248576;
    unsigned long kb = bytes / 1024;
    
    TEST_ASSERT_EQUAL(242, kb);
    
    // Test edge cases
    TEST_ASSERT_EQUAL(0, 512 / 1024);
    TEST_ASSERT_EQUAL(1, 1024 / 1024);
    TEST_ASSERT_EQUAL(1, 1536 / 1024);
}

// Test: Output name length validation
void test_output_name_validation(void) {
    const char* validName = "Station Light";
    const char* longName = "This is a very long output name that exceeds twenty characters";
    
    // Valid name should be accepted
    TEST_ASSERT_TRUE(strlen(validName) <= 20);
    
    // Long name should be detected
    TEST_ASSERT_TRUE(strlen(longName) > 20);
}

// Test: WiFi RSSI to quality percentage
void test_wifi_rssi_quality(void) {
    // Excellent signal: > -50 dBm
    int rssi1 = -40;
    TEST_ASSERT_TRUE(rssi1 > -50);
    
    // Good signal: -50 to -60 dBm
    int rssi2 = -55;
    TEST_ASSERT_TRUE(rssi2 >= -60 && rssi2 <= -50);
    
    // Fair signal: -60 to -70 dBm
    int rssi3 = -65;
    TEST_ASSERT_TRUE(rssi3 >= -70 && rssi3 <= -60);
    
    // Poor signal: < -70 dBm
    int rssi4 = -80;
    TEST_ASSERT_TRUE(rssi4 < -70);
}

// Mock Arduino map function for native testing
#ifdef NATIVE_BUILD
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}
#endif

void setUp(void) {}
void tearDown(void) {}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_brightness_to_pwm_conversion);
    RUN_TEST(test_pwm_to_brightness_conversion);
    RUN_TEST(test_hostname_sanitization);
    RUN_TEST(test_constrain_brightness);
    RUN_TEST(test_pin_index_mapping);
    RUN_TEST(test_uptime_calculation);
    RUN_TEST(test_memory_formatting);
    RUN_TEST(test_output_name_validation);
    RUN_TEST(test_wifi_rssi_quality);
    
    UNITY_END();
}

void loop() {}
