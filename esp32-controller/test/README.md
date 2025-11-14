# RailHub32 Controller - Unit Tests

## Overview

This directory contains comprehensive unit tests for the RailHub32 Controller firmware. The tests are organized by functional area and use the Unity testing framework integrated with PlatformIO.

## Test Structure

```
test/
├── README.md                       # This file
├── test_main.cpp                   # Main test runner
├── test_gpio/
│   └── test_gpio_control.cpp      # GPIO and PWM control tests
├── test_json/
│   └── test_json_parsing.cpp      # JSON API serialization tests
├── test_config/
│   └── test_configuration.cpp     # Configuration validation tests
└── test_utils/
    └── test_helpers.cpp           # Utility function tests
```

## Test Categories

### 1. GPIO Control Tests (`test_gpio/`)

Tests for hardware GPIO and PWM functionality:
- ✅ GPIO pin validation and safety checks
- ✅ Reserved pin exclusion (flash pins 6-11)
- ✅ Pin uniqueness verification
- ✅ Status LED configuration
- ✅ PWM channel assignment

**File**: `test_gpio_control.cpp`  
**Tests**: 5

### 2. JSON Parsing Tests (`test_json/`)

Tests for API request/response handling:
- ✅ Control command parsing
- ✅ Invalid JSON error handling
- ✅ Missing field defaults
- ✅ Status response serialization
- ✅ Output name updates
- ✅ Large document handling
- ✅ Brightness range validation
- ✅ Empty string handling

**File**: `test_json_parsing.cpp`  
**Tests**: 8

### 3. Configuration Tests (`test_config/`)

Tests for system configuration validation:
- ✅ WiFi AP settings
- ✅ Password length requirements
- ✅ IP address configuration
- ✅ WiFiManager settings
- ✅ Device name validation
- ✅ Output count limits
- ✅ WiFi channel validity
- ✅ Portal trigger settings
- ✅ Connection limits

**File**: `test_configuration.cpp`  
**Tests**: 11

### 4. Utility Tests (`test_utils/`)

Tests for helper functions and utilities:
- ✅ Brightness to PWM conversion
- ✅ PWM to brightness conversion
- ✅ Hostname sanitization
- ✅ Value constraining
- ✅ Pin index mapping
- ✅ Uptime calculations
- ✅ Memory formatting
- ✅ Output name validation
- ✅ WiFi signal quality

**File**: `test_helpers.cpp`  
**Tests**: 9

## Running Tests

### On-Device Testing (ESP32)

Run tests on actual ESP32 hardware:

```bash
# Run all tests on ESP32
pio test -e esp32dev_test

# Run specific test
pio test -e esp32dev_test -f test_gpio_control

# Run with verbose output
pio test -e esp32dev_test -v
```

### Native Testing (Local Machine)

Run tests on your development machine (faster, no hardware needed):

```bash
# Run all tests natively
pio test -e native

# Run specific test file
pio test -e native -f test_json_parsing
```

### Continuous Integration

Tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Run Unit Tests
  run: |
    pio test -e native
    pio test -e esp32dev_test
```

## Test Results Interpretation

### Success Output
```
test/test_gpio_control.cpp:XX:test_gpio_pin_validation [PASSED]
test/test_gpio_control.cpp:XX:test_gpio_reserved_pins [PASSED]
...
-----------------------
5 Tests 0 Failures 0 Ignored
OK
```

### Failure Output
```
test/test_gpio_control.cpp:XX:test_gpio_pin_validation [FAILED]
Expected 16 Was 15
...
-----------------------
5 Tests 1 Failures 0 Ignored
FAIL
```

## Test Coverage

| Component | Coverage | Tests |
|-----------|----------|-------|
| **GPIO Control** | ✅ High | 5 tests |
| **JSON API** | ✅ High | 8 tests |
| **Configuration** | ✅ Complete | 11 tests |
| **Utilities** | ✅ High | 9 tests |
| **Total** | - | **33 tests** |

## Adding New Tests

### 1. Create Test File

```cpp
#include <unity.h>

void test_new_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}

void setUp(void) {}
void tearDown(void) {}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_new_feature);
    UNITY_END();
}

void loop() {}
```

### 2. Test Naming Convention

- File: `test_<category>/<test_module_name>.cpp`
- Function: `test_<specific_functionality>(void)`
- Use descriptive names that explain what is being tested

### 3. Common Assertions

```cpp
// Equality
TEST_ASSERT_EQUAL(expected, actual);
TEST_ASSERT_EQUAL_STRING("expected", actual);

// Boolean
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);

// Comparison
TEST_ASSERT_GREATER_THAN(threshold, actual);
TEST_ASSERT_LESS_THAN(threshold, actual);

// Floating point
TEST_ASSERT_EQUAL_FLOAT(expected, actual);
TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual);

// Null checks
TEST_ASSERT_NULL(pointer);
TEST_ASSERT_NOT_NULL(pointer);
```

## Best Practices

### ✅ DO:
- Write focused tests that verify one thing
- Use descriptive test names
- Test both success and failure cases
- Clean up resources in `tearDown()`
- Mock external dependencies for unit tests
- Keep tests independent of each other
- Test edge cases and boundary conditions

### ❌ DON'T:
- Write tests that depend on specific timing
- Create tests with external dependencies (WiFi, etc.)
- Test multiple unrelated things in one test
- Leave hardcoded values without explanation
- Skip error case testing
- Write tests that modify global state

## Debugging Failed Tests

### 1. Verbose Output
```bash
pio test -e esp32dev_test -v
```

### 2. Serial Monitor
```bash
pio device monitor -e esp32dev_test
```

### 3. Individual Test Execution
Comment out other `RUN_TEST()` calls to isolate:
```cpp
UNITY_BEGIN();
// RUN_TEST(test_other_feature);
RUN_TEST(test_failing_feature);  // Only run this one
UNITY_END();
```

### 4. Add Debug Output
```cpp
void test_feature(void) {
    Serial.println("Debug: Testing feature X");
    Serial.printf("Value: %d\n", testValue);
    TEST_ASSERT_EQUAL(expected, actual);
}
```

## Performance Benchmarks

| Environment | Test Suite Runtime | Tests/Second |
|-------------|-------------------|--------------|
| **Native** | ~0.5s | 66 tests/sec |
| **ESP32** | ~3-5s | 6-11 tests/sec |

## Test Maintenance

### Regular Updates
- ✅ Run tests before committing code
- ✅ Update tests when modifying features
- ✅ Add tests for new features
- ✅ Review test coverage monthly

### Regression Testing
All tests must pass before merging to main branch.

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: Unit Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.x'
      - name: Install PlatformIO
        run: pip install platformio
      - name: Run Tests
        run: pio test -e native
```

## Resources

- [Unity Test Framework Documentation](https://github.com/ThrowTheSwitch/Unity)
- [PlatformIO Unit Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/)
- [ESP32 Testing Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)

## License

Same as main project (MIT License)

---

**Total Test Count**: 33 tests  
**Last Updated**: November 13, 2025  
**Maintained by**: RailHub32 Development Team
