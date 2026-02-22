#include <Arduino.h>
#include "MHZ19.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

// ------------------- MH-Z19E Config -------------------
#define RX_PIN 16 //16 dev module esp32 37 for esps2
#define TX_PIN 17 //17
#define BAUDRATE 9600

// ------------------- LED Config -------------------
#define LED_PIN LED_BUILTIN
#define LEDC_FREQ 5000
#define LEDC_RESOLUTION 8

MHZ19 myMHZ19;
HardwareSerial mySerial(1); // UART1

unsigned long getDataTimer = 0;
int latestCO2 = 0;
int latestTemp = 0;
bool abcEnabled = true;
int thresholdPpm = 1000;
bool sensorBusy = false;
String fwVersion = "";
int rangePpm = 2000;
unsigned long resetAtMs = 0;

const int thresholdMinPpm = 400;
const int thresholdMaxPpm = 5000;
const int rangeMinPpm = 1000;
const int rangeMaxPpm = 10000;
const unsigned long calibrationCooldownMs = 10UL * 1000UL;
unsigned long lastCalibrationMs = 0;

Preferences prefs;

// ------------------- Wi-Fi Config -------------------
const char* staSsid = "YOUR-SSID-WIFI";
const char* staPassword = "YOUR-WIFI-PASSWORD";
const char* apSsid = "MHZ19-AP";
const char* apPassword = "12345678";

// ------------------- Web Server -------------------
AsyncWebServer server(80);

extern const char index_html[] PROGMEM;

void setup(){
  Serial.begin(115200);
  delay(2500);

  prefs.begin("co2", false);
  thresholdPpm = prefs.getInt("threshold", thresholdPpm);
  if (thresholdPpm < thresholdMinPpm || thresholdPpm > thresholdMaxPpm) {
    thresholdPpm = 1000;
  }
  rangePpm = prefs.getInt("range", rangePpm);
  if (rangePpm < rangeMinPpm || rangePpm > rangeMaxPpm) {
    rangePpm = 2000;
  }
  abcEnabled = prefs.getBool("abc", abcEnabled);
  lastCalibrationMs = prefs.getULong("cal_ms", 0);
  fwVersion = prefs.getString("fw", "");

  // --- Configure LED PWM ---
  ledcAttach(LED_PIN, LEDC_FREQ, LEDC_RESOLUTION);

  // --- Connect to known Wi-Fi, fallback to AP ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(staSsid, staPassword);

  const unsigned long wifiTimeoutMs = 10000;
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < wifiTimeoutMs) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to Wi-Fi: " + String(staSsid));
    Serial.println("IP: " + WiFi.localIP().toString());
  } else {
    WiFi.mode(WIFI_AP);
    bool apStarted = WiFi.softAP(apSsid, apPassword);
    delay(200);
    Serial.println("AP started: " + String(apStarted ? "YES" : "NO"));
    Serial.println("SSID: " + String(apSsid));
    Serial.println("IP: " + WiFi.softAPIP().toString());
  }

  // --- Initialize MH-Z19E (commented out) ---
  
  mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(abcEnabled);
  myMHZ19.setFilter(true, true);
  myMHZ19.setRange(rangePpm);

  char myVersion[4];          
  myMHZ19.getVersion(myVersion);
  char fwStr[8];
  snprintf(fwStr, sizeof(fwStr), "%c%c.%c%c", myVersion[0], myVersion[1], myVersion[2], myVersion[3]);
  fwVersion = String(fwStr);
  prefs.putString("fw", fwVersion);

  Serial.print("\nFirmware Version: ");
  for(byte i = 0; i < 4; i++)
  {
    Serial.print(myVersion[i]);
    if(i == 1)
      Serial.print(".");    
  }
  Serial.println("");

  int sensorRange = myMHZ19.getRange();
  int backgroundCO2 = myMHZ19.getBackgroundCO2();
  int tempCal = myMHZ19.getTempAdjustment();
  if (sensorRange > 0) {
    rangePpm = sensorRange;
    prefs.putInt("range", rangePpm);
  }

  Serial.print("Range: ");
  Serial.println(sensorRange);   
  Serial.print("Background CO2: ");
  Serial.println(backgroundCO2);
  Serial.print("Temperature Cal: ");
  Serial.println(tempCal);
  Serial.print("ABC Status: "); myMHZ19.getABC() ? Serial.println("ON") :  Serial.println("OFF");

  

  // --- Configure server routes (Core 0) ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    bool sensorABC = myMHZ19.getABC();
    if (sensorABC != abcEnabled) {
      abcEnabled = sensorABC;
      prefs.putBool("abc", abcEnabled);
    }
    String mood = latestCO2 < thresholdPpm ? "happy" : "sad";
    unsigned long nowMs = millis();
    unsigned long elapsed = nowMs - lastCalibrationMs;
    unsigned long remaining = 0;
    if (elapsed < calibrationCooldownMs) {
      remaining = calibrationCooldownMs - elapsed;
    }
    String json = "{\"co2\":" + String(latestCO2) +
      ",\"temp\":" + String(latestTemp) +
      ",\"mood\":\"" + mood + "\"" +
      ",\"threshold\":" + String(thresholdPpm) +
      ",\"range\":" + String(rangePpm) +
      ",\"abc\":" + String(abcEnabled ? "true" : "false") +
      ",\"calRemainingMs\":" + String(remaining) +
      ",\"fw\":\"" + fwVersion + "\"" +
      "}";
    request->send(200, "application/json", json);
  });

  server.on("/abc", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!request->hasParam("enabled", true)) {
      request->send(400, "application/json", "{\"error\":\"missing enabled\"}");
      return;
    }
    String value = request->getParam("enabled", true)->value();

    if (value != "true" && value != "false") {
      request->send(400, "application/json", "{\"error\":\"invalid enabled\"}");
      return;
    }
    bool enabled = value == "true";
    if (sensorBusy) {
      request->send(409, "application/json", "{\"error\":\"sensor busy\"}");
      return;
    }
    sensorBusy = true;
    myMHZ19.autoCalibration(enabled);
    abcEnabled = enabled;
    prefs.putBool("abc", abcEnabled);
    sensorBusy = false;
    String json = "{\"abc\":" + String(abcEnabled ? "true" : "false") + "}";
    request->send(200, "application/json", json);
  });

  server.on("/abc/refresh", HTTP_GET, [](AsyncWebServerRequest *request){
    bool sensorABC = myMHZ19.getABC();
    String json = "{\"abc\":" + String(sensorABC ? "true" : "false") + "}";
    request->send(200, "application/json", json);
  });

  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
    unsigned long nowMs = millis();
    if ((nowMs - lastCalibrationMs) < calibrationCooldownMs) {
      unsigned long remaining = calibrationCooldownMs - (nowMs - lastCalibrationMs);
      String json = "{\"error\":\"cooldown\",\"retryAfterMs\":" + String(remaining) + "}";
      request->send(429, "application/json", json);
      return;
    }
    if (sensorBusy) {
      request->send(409, "application/json", "{\"error\":\"sensor busy\"}");
      return;
    }
    sensorBusy = true;
    int co2Reading = myMHZ19.getCO2(true);
    if (co2Reading != 0) {
      latestCO2 = co2Reading;
    }
    myMHZ19.calibrate();
    lastCalibrationMs = nowMs;
    prefs.putULong("cal_ms", lastCalibrationMs);
    sensorBusy = false;
    String json = "{\"calibrated\":true,\"co2\":" + String(latestCO2) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/threshold", HTTP_POST, [](AsyncWebServerRequest *request){
    String value;
    if (request->hasParam("value", true)) {
      value = request->getParam("value", true)->value();
    } else if (request->hasParam("value")) {
      value = request->getParam("value")->value();
    } else {
      request->send(400, "application/json", "{\"error\":\"missing value\"}");
      return;
    }
    int requested = value.toInt();
    if (requested <= 0) {
      request->send(400, "application/json", "{\"error\":\"invalid value\"}");
      return;
    }
    if (requested < thresholdMinPpm) requested = thresholdMinPpm;
    if (requested > thresholdMaxPpm) requested = thresholdMaxPpm;
    thresholdPpm = requested;
    prefs.putInt("threshold", thresholdPpm);
    String json = "{\"threshold\":" + String(thresholdPpm) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/range", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!request->hasParam("value", true)) {
      request->send(400, "application/json", "{\"error\":\"missing value\"}");
      return;
    }
    int requested = request->getParam("value", true)->value().toInt();
    if (requested <= 0) {
      request->send(400, "application/json", "{\"error\":\"invalid value\"}");
      return;
    }
    if (requested < rangeMinPpm) requested = rangeMinPpm;
    if (requested > rangeMaxPpm) requested = rangeMaxPpm;
    if (sensorBusy) {
      request->send(409, "application/json", "{\"error\":\"sensor busy\"}");
      return;
    }
    sensorBusy = true;
    myMHZ19.setRange(requested);
    rangePpm = requested;
    prefs.putInt("range", rangePpm);
    sensorBusy = false;
    String json = "{\"range\":" + String(rangePpm) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/background", HTTP_GET, [](AsyncWebServerRequest *request){
    if (sensorBusy) {
      request->send(409, "application/json", "{\"error\":\"sensor busy\"}");
      return;
    }
    sensorBusy = true;
    int backgroundCO2 = myMHZ19.getBackgroundCO2();
    sensorBusy = false;
    String json = "{\"background\":" + String(backgroundCO2) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    resetAtMs = millis() + 200;
    request->send(200, "application/json", "{\"reset\":true}");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });

  // --- Start AsyncWebServer on Core 0 ---
  server.begin();

    // --- Sensor task removed to avoid core pinning ---
}

void loop(){
  if (resetAtMs != 0 && millis() >= resetAtMs) {
    delay(50);
    ESP.restart();
  }
  if (!sensorBusy && (millis() - getDataTimer >= 2000)){
    int co2Reading = myMHZ19.getCO2(true); // Request CO2 (as ppm)
    if (co2Reading != 0) {
      latestCO2 = co2Reading;
    }
    latestTemp = myMHZ19.getTemperature();
    getDataTimer = millis();

    Serial.print("CO2 (ppm): ");
    Serial.print(latestCO2);
    Serial.print(" | Temp (C): ");
    Serial.println(latestTemp);
  }

  if (latestCO2 < thresholdPpm) {
    ledcWrite(LED_PIN, 255);
  } else {
    const unsigned long pulsePeriodMs = 2000;
    unsigned long phase = millis() % pulsePeriodMs;
    uint8_t duty = phase < pulsePeriodMs / 2
      ? map(phase, 0, pulsePeriodMs / 2, 0, 255)
      : map(phase, pulsePeriodMs / 2, pulsePeriodMs, 255, 0);
    ledcWrite(LED_PIN, duty);
  }
}
