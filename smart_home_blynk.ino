/*
  ==========================================================
  SMART HOME AUTOMATION SYSTEM - Blynk Version
  Board   : ESP32 (also works on ESP8266 with minor pin edits)
  Sensors : DHT11 (Temp/Humidity), PIR (Motion), MQ-2 (Gas)
  Actuators: 2-channel Relay Module, Buzzer
  App     : Blynk IoT (blynk.cloud)
  ==========================================================

  SETUP STEPS:
  1. Create a free account at https://blynk.cloud
  2. Create a new Template -> note down Template ID & Name
  3. Add datastreams (Virtual Pins) in the template:
       V0 - Relay 1 (Light)     -> Type: Switch (0/1)
       V1 - Relay 2 (Fan)       -> Type: Switch (0/1)
       V2 - Temperature         -> Type: Value (Double, °C)
       V3 - Humidity            -> Type: Value (Double, %)
       V4 - Motion Status       -> Type: Value (Integer 0/1)
       V5 - Gas Level           -> Type: Value (Integer, raw ADC)
  4. Create a New Device from that template -> copy the Auth Token
  5. Paste Template ID, Template Name, Auth Token below
  6. Install libraries (Arduino IDE > Library Manager):
       - Blynk (by Volodymyr Shymanskyy)
       - DHT sensor library (by Adafruit)
       - Adafruit Unified Sensor (dependency of DHT lib)
  7. Board Manager: install "esp32 by Espressif Systems"
  8. Select Board: ESP32 Dev Module, correct COM port, upload.
  9. Build the Blynk mobile dashboard using the same virtual pins.
*/

#define BLYNK_TEMPLATE_ID   "YourTemplateID"      // <-- replace
#define BLYNK_TEMPLATE_NAME "Smart Home Automation"
#define BLYNK_AUTH_TOKEN    "YourAuthToken"        // <-- replace

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ---------------- WiFi credentials ----------------
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------------- Pin definitions ------------------
#define DHTPIN      4
#define DHTTYPE     DHT11
#define PIR_PIN     27
#define MQ2_PIN     34      // must be an ADC-capable pin on ESP32
#define BUZZER_PIN  26
#define RELAY1_PIN  25      // Light
#define RELAY2_PIN  33      // Fan

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

int  gasThreshold    = 1500;   // calibrate for your MQ-2 unit
bool gasAlertSent     = false;
bool motionAlertSent  = false;

// ---------- App -> Device: relay control ----------
BLYNK_WRITE(V0) {  // Light relay
  int state = param.asInt();
  digitalWrite(RELAY1_PIN, state);
}

BLYNK_WRITE(V1) {  // Fan relay
  int state = param.asInt();
  digitalWrite(RELAY2_PIN, state);
}

// Sync relay states with app when device (re)connects
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0, V1);
}

// ---------- Device -> App: sensor readings ----------
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
  } else {
    Serial.println("DHT read failed");
  }

  int motion = digitalRead(PIR_PIN);
  Blynk.virtualWrite(V4, motion);
  if (motion == HIGH && !motionAlertSent) {
    Blynk.logEvent("motion_detected", "Motion detected at home!");
    motionAlertSent = true;
  } else if (motion == LOW) {
    motionAlertSent = false;
  }

  int gasValue = analogRead(MQ2_PIN);
  Blynk.virtualWrite(V5, gasValue);
  if (gasValue > gasThreshold && !gasAlertSent) {
    digitalWrite(BUZZER_PIN, HIGH);
    Blynk.logEvent("gas_alert", "Gas leak detected! Level: " + String(gasValue));
    gasAlertSent = true;
  } else if (gasValue <= gasThreshold) {
    digitalWrite(BUZZER_PIN, LOW);
    gasAlertSent = false;
  }

  Serial.printf("T:%.1fC H:%.1f%% Motion:%d Gas:%d\n", t, h, motion, gasValue);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Most relay modules are ACTIVE-LOW; if yours turns ON at boot, swap HIGH/LOW here
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData); // read + push every 2 s
}

void loop() {
  Blynk.run();
  timer.run();
}
