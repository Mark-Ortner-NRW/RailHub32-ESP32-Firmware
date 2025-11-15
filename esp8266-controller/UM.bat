@echo off
REM Quick Upload and Monitor script for ESP8266
REM Just type "UM" to upload and monitor
echo.
echo ====================================
echo    ESP8266 Upload and Monitor
echo ====================================
echo.
platformio run --target upload --target monitor
