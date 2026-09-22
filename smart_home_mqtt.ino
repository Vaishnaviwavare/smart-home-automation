/*
  ==========================================================
  SMART HOME AUTOMATION SYSTEM - MQTT Version
  Board    : ESP32
  Broker   : broker.hivemq.com (free public broker, for testing)
             -> for real use, run your own Mosquitto broker
                or use a free tier on HiveMQ Cloud / EMQX Cloud
  Dashboard: Node-RED, MQTT Explorer, or a phone MQTT app
             (e.g. IoT MQTT Panel) subscribed to the topics below
  ==========================================================

  SETUP STEPS:
  1. Install library: PubSubClient (by Nick O'Leary) via Library Manager
  2. Install: DHT sensor library + Adafruit Unified Sensor
  3. Fill in your WiFi credentials and broker details below
  4. Upload to ESP32
  5. Test with MQTT Explorer (https://mqtt-explorer.com/) or
     mosquitto_sub, subscribing to "home/#"
  6. Publish "1" or "0" to home/relay1 or home/relay2 to test control

  TOPICS:
    home/relay1      (subscribe) - "1"/"0" -> controls light relay
    home/relay2      (subscribe) - "1"/"0" -> controls fan relay
    home/temperature (publish)   - float, °C
    home/humidity    (publish)   - float, %
    home/motion      (publish)   - "1"/"0"
    home/gas         (publish)   - raw ADC value
    home/alert       (publish)   - text alerts (motion/gas)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ---------------- WiFi & MQTT ----------------
const char* ssid       = "YOUR_WIFI_SSID";
const char* password   = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";   // replace with your own broker for production
const int   mqtt_port   = 1883;
const char* mqtt_client_id = "ESP32_SmartHome_01"; // must be unique on the broker

// ---------------- Topics ----------------
const char* topic_relay1 = "home/relay1";
const char* topic_relay2 = "home/relay2";
const char* topic_temp   = "home/temperature";
const char* topic_hum    = "home/humidity";
const char* topic_motion = "home/motion";
const char* topic_gas    = "home/gas";
const char* topic_alert  = "home/alert";

// ---------------- Pins ----------------
#define DHTPIN      4
#define DHTTYPE     DHT11
#define PIR_PIN     27
#define MQ2_PIN     34
#define BUZZER_PIN  26
#define RELAY1_PIN  25
#define RELAY2_PIN  33

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
int gasThreshold = 1500;   // calibrate for your MQ-2 unit

void setup_wifi() {
  delay(10);
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  String t = String(topic);

  if (t == topic_relay1) digitalWrite(RELAY1_PIN, msg == "1" ? HIGH : LOW);
  if (t == topic_relay2) digitalWrite(RELAY2_PIN, msg == "1" ? HIGH : LOW);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      client.subscribe(topic_relay1);
      client.subscribe(topic_relay2);
    } else {
      Serial.printf("failed, rc=%d, retrying in 2s\n", client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 3000) {   // every 3 s
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) client.publish(topic_temp, String(t).c_str());
    if (!isnan(h)) client.publish(topic_hum, String(h).c_str());

    int motion = digitalRead(PIR_PIN);
    client.publish(topic_motion, motion ? "1" : "0");
    if (motion) client.publish(topic_alert, "Motion detected!");

    int gasValue = analogRead(MQ2_PIN);
    client.publish(topic_gas, String(gasValue).c_str());
    if (gasValue > gasThreshold) {
      digitalWrite(BUZZER_PIN, HIGH);
      client.publish(topic_alert, "Gas leak detected!");
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
