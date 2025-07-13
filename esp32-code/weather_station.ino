#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_NAME "Weather Station"
#define BLYNK_TEMPLATE_ID "TMPL2D0KSKaqZ"    // Replace with your Template ID
#define BLYNK_DEVICE_NAME "Weather Station"  // Replace with your device name
#define BLYNK_AUTH_TOKEN "h1ff7e-FF1IYy1il2kw3ARymSXGImd8g"  // Replace with your Auth Token

// ESP32 specific includes - Order matters!
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Wire.h>

// Third-party libraries
#include <BlynkSimpleEsp32.h>
#include <Adafruit_BMP085.h>     // BMP180 sensor
#include <DHT.h>
#include <PubSubClient.h>        // MQTT
#include <ArduinoJson.h>         // JSON

// WiFi Credentials
const char* ssid = "abdelilah";     
const char* password = "abdelilah2025"; 

// Blynk Auth
char auth[] = BLYNK_AUTH_TOKEN;

// Pins
#define DHT_PIN 4
#define LDR_PIN 34
#define RAIN_PIN 32
#define DHT_TYPE DHT22

// Sensor Objects
Adafruit_BMP085 bmp;
DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

// MQTT Setup
WiFiClient espClient;
PubSubClient mqtt(espClient);
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "weatherstation/data";

// Web Server Setup
WebServer server(80);

// Sensor readings
float temperature_bmp = 0;
float temperature_dht = 0;
float humidity = 0;
float pressure = 0;
float altitude = 0;
int light_intensity = 0;
int rain_level = 0;
bool rain_detected = false;

// Data analysis
float temp_avg = 0;
float humidity_avg = 0;
int reading_count = 0;
float max_temp = -100;
float min_temp = 100;

// Virtual pins for Blynk
#define V_TEMP_BMP    V0
#define V_TEMP_DHT    V1
#define V_HUMIDITY    V2
#define V_PRESSURE    V3
#define V_LIGHT       V4
#define V_RAIN        V5
#define V_ALTITUDE    V6
#define V_RAIN_STATUS V7
#define V_TEMP_AVG    V8
#define V_TEMP_MAX    V9
#define V_TEMP_MIN    V10
#define V_RESET_STATS V15

// Function Declarations
void handleDataRequest();
void handleRoot();
void handleNotFound();
void readAndSendSensors();
void printSensorData();
void performDataAnalysis();
void sendMQTTData();
void reconnectMQTT();

// Web Server Functions
void handleDataRequest() {
  Serial.println("Web client requesting data...");
  
  // Create JSON response with current sensor data
  String json = "{";
  json += "\"temperature_bmp\":" + String(temperature_bmp, 2) + ",";
  json += "\"temperature_dht\":" + String(temperature_dht, 2) + ",";
  json += "\"humidity\":" + String(humidity, 2) + ",";
  json += "\"pressure\":" + String(pressure, 2) + ",";
  json += "\"light\":" + String(light_intensity) + ",";
  json += "\"rain\":" + String(rain_level) + ",";
  json += "\"rain_detected\":" + String(rain_detected ? "true" : "false") + ",";
  json += "\"altitude\":" + String(altitude, 2) + ",";
  json += "\"temp_avg\":" + String(temp_avg, 2) + ",";
  json += "\"temp_max\":" + String(max_temp, 2) + ",";
  json += "\"temp_min\":" + String(min_temp, 2) + ",";
  json += "\"readings\":" + String(reading_count) + ",";
  json += "\"timestamp\":" + String(millis());
  json += "}";
  
  // Send CORS headers to allow web browser access
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "application/json", json);
  
  Serial.println("✅ Data sent to web client");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 Weather Station</title>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;}";
  html += ".card{background:white;padding:20px;margin:10px;border-radius:10px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
  html += ".value{font-size:2em;color:#2196F3;font-weight:bold;}";
  html += ".label{color:#666;margin-bottom:5px;}";
  html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:15px;}";
  html += "</style></head><body>";
  
  html += "<h1>🌤️ ESP32 Weather Station</h1>";
  html += "<div class='grid'>";
  
  html += "<div class='card'><div class='label'>🌡️ Temperature (BMP180)</div>";
  html += "<div class='value'>" + String(temperature_bmp, 1) + " °C</div></div>";
  
  html += "<div class='card'><div class='label'>🌡️ Temperature (DHT22)</div>";
  html += "<div class='value'>" + String(temperature_dht, 1) + " °C</div></div>";
  
  html += "<div class='card'><div class='label'>💧 Humidity</div>";
  html += "<div class='value'>" + String(humidity, 1) + " %</div></div>";
  
  html += "<div class='card'><div class='label'>📊 Pressure</div>";
  html += "<div class='value'>" + String(pressure, 1) + " hPa</div></div>";
  
  html += "<div class='card'><div class='label'>☀️ Light Level</div>";
  html += "<div class='value'>" + String(light_intensity) + " %</div></div>";
  
  html += "<div class='card'><div class='label'>🌧️ Rain Level</div>";
  html += "<div class='value'>" + String(rain_level) + " %</div>";
  html += "<div>Status: " + String(rain_detected ? "RAIN DETECTED" : "DRY") + "</div></div>";
  
  html += "<div class='card'><div class='label'>⛰️ Altitude</div>";
  html += "<div class='value'>" + String(altitude, 1) + " m</div></div>";
  
  html += "<div class='card'><div class='label'>📈 Statistics</div>";
  html += "<div>Average: " + String(temp_avg, 1) + " °C</div>";
  html += "<div>Maximum: " + String(max_temp, 1) + " °C</div>";
  html += "<div>Minimum: " + String(min_temp, 1) + " °C</div>";
  html += "<div>Readings: " + String(reading_count) + "</div></div>";
  
  html += "</div>";
  html += "<hr><p><a href='/data' style='background:#2196F3;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;'>📊 Get JSON Data</a></p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  html += "<p><small>Refresh page for latest readings</small></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🚀 ESP32 Weather Station Starting...");
  Serial.println("ESP32 Arduino Core Version: " + String(ESP_ARDUINO_VERSION_MAJOR) + "." + String(ESP_ARDUINO_VERSION_MINOR));

  // Initialize I2C for BMP180
  Wire.begin(21, 22);
  delay(1000);

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("❌ Could not find BMP180 sensor!");
    Serial.println("Check wiring: SDA=21, SCL=22, VCC=3.3V, GND=GND");
    // Continue without BMP180
  } else {
    Serial.println("✅ BMP180 initialized successfully");
  }

  // Initialize DHT22
  dht.begin();
  delay(2000);
  Serial.println("✅ DHT22 initialized successfully");

  // Connect to WiFi
  Serial.println("📡 Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
    delay(1000);
    Serial.print(".");
    wifi_attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("📍 ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    Serial.println("Continuing in AP mode...");
    WiFi.softAP("ESP32_WeatherStation", "12345678");
    Serial.println("📍 AP IP address: " + WiFi.softAPIP().toString());
  }

  // Initialize Blynk (only if WiFi connected)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("📱 Connecting to Blynk...");
    Blynk.config(auth);
    if (Blynk.connect()) {
      Serial.println("✅ Blynk connected!");
    } else {
      Serial.println("❌ Blynk connection failed!");
    }
  }

  // Setup Web Server
  server.on("/", handleRoot);
  server.on("/data", HTTP_GET, handleDataRequest);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("🌐 Web server started!");
  Serial.println("🔗 Access dashboard at: http://" + WiFi.localIP().toString() + "/");
  Serial.println("📊 Get JSON data at: http://" + WiFi.localIP().toString() + "/data");

  // MQTT Setup (only if WiFi connected)
  if (WiFi.status() == WL_CONNECTED) {
    mqtt.setServer(mqtt_server, 1883);
    Serial.println("📡 MQTT client configured");
  }

  // Setup timers
  timer.setInterval(3000L, readAndSendSensors);  // Read sensors every 3 seconds
  timer.setInterval(30000L, performDataAnalysis); // Analysis every 30 seconds

  Serial.println("🎉 Weather Station Ready!");
  Serial.println("=================================");
  
  // Take initial reading
  readAndSendSensors();
}

void loop() {
  // Handle Blynk connection
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.run();
  }
  
  // Run timers
  timer.run();
  
  // Handle web server requests
  server.handleClient();

  // Handle MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) {
      reconnectMQTT();
    }
    mqtt.loop(); // Handle MQTT messages
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}

void readAndSendSensors() {
  Serial.println("📊 Reading sensors...");

  // Read BMP180 (with error handling)
  if (bmp.begin()) {
    temperature_bmp = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F; // Convert to hPa
    altitude = bmp.readAltitude();
  } else {
    temperature_bmp = 25.0; // Default value
    pressure = 1013.25;     // Sea level pressure
    altitude = 0;
    Serial.println("⚠️ BMP180 reading failed, using defaults");
  }

  // Read DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check for valid DHT readings
  if (isnan(h) || isnan(t)) {
    Serial.println("⚠️ Failed to read from DHT sensor!");
    temperature_dht = temperature_bmp; // Use BMP temp as backup
    humidity = 50.0; // Default humidity
  } else {
    temperature_dht = t;
    humidity = h;
  }

  // Read analog sensors
  int ldr_raw = analogRead(LDR_PIN);
  light_intensity = map(ldr_raw, 0, 4095, 0, 100);

  int rain_raw = analogRead(RAIN_PIN);
  rain_level = map(rain_raw, 0, 4095, 100, 0); // Inverted: dry=100%, wet=0%
  rain_detected = rain_raw < 2000; // Adjust threshold as needed

  // Send to Blynk (only if connected)
  if (Blynk.connected()) {
    Blynk.virtualWrite(V_TEMP_BMP, temperature_bmp);
    Blynk.virtualWrite(V_TEMP_DHT, temperature_dht);
    Blynk.virtualWrite(V_HUMIDITY, humidity);
    Blynk.virtualWrite(V_PRESSURE, pressure);
    Blynk.virtualWrite(V_LIGHT, light_intensity);
    Blynk.virtualWrite(V_RAIN, rain_level);
    Blynk.virtualWrite(V_ALTITUDE, altitude);
    Blynk.virtualWrite(V_RAIN_STATUS, rain_detected ? "RAIN!" : "DRY");
  }

  printSensorData();
  
  // Send MQTT data (only if connected)
  if (WiFi.status() == WL_CONNECTED) {
    sendMQTTData();
  }
}

void printSensorData() {
  Serial.println("🌡️ === Weather Data ===");
  Serial.printf("🌡️ Temp (BMP180): %.2f °C\n", temperature_bmp);
  Serial.printf("🌡️ Temp (DHT22): %.2f °C\n", temperature_dht);
  Serial.printf("💧 Humidity: %.2f %%\n", humidity);
  Serial.printf("📊 Pressure: %.2f hPa\n", pressure);
  Serial.printf("⛰️ Altitude: %.2f m\n", altitude);
  Serial.printf("☀️ Light: %d %%\n", light_intensity);
  Serial.printf("🌧️ Rain: %d %% (%s)\n", rain_level, rain_detected ? "DETECTED" : "DRY");
  Serial.printf("📶 WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.printf("📱 Blynk: %s\n", Blynk.connected() ? "Connected" : "Disconnected");
  Serial.printf("🆓 Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("========================");
}

void performDataAnalysis() {
  reading_count++;

  // Calculate running averages
  if (reading_count == 1) {
    temp_avg = temperature_dht;
    humidity_avg = humidity;
    max_temp = temperature_dht;
    min_temp = temperature_dht;
  } else {
    temp_avg = (temp_avg * (reading_count - 1) + temperature_dht) / reading_count;
    humidity_avg = (humidity_avg * (reading_count - 1) + humidity) / reading_count;
  }

  // Track min/max temperatures
  if (temperature_dht > max_temp) max_temp = temperature_dht;
  if (temperature_dht < min_temp) min_temp = temperature_dht;

  // Send analysis to Blynk (only if connected)
  if (Blynk.connected()) {
    Blynk.virtualWrite(V_TEMP_AVG, temp_avg);
    Blynk.virtualWrite(V_TEMP_MAX, max_temp);
    Blynk.virtualWrite(V_TEMP_MIN, min_temp);
  }

  Serial.println("📈 Data Analysis:");
  Serial.printf("📊 Avg Temp: %.2f °C\n", temp_avg);
  Serial.printf("🔥 Max Temp: %.2f °C\n", max_temp);
  Serial.printf("🧊 Min Temp: %.2f °C\n", min_temp);
  Serial.printf("📋 Readings: %d\n", reading_count);
}

void sendMQTTData() {
  if (mqtt.connected()) {
    // Create a smaller JSON document to avoid memory issues
    StaticJsonDocument<512> doc;  // Fixed-size JSON buffer
    
    doc["device"] = "ESP32_WeatherStation";
    doc["timestamp"] = millis();
    
    JsonObject sensors = doc.createNestedObject("sensors");
    sensors["temp_bmp"] = round(temperature_bmp * 100) / 100.0;
    sensors["temp_dht"] = round(temperature_dht * 100) / 100.0;
    sensors["humidity"] = round(humidity * 100) / 100.0;
    sensors["pressure"] = round(pressure * 100) / 100.0;
    sensors["light"] = light_intensity;
    sensors["rain"] = rain_level;
    sensors["rain_detected"] = rain_detected;

    String payload;
    serializeJson(doc, payload);

    if (mqtt.publish(mqtt_topic, payload.c_str())) {
      Serial.println("📡 Data sent via MQTT");
    } else {
      Serial.println("❌ Failed to send MQTT data");
    }
  }
}

void reconnectMQTT() {
  static unsigned long lastAttempt = 0;
  if (millis() - lastAttempt < 5000) return; // Don't spam reconnection attempts
  lastAttempt = millis();
  
  Serial.print("📡 Attempting MQTT connection...");
  String clientId = "ESP32WeatherStation-";
  clientId += String(random(0xffff), HEX);

  if (mqtt.connect(clientId.c_str())) {
    Serial.println(" ✅ Connected!");
    mqtt.subscribe("weatherstation/commands");
  } else {
    Serial.printf(" ❌ Failed, rc=%d. Will retry...\n", mqtt.state());
  }
}

// Blynk callback functions
BLYNK_WRITE(V_RESET_STATS) {
  if (param.asInt() == 1) {
    reading_count = 0;
    temp_avg = 0;
    humidity_avg = 0;
    max_temp = -100;
    min_temp = 100;
    Serial.println("🔄 Statistics reset via Blynk!");
    Blynk.virtualWrite(V_RESET_STATS, 0); // Reset button state
  }
}

BLYNK_CONNECTED() {
  Serial.println("✅ Blynk Connected!");
  Blynk.syncAll();  // Synchronize all virtual pin
}

BLYNK_DISCONNECTED() {
  Serial.println("❌ Blynk Disconnected!");
}
