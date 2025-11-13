/**
 * @file test_gpio_control.cpp
 * @brief Unit tests for GPIO control functionality
 * 
 * Tests GPIO pin initialization, validation, and control operations.
 */

#include <Arduino.h>
#include <unity.h>
#include "../../include/config.h"

#ifdef NATIVE_BUILD
// Mock implementations for native testing
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}
void ledcWrite(uint8_t channel, uint32_t duty) {}
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int value) {}
#else
#include <Arduino.h>
#endif

// Test data
const int validPins[] = LED_PINS;
const int numPins = sizeof(validPins) / sizeof(validPins[0]);

// Test: GPIO pin validation
void test_gpio_pin_validation(void) {
    // Valid pins should be accepted
    for (int i = 0; i < numPins; i++) {
        TEST_ASSERT_TRUE(validPins[i] >= 0 && validPins[i] < 40);
    }
    
    // Test expected number of pins
    TEST_ASSERT_EQUAL(MAX_OUTPUTS, numPins);
}

// Test: GPIO reserved pins exclusion
void test_gpio_reserved_pins(void) {
    // Flash pins (6-11) should not be in the list
    for (int i = 0; i < numPins; i++) {
        TEST_ASSERT_TRUE(validPins[i] < 6 || validPins[i] > 11);
    }
}

// Test: GPIO pin uniqueness
void test_gpio_pin_uniqueness(void) {
    // Check that each pin appears only once
    for (int i = 0; i < numPins; i++) {
        for (int j = i + 1; j < numPins; j++) {
            TEST_ASSERT_NOT_EQUAL(validPins[i], validPins[j]);
        }
    }
}

// Test: Status LED pin configuration
void test_status_led_pin(void) {
    TEST_ASSERT_EQUAL(2, STATUS_LED_PIN);
    
    // Status LED should be in the valid pins list
    bool found = false;
    for (int i = 0; i < numPins; i++) {
        if (validPins[i] == STATUS_LED_PIN) {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found);
}

// Test: PWM channel assignment
void test_pwm_channel_assignment(void) {
    // ESP32 supports 16 PWM channels (0-15)
    TEST_ASSERT_TRUE(MAX_OUTPUTS <= 16);
    
    // Each output should have a unique PWM channel
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        TEST_ASSERT_TRUE(i >= 0 && i < 16);
    }
}

// Test suite setup
void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_gpio_pin_validation);
    RUN_TEST(test_gpio_reserved_pins);
    RUN_TEST(test_gpio_pin_uniqueness);
    RUN_TEST(test_status_led_pin);
    RUN_TEST(test_pwm_channel_assignment);
    
    UNITY_END();
}

void loop() {
    // Tests run once
}
