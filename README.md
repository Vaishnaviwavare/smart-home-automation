# 🏠 Smart Home Automation System (ESP32 + Blynk/MQTT)

An IoT-based home automation system that lets you control appliances (light, fan)
remotely from a mobile app, while continuously logging temperature, humidity,
motion, and gas leak data — with real-time alerts.

## 📋 Table of Contents
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Required](#hardware-required)
- [Circuit Diagram & Wiring](#circuit-diagram--wiring)
- [Software Setup](#software-setup)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Testing](#testing)
- [Future Improvements](#future-improvements)
- [Demo](#demo)

## Features
- 📱 Remote ON/OFF control of 2 appliances (light & fan) via relay module
- 🌡️ Real-time temperature & humidity logging (DHT11)
- 🚶 Motion detection with push notification (PIR sensor)
- 🔥 Gas leak detection with buzzer alarm + app alert (MQ-2 sensor)
- ☁️ Two connectivity options included: **Blynk IoT** (easiest for beginners)
  and **MQTT** (industry-standard, works with Node-RED/Home Assistant)
- 🔌 Works on ESP32 (WiFi built-in); portable to ESP8266 with pin changes

## System Architecture
```
[DHT11] [PIR] [MQ-2]  --sensors-->  [ESP32]  --WiFi-->  [Blynk Cloud / MQTT Broker]  --> [Mobile App / Dashboard]
                                       |
                                       v
                              [Relay Module] --> [Light, Fan]
                              [Buzzer] --> [Gas Alarm]
```

## Hardware Required

| Component                     | Qty | Approx. Price (INR) | Notes                              |
|-------------------------------|-----|----------------------|-------------------------------------|
| ESP32 DevKit (30/38-pin)      | 1   | ₹350–500             | ESP8266 NodeMCU also works          |
| 2-Channel Relay Module (5V)   | 1   | ₹100–150             | Use opto-isolated module for safety |
| DHT11 Temperature/Humidity    | 1   | ₹50–80               | DHT22 for better accuracy           |
| PIR Motion Sensor (HC-SR501)  | 1   | ₹60–100              |                                      |
| MQ-2 Gas Sensor Module        | 1   | ₹80–120              | Detects LPG, smoke, propane         |
| Active Buzzer                 | 1   | ₹20–30               |                                      |
| Breadboard + Jumper Wires     | -   | ₹150                 |                                      |
| 5V/2A USB Power Adapter       | 1   | ₹100                 |                                      |
| (Optional) Bulb holder + AC bulb, small fan/DC motor for demo | - | - | For a live demo video |

⚠️ **Safety note**: If switching mains (220V/110V) appliances, use a relay
module rated for mains voltage, keep AC wiring insulated, and ideally demo
with a low-voltage DC bulb/fan instead of live mains for a college project.

## Circuit Diagram & Wiring

| ESP32 Pin | Connects To          |
|-----------|-----------------------|
| GPIO 4    | DHT11 Data pin         |
| GPIO 27   | PIR OUT                |
| GPIO 34   | MQ-2 AOUT (analog)     |
| GPIO 26   | Buzzer +               |
| GPIO 25   | Relay IN1 (Light)      |
| GPIO 33   | Relay IN2 (Fan)        |
| 5V / 3.3V | VCC of all sensors*    |
| GND       | GND — common ground for everything |

\*DHT11, PIR, and relay module typically run on 5V; MQ-2 module usually
also needs 5V for its heater coil. Double-check your specific module's
datasheet. Draw the schematic in [Fritzing](https://fritzing.org/) or
[draw.io](https://app.diagrams.net/) and export it as `circuit_diagram.png`
to include in this repo.

## Software Setup

### 1. Arduino IDE setup
1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. File → Preferences → Additional Board URLs, add:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Tools → Board → Boards Manager → search "esp32" → install

### 2. Install Libraries (Sketch → Include Library → Manage Libraries)
- `Blynk` by Volodymyr Shymanskyy (for the Blynk version)
- `PubSubClient` by Nick O'Leary (for the MQTT version)
- `DHT sensor library` by Adafruit
- `Adafruit Unified Sensor` (dependency)

### 3A. Blynk setup (recommended for freshers/demo)
1. Sign up at [blynk.cloud](https://blynk.cloud)
2. Create a new **Template** → name it "Smart Home Automation"
3. Add datastreams: V0 (Relay1), V1 (Relay2), V2 (Temp), V3 (Humidity),
   V4 (Motion), V5 (Gas) — see comments in `smart_home_blynk.ino`
4. Create a **Device** from the template → copy the **Auth Token**
5. Paste your WiFi SSID/password + Template ID/Name/Auth Token into
   `smart_home_blynk/smart_home_blynk.ino`
6. Build a simple mobile dashboard in the Blynk app (switches + gauges)
7. Upload the sketch to your ESP32

### 3B. MQTT setup (alternative / more "industry standard")
1. Use the public test broker `broker.hivemq.com` (fine for a demo) or
   set up your own broker with [Mosquitto](https://mosquitto.org/)
2. Fill in WiFi + broker details in `smart_home_mqtt/smart_home_mqtt.ino`
3. Upload to ESP32
4. Monitor/test with [MQTT Explorer](https://mqtt-explorer.com/) or
   `mosquitto_sub -h broker.hivemq.com -t "home/#"`
5. (Optional) Build a dashboard in **Node-RED** subscribing to the topics

## Project Structure
```
smart-home-automation/
├── README.md
├── smart_home_blynk/
│   └── smart_home_blynk.ino      # Blynk version
├── smart_home_mqtt/
│   └── smart_home_mqtt.ino       # MQTT version
├── circuit_diagram.png           # (add your own schematic image)
└── demo.gif / demo_video_link.md # (add a short demo clip)
```

## How It Works
1. ESP32 connects to WiFi and to Blynk Cloud / an MQTT broker.
2. Every 2–3 seconds it reads DHT11, PIR, and MQ-2 sensors.
3. Sensor values are pushed to the cloud/broker and shown on the app/dashboard.
4. When you tap a switch in the app, the message is sent to the ESP32,
   which toggles the relay controlling the light/fan.
5. If gas level crosses the threshold, the buzzer sounds and an alert
   notification is sent. Same for motion detection.

## Testing
- Serial Monitor (115200 baud) prints live sensor values for debugging.
- Test relay switching independently first (before wiring appliances).
- Calibrate `gasThreshold` by exposing the MQ-2 to a lighter's unburned
  gas briefly (safely, in ventilated area) and noting the ADC value rise.
- Test WiFi reconnection by power-cycling your router.

## Future Improvements
- Add a **web dashboard** (React/HTML) instead of relying only on Blynk app
- Store historical sensor data in **Firebase/InfluxDB** and plot trends
- Add **voice control** via Google Assistant/Alexa (using IFTTT + Blynk)
- Add **OTA (over-the-air) firmware updates**
- Add a **local fallback mode** (physical switches) if WiFi is down
- Migrate from Blynk legacy timers to fully async MQTT for lower latency

## Demo
_Add a short video/GIF here showing the app controlling the relay and the
gas/motion alert firing — recruiters engage with this far more than code._

---
**Author:** Your Name
**Contact:** your.email@example.com | [LinkedIn](#)
