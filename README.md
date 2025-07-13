# 🌤️ ESP32 Weather Station

A comprehensive IoT weather monitoring system built with ESP32, featuring real-time data collection, web dashboard, and cloud connectivity.

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Circuit Diagram](#circuit-diagram)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

### Core Monitoring
- **Dual Temperature Sensing**: BMP180 and DHT22 sensors for accurate readings
- **Environmental Monitoring**: Humidity, atmospheric pressure, and altitude
- **Weather Detection**: Rain sensor with threshold detection
- **Light Monitoring**: Ambient light level measurement
- **Statistical Analysis**: Min/max/average temperature tracking

### Connectivity & Integration
- **WiFi Connectivity**: Automatic connection with fallback to AP mode
- **Blynk Integration**: Real-time mobile app monitoring
- **MQTT Support**: Publish data to IoT platforms
- **Web Dashboard**: Beautiful responsive interface
- **REST API**: JSON endpoints for data access

### Advanced Features
- **Real-time Updates**: Live data streaming every 3 seconds
- **Historical Charts**: Temperature and pressure trend visualization
- **Weather Alerts**: Automatic threshold-based notifications
- **Data Export**: CSV/JSON export functionality
- **Weather Forecasting**: Basic trend-based predictions

## 🛠️ Hardware Requirements

### Main Components
- **ESP32 Development Board** (DevKit V1 or similar)
- **BMP180 Barometric Pressure Sensor**
- **DHT22 Temperature & Humidity Sensor**
- **LDR (Light Dependent Resistor)**
- **Rain Sensor Module**
- **Breadboard and Jumper Wires**
- **10kΩ Resistor** (for LDR)

### Pin Connections

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| DHT22 Data | GPIO 4 | Digital pin with 10kΩ pull-up |
| BMP180 SDA | GPIO 21 | I2C Data |
| BMP180 SCL | GPIO 22 | I2C Clock |
| LDR Signal | GPIO 34 | Analog input |
| Rain Sensor | GPIO 32 | Analog input |
| Power | 3.3V/5V | Depending on sensor |
| Ground | GND | Common ground |

## 💻 Software Requirements

### Development Environment
- **Arduino IDE** (v1.8.19 or later)
- **ESP32 Board Package** (v2.0.0 or later)

### Required Libraries
```bash
# Install through Arduino IDE Library Manager
Blynk by Volodymyr Shymanskyy (v1.0.1)
Adafruit BMP085 Library (v1.2.2)
DHT sensor library by Adafruit (v1.4.4)
PubSubClient by Nick O'Leary (v2.8.0)
ArduinoJson by Benoit Blanchon (v6.19.4)
```

### Installation Commands
```bash
# Through Arduino IDE:
# Tools → Manage Libraries → Search and install each library above

# Or using Arduino CLI:
arduino-cli lib install "Blynk"
arduino-cli lib install "Adafruit BMP085 Library"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "PubSubClient"
arduino-cli lib install "ArduinoJson"
```

## 🔧 Installation

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/esp32-weather-station.git
cd esp32-weather-station
```

### 2. Hardware Setup
1. Connect components according to the pin diagram above
2. Ensure proper power supply (3.3V for sensors)
3. Double-check I2C connections for BMP180

### 3. Software Configuration
1. Open `esp32_weather_station.ino` in Arduino IDE
2. Configure your settings in the code:

```cpp
// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Blynk Configuration
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

// MQTT Configuration (optional)
const char* mqtt_server = "broker.hivemq.com";
```

### 4. Upload Code
1. Select your ESP32 board: `Tools → Board → ESP32 Dev Module`
2. Select correct port: `Tools → Port → COM_X` (Windows) or `/dev/ttyUSB0` (Linux)
3. Click Upload button

## ⚙️ Configuration

### Blynk Setup
1. Download Blynk app from [App Store](https://apps.apple.com/app/blynk-control-arduino-raspberry/id808760481) or [Google Play](https://play.google.com/store/apps/details?id=cc.blynk)
2. Create new project, select ESP32 as device
3. Copy Auth Token to your code
4. Add widgets for virtual pins V0-V15

### Web Dashboard Setup
1. Find ESP32 IP address in Serial Monitor
2. Update `ESP32_IP` variable in `web_dashboard.html`:
```javascript
const ESP32_IP = "192.168.1.100"; // Replace with your ESP32 IP
```
3. Open `web_dashboard.html` in web browser

### MQTT Configuration (Optional)
- Server: `broker.hivemq.com` (or your preferred broker)
- Topic: `weatherstation/data`
- Port: 1883 (standard MQTT port)

## 📱 Usage

### Web Dashboard
1. Connect to ESP32 WiFi network or ensure both devices are on same network
2. Open browser and navigate to ESP32 IP address
3. View real-time data, charts, and export options

### Mobile App (Blynk)
1. Open Blynk app
2. Play your project
3. Monitor sensors in real-time
4. Receive push notifications for alerts

### API Endpoints
- `GET /` - Main dashboard page
- `GET /data` - JSON sensor data
- `POST /reset` - Reset statistics (future feature)

## 📊 API Documentation

### GET /data
Returns current sensor readings in JSON format.

**Response Example:**
```json
{
  "temperature_bmp": 23.45,
  "temperature_dht": 24.12,
  "humidity": 65.30,
  "pressure": 1013.25,
  "light": 75,
  "rain": 0,
  "rain_detected": false,
  "altitude": 123.45,
  "temp_avg": 23.78,
  "temp_max": 25.60,
  "temp_min": 22.10,
  "readings": 150,
  "timestamp": 1234567890
}
```

### MQTT Topics
- **Publish**: `weatherstation/data` - Sensor data every 30 seconds
- **Subscribe**: `weatherstation/commands` - Control commands

## 🔧 Troubleshooting

### Common Issues

#### WiFi Connection Problems
```
❌ WiFi connection failed!
```
**Solution**: 
- Check SSID and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Move closer to router

#### Sensor Reading Errors
```
❌ Could not find BMP180 sensor!
```
**Solution**:
- Check I2C connections (SDA=21, SCL=22)
- Verify sensor power supply (3.3V)
- Test with I2C scanner code

#### Blynk Connection Issues
```
❌ Blynk connection failed!
```
**Solution**:
- Verify Auth Token
- Check internet connection
- Ensure Blynk server is accessible

#### Web Dashboard Not Loading
**Solution**:
- Find correct ESP32 IP address in Serial Monitor
- Update IP in HTML file
- Check if ESP32 web server is running

### Debug Mode
Enable detailed logging by adding to setup():
```cpp
Serial.setDebugOutput(true);
```

## 🤝 Contributing

We welcome contributions! Here's how you can help:

### Development Setup
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Areas for Contribution
- Additional sensor support
- Enhanced web dashboard features
- Mobile app improvements
- Documentation updates
- Bug fixes and optimizations

### Code Style
- Follow Arduino coding standards
- Use meaningful variable names
- Add comments for complex logic
- Test on actual hardware

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Blynk](https://blynk.io/) for IoT platform
- [Adafruit](https://www.adafruit.com/) for sensor libraries
- [ESP32 Community](https://github.com/espressif/arduino-esp32) for Arduino support
- All contributors and testers

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/esp32-weather-station/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/esp32-weather-station/discussions)
- **Wiki**: [Project Wiki](https://github.com/yourusername/esp32-weather-station/wiki)

---

⭐ **If you find this project helpful, please give it a star!** ⭐

Made with ❤️ by [Your Name]