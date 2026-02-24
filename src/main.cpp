#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "DoorMonitor.h"

// WiFi credentials from environment
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

#define DOOR_TRIGGER_PIN 14  // GPIO 14 (D5) - Digital output to trigger garage door
#define SDA_PIN 4            // GPIO 4 (D2) - I2C Data
#define SCL_PIN 5            // GPIO 5 (D1) - I2C Clock

// MPU-6050 sensor
Adafruit_MPU6050 mpu;

// Door monitor instance
DoorMonitor doorMonitor;

ESP8266WebServer server(80);

// HTML page
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 20px; background: #f5f5f5; }
    .container { max-width: 800px; margin: 0 auto; }
    h1 { color: #333; }
    .button {
      display: inline-block;
      padding: 20px 40px;
      font-size: 24px;
      margin: 10px;
      cursor: pointer;
      border: none;
      border-radius: 8px;
      color: white;
      background-color: #2196F3;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    .button:hover { opacity: 0.8; transform: translateY(-2px); }
    .status {
      font-size: 18px;
      margin: 20px 0;
      padding: 20px;
      border-radius: 12px;
      background-color: white;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      text-align: left;
    }
    .status-row { display: flex; justify-content: space-between; margin: 10px 0; padding: 8px; border-bottom: 1px solid #eee; }
    .status-label { font-weight: bold; color: #666; }
    .sensor-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin: 20px 0; }
    .sensor-card {
      background: white;
      padding: 15px;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    .sensor-label { font-size: 14px; color: #666; margin-bottom: 5px; }
    .sensor-value { font-size: 28px; font-weight: bold; color: #333; }
    .sensor-unit { font-size: 14px; color: #999; }
    .closed { color: #4CAF50; font-weight: bold; }
    .open { color: #2196F3; font-weight: bold; }
    .door_closed { color: #4CAF50; font-weight: bold; }
    .door_open { color: #2196F3; font-weight: bold; }
    .door_opening { color: #ff9800; font-weight: bold; }
    .door_closing { color: #ff9800; font-weight: bold; }
    .door_stopped { color: #f44336; font-weight: bold; }
    .door_unknown { color: #999; font-weight: bold; }
    .opening { color: #ff9800; }
    .closing { color: #ff9800; }
    .stopped { color: #f44336; }
    .health-ok { color: #4CAF50; font-weight: bold; }
    .health-fail { color: #f44336; font-weight: bold; }
    .timestamp { font-size: 12px; color: #999; text-align: right; margin-top: 10px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🚪 Garage Door Monitor</h1>
    
    <div class="sensor-grid">
      <div class="sensor-card">
        <div class="sensor-label">Accel Y (Vertical)</div>
        <div class="sensor-value" id="accelY">--</div>
        <div class="sensor-unit">m/s²</div>
      </div>
      <div class="sensor-card">
        <div class="sensor-label">Accel Z (Horizontal)</div>
        <div class="sensor-value" id="accelZ">--</div>
        <div class="sensor-unit">m/s²</div>
      </div>
      <div class="sensor-card">
        <div class="sensor-label">Accel X</div>
        <div class="sensor-value" id="accelX">--</div>
        <div class="sensor-unit">m/s²</div>
      </div>
    </div>
    
    <div class="status">
      <div class="status-row">
        <span class="status-label">Door State:</span>
        <span id="status" class="stopped">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Status Details:</span>
        <span id="details">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Movement:</span>
        <span id="moving">--</span>
      </div>
      <div class="status-row">
        <span class="status-label">At Position:</span>
        <span id="atPosition">--</span>
      </div>
      <div class="status-row">
        <span class="status-label">Sensor Health:</span>
        <span id="sensorHealth">--</span>
      </div>
      <div class="timestamp">Last update: <span id="timestamp">--</span></div>
    </div>
    
    <button class="button" onclick="triggerDoor()">Trigger Door</button>
  </div>
  <script>
    function triggerDoor() {
      fetch('/trigger')
        .then(response => response.text())
        .then(data => {
          console.log('Door triggered:', data);
        });
    }
    
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          let statusEl = document.getElementById('status');
          statusEl.innerText = data.state;
          statusEl.className = data.state.toLowerCase().replace(/ /g, '_');
          
          document.getElementById('details').innerText = data.details;
          document.getElementById('accelY').innerText = data.accelY.toFixed(2);
          document.getElementById('accelZ').innerText = data.accelZ.toFixed(2);
          document.getElementById('accelX').innerText = data.accelX.toFixed(2);
          
          document.getElementById('moving').innerText = data.isMoving ? 'YES' : 'No';
          document.getElementById('atPosition').innerText = data.isAtPosition ? 'YES' : 'No';
          
          let healthEl = document.getElementById('sensorHealth');
          healthEl.innerText = data.sensorHealthy ? '✓ OK' : '✗ FAILED';
          healthEl.className = data.sensorHealthy ? 'health-ok' : 'health-fail';
          
          let now = new Date();
          document.getElementById('timestamp').innerText = now.toLocaleTimeString();
        })
        .catch(err => {
          console.error('Update failed:', err);
          document.getElementById('status').innerText = 'Connection Error';
        });
    }
    
    // Update status every 500ms
    setInterval(updateStatus, 500);
    updateStatus();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleTrigger() {
  // Pulse the door trigger pin (simulate button press)
  digitalWrite(DOOR_TRIGGER_PIN, HIGH);
  delay(500);  // 500ms pulse
  digitalWrite(DOOR_TRIGGER_PIN, LOW);
  
  Serial.println("Door trigger activated");
  server.send(200, "text/plain", "Door triggered");
}

AccelData readSensorData() {
  AccelData data;
  sensors_event_t a, g, temp;
  
  if (mpu.getEvent(&a, &g, &temp)) {
    data.x = a.acceleration.x;
    data.y = a.acceleration.y;
    data.z = a.acceleration.z;
    data.valid = true;
  } else {
    data.x = 0;
    data.y = 0;
    data.z = 0;
    data.valid = false;
  }
  
  return data;
}

void handleStatus() {
  AccelData accel = readSensorData();
  doorMonitor.updateState(accel, millis());
  
  String json = "{";
  json += "\"state\":\"" + String(doorMonitor.getStateString()) + "\",";
  json += "\"details\":\"" + String(doorMonitor.getDetailedStatus()) + "\",";
  json += "\"accelX\":" + String(accel.x, 2) + ",";
  json += "\"accelY\":" + String(accel.y, 2) + ",";
  json += "\"accelZ\":" + String(accel.z, 2) + ",";
  json += "\"isMoving\":" + String(doorMonitor.isMoving() ? "true" : "false") + ",";
  json += "\"isAtPosition\":" + String(doorMonitor.isAtPosition() ? "true" : "false") + ",";
  json += "\"sensorHealthy\":" + String(doorMonitor.isSensorHealthy() ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize door trigger pin
  pinMode(DOOR_TRIGGER_PIN, OUTPUT);
  digitalWrite(DOOR_TRIGGER_PIN, LOW);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  
  // Configure MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Get initial reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  doorMonitor.initialize(a.acceleration.y, a.acceleration.z, millis());
  
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  doorMonitor.initialize(a.acceleration.y, a.acceleration.z, millis());
  
  server.on("/", handleRoot);
  server.on("/trigger", handleTrigger);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Garage door monitor ready");
}

void loop() {
  server.handleClient();
  
  // Continuously monitor door state
  static unsigned long lastUpdate = 0;
  static unsigned long lastPrint = 0;
  
  if (millis() - lastUpdate > 100) {  // Update every 100ms
    AccelData accel = readSensorData();
    doorMonitor.updateState(accel, millis());
    lastUpdate = millis();
    
    // Print sensor readings every 2 seconds
    if (millis() - lastPrint > 2000) {
      Serial.print("MPU6050 - Y: ");
      Serial.print(accel.y, 2);
      Serial.print(" m/s², Z: ");
      Serial.print(accel.z, 2);
      Serial.print(" m/s² | State: ");
      Serial.print(doorMonitor.getStateString());
      Serial.print(" | ");
      Serial.println(doorMonitor.getDetailedStatus());
      lastPrint = millis();
    }
    
    // Print status changes immediately
    static DoorState lastPrintedState = DOOR_UNKNOWN;
    DoorState currentState = doorMonitor.getState();
    if (currentState != lastPrintedState) {
      Serial.print("*** STATE CHANGE *** Door state: ");
      Serial.print(doorMonitor.getStateString());
      Serial.print(" | ");
      Serial.println(doorMonitor.getDetailedStatus());
      lastPrintedState = currentState;
    }
  }
}