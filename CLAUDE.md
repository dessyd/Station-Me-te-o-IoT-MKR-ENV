# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

IoT weather station using Arduino MKR WiFi 1010 with MKR ENV Shield. Features deep sleep power management, binary MQTT protocol, and automatic device provisioning.

## Build System

**Platform:** Arduino IDE (no CLI build configuration)

**Required Libraries** (install via Arduino Library Manager):

- WiFiNINA
- ArduinoMqttClient
- Arduino_MKRENV
- ArduinoECCX08
- FlashStorage
- ArduinoLowPower

**To compile:** Open `MQTTSensorClient/MQTTSensorClient.ino` in Arduino IDE, select board "Arduino MKR WiFi 1010", then Verify/Upload.

## Architecture

```text
MQTTSensorClient.ino (orchestration)
    ├── ProvisionManager (UDP device ID + MQTT broker discovery)
    ├── NetworkManager (WiFi + MQTT connection)
    ├── SensorManager (ENV Shield sensor readings)
    ├── DataEncoder (binary compact encoding)
    ├── StorageManager (flash persistence across sleep)
    └── Config.h (centralized configuration)
```

**Key patterns:**

- Modular manager classes with single responsibilities
- Deep sleep cycle: wake → read → encode → transmit → sleep (5 min default)
- Differential transmission: only changed sensor values sent
- Binary protocol: 2-10 bytes vs ~200 bytes JSON

## Configuration

**Secrets** (`MQTTSensorClient/arduino_secrets.h` - gitignored):

```cpp
#define SECRET_SSID "..."
#define SECRET_PASSWORD "..."
#define SECRET_PROVISIONNING_IP "192.168.1.x"
```

**System parameters** (`Config.h`):

- `SLEEP_TIME` - Deep sleep duration (default: 300000ms)
- `*_THRESHOLD` - Change detection thresholds per sensor
- `MAX_NO_SEND_INTERVAL` - Forced update interval (default: 1 hour)
- `DEBUG_ENABLED` - Comment out for production (zero overhead)

## Provisioning Server

Python UDP server (`id_server.py`) assigns device IDs and provides MQTT broker address.

```bash
# Run provisioning server
python id_server.py
```

Environment variables: `SERVER_HOST`, `SERVER_PORT`, `MQTT_BROKER_IP`, `MAP_FILE`

## Binary Message Format

```text
[Device ID: 1 byte][Flags: 1 byte][Sensor values: 0-8 bytes]
```

Flags bitmap (LSB first): Temp, Humidity, Pressure, Lux, UVA, UVB, UVI, Battery

MQTT topic: `sensors/{deviceID}/compact`

## Debug Mode

Enable in `Config.h`:

```cpp
#define DEBUG_ENABLED  // Comment to disable
```

Serial monitor: 9600 baud (changes to 115200 in some contexts)

## Language Note

Documentation and code comments are in French. Variable/function names use English conventions.
