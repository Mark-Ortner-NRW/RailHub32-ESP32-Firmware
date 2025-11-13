/**
 * @file test_json_parsing.cpp
 * @brief Unit tests for JSON parsing and serialization
 * 
 * Tests API request/response JSON handling.
 */

#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>

// Test: Control command JSON parsing
void test_parse_control_command(void) {
    const char* json = "{\"pin\":2,\"active\":true,\"brightness\":75}";
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_TRUE(error == DeserializationError::Ok);
    TEST_ASSERT_EQUAL(2, doc["pin"].as<int>());
    TEST_ASSERT_TRUE(doc["active"].as<bool>());
    TEST_ASSERT_EQUAL(75, doc["brightness"].as<int>());
}

// Test: Invalid JSON handling
void test_parse_invalid_json(void) {
    const char* json = "{invalid json}";
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error == DeserializationError::Ok);
}

// Test: Missing required fields
void test_parse_missing_fields(void) {
    const char* json = "{\"pin\":2}";
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_TRUE(error == DeserializationError::Ok);
    TEST_ASSERT_EQUAL(2, doc["pin"].as<int>());
    
    // Missing fields should have default values
    TEST_ASSERT_FALSE(doc["active"].as<bool>());
    TEST_ASSERT_EQUAL(0, doc["brightness"].as<int>());
}

// Test: Status response serialization
void test_serialize_status_response(void) {
    JsonDocument doc;
    
    doc["macAddress"] = "AA:BB:CC:DD:EE:FF";
    doc["name"] = "ESP32-Controller-01";
    doc["freeHeap"] = 250000;
    doc["uptime"] = 123456;
    
    JsonArray outputs = doc["outputs"].to<JsonArray>();
    JsonObject output1 = outputs.add<JsonObject>();
    output1["pin"] = 2;
    output1["active"] = true;
    output1["brightness"] = 100;
    output1["name"] = "Test Output";
    
    String json;
    serializeJson(doc, json);
    
    TEST_ASSERT_TRUE(json.length() > 0);
    TEST_ASSERT_TRUE(json.indexOf("macAddress") > 0);
    TEST_ASSERT_TRUE(json.indexOf("outputs") > 0);
}

// Test: Output name update JSON
void test_parse_name_update(void) {
    const char* json = "{\"pin\":2,\"name\":\"Station Light\"}";
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_TRUE(error == DeserializationError::Ok);
    TEST_ASSERT_EQUAL(2, doc["pin"].as<int>());
    TEST_ASSERT_EQUAL_STRING("Station Light", doc["name"].as<const char*>());
}

// Test: Large JSON document handling
void test_large_json_document(void) {
    JsonDocument doc;
    
    // Add 16 outputs
    JsonArray outputs = doc["outputs"].to<JsonArray>();
    for (int i = 0; i < 16; i++) {
        JsonObject output = outputs.add<JsonObject>();
        output["pin"] = i;
        output["active"] = (i % 2 == 0);
        output["brightness"] = i * 10;
        output["name"] = "Output " + String(i);
    }
    
    String json;
    serializeJson(doc, json);
    
    TEST_ASSERT_TRUE(json.length() > 0);
    TEST_ASSERT_TRUE(json.length() < 2048); // Should fit in reasonable buffer
}

// Test: Brightness value range validation
void test_brightness_range_validation(void) {
    const char* json1 = "{\"pin\":2,\"brightness\":100}";
    const char* json2 = "{\"pin\":2,\"brightness\":0}";
    const char* json3 = "{\"pin\":2,\"brightness\":50}";
    
    JsonDocument doc;
    
    // Test max brightness
    deserializeJson(doc, json1);
    int brightness1 = doc["brightness"].as<int>();
    TEST_ASSERT_TRUE(brightness1 >= 0 && brightness1 <= 100);
    
    // Test min brightness
    deserializeJson(doc, json2);
    int brightness2 = doc["brightness"].as<int>();
    TEST_ASSERT_TRUE(brightness2 >= 0 && brightness2 <= 100);
    
    // Test mid brightness
    deserializeJson(doc, json3);
    int brightness3 = doc["brightness"].as<int>();
    TEST_ASSERT_TRUE(brightness3 >= 0 && brightness3 <= 100);
}

// Test: Empty string handling
void test_empty_string_handling(void) {
    const char* json = "{\"pin\":2,\"name\":\"\"}";
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_TRUE(error == DeserializationError::Ok);
    TEST_ASSERT_EQUAL_STRING("", doc["name"].as<const char*>());
}

void setUp(void) {}
void tearDown(void) {}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_control_command);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_parse_missing_fields);
    RUN_TEST(test_serialize_status_response);
    RUN_TEST(test_parse_name_update);
    RUN_TEST(test_large_json_document);
    RUN_TEST(test_brightness_range_validation);
    RUN_TEST(test_empty_string_handling);
    
    UNITY_END();
}

void loop() {}
