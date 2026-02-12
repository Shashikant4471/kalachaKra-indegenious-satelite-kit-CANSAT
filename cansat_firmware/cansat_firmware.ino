/*
 * CanSat Firmware v1.1
 * Satellite in a Coke Can
 * 
 * Components:
 * - ESP8266 (NodeMCU/Wemos D1 Mini)
 * - MPU6050 / ADXL345 Accelerometer
 * - HMC5883L / QMC5883L Magnetometer  
 * - DHT11/22 Temperature & Humidity
 * - HC-SR04 Ultrasonic Sensor
 * - GPS Module (NEO-6M or similar)
 * - Micro Servo (SG90) for parachute
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <DHT.h>

// ============== PIN DEFINITIONS ==============
#define DHT_PIN       D3   // GPIO0 - DHT11/22 data
#define DHT_TYPE      DHT11  // Change to DHT22 if you have DHT22
#define SERVO_PIN     D4   // GPIO2 - Parachute servo (disabled for now)
// D5 and D6 free (ultrasonic removed)
#define LED_PIN       D7   // GPIO13 - Status LED
#define GPS_RX        D8   // GPIO15 - GPS TX -> ESP RX
#define GPS_TX        3    // RX pin - ESP TX -> GPS RX (optional)
#define BATTERY_PIN   A0   // Analog - Battery voltage monitoring

// I2C Pins (default for ESP8266)
// SDA = D2 (GPIO4)
// SCL = D1 (GPIO5)

// ============== I2C ADDRESSES ==============
#define MPU6050_ADDR     0x68
#define HMC5883L_ADDR    0x1E
#define QMC5883L_ADDR    0x0D

// ============== WiFi SETTINGS ==============
const char* AP_SSID = "CanSat-Telemetry";
const char* AP_PASS = "cansat2024";

// ============== MISSION SETTINGS ==============
#define PARACHUTE_ALTITUDE   300    // Deploy at 3 meters (in cm)
#define SERVO_CLOSED         0      // Servo angle - locked
#define SERVO_OPEN           90     // Servo angle - deployed
#define TELEMETRY_INTERVAL   100    // Send data every 100ms

// ============== GLOBAL OBJECTS ==============
ESP8266WebServer server(80);
// Servo parachuteServo;  // DISABLED - uncomment when servo is connected
SoftwareSerial gpsSerial(GPS_RX, GPS_TX); // RX, TX

// GPS Buffer
char gpsBuffer[128];
int gpsBufferIndex = 0;

// ============== SENSOR DATA STRUCTURE ==============
struct SensorData {
  // Accelerometer
  float accelX, accelY, accelZ;
  float roll, pitch;
  
  // Magnetometer
  float magX, magY, magZ;
  float heading;
  
  // Environment
  float temperature;
  float humidity;
  
  // GPS Data
  float latitude;
  float longitude;
  float gpsAltitude;
  float gpsSpeed;
  int gpsSatellites;
  bool gpsValid;
  
  // System
  float batteryVoltage;
  unsigned long missionTime;
  bool parachuteDeployed;
  String status;
};

SensorData telemetry;

// ============== DHT SENSOR ==============
DHT dht(DHT_PIN, DHT_TYPE);

void initDHT() {
  dht.begin();
  Serial.println("[OK] DHT sensor initialized");
}

float readTemperature() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("[WARN] DHT temperature read failed!");
    return telemetry.temperature; // Return last good value
  }
  return temp;
}

float readHumidity() {
  float hum = dht.readHumidity();
  if (isnan(hum)) {
    Serial.println("[WARN] DHT humidity read failed!");
    return telemetry.humidity; // Return last good value
  }
  return hum;
}

// ============== MPU6050 ACCELEROMETER ==============
void initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0x00);  // Wake up MPU6050
  Wire.endTransmission(true);
  Serial.println("[OK] MPU6050 initialized");
}

void readMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);  // Starting register for accel data
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (size_t)6, true);
  
  int16_t rawX = Wire.read() << 8 | Wire.read();
  int16_t rawY = Wire.read() << 8 | Wire.read();
  int16_t rawZ = Wire.read() << 8 | Wire.read();
  
  // Convert to g (assuming ±2g range, 16384 LSB/g)
  telemetry.accelX = rawX / 16384.0;
  telemetry.accelY = rawY / 16384.0;
  telemetry.accelZ = rawZ / 16384.0;
  
  // Calculate roll and pitch
  telemetry.roll = atan2(telemetry.accelY, telemetry.accelZ) * 180.0 / PI;
  telemetry.pitch = atan2(-telemetry.accelX, 
    sqrt(telemetry.accelY * telemetry.accelY + telemetry.accelZ * telemetry.accelZ)) * 180.0 / PI;
}

// ============== QMC5883L MAGNETOMETER (HW-246) ==============
void initQMC5883L() {
  // Soft reset
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x0B);  // SET/RESET period register
  Wire.write(0x01);  // Recommended value
  Wire.endTransmission();
  
  // Configure: continuous mode, 200Hz, 8G range, 512 oversampling
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x09);  // Control Register 1
  Wire.write(0x1D);  // OSR=512, RNG=8G, ODR=200Hz, Mode=Continuous
  Wire.endTransmission();
  
  Serial.println("[OK] QMC5883L (HW-246) initialized");
}

void readQMC5883L() {
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x00);  // Data output register (X LSB)
  Wire.endTransmission();
  Wire.requestFrom(QMC5883L_ADDR, 6);
  
  if (Wire.available() == 6) {
    // QMC5883L sends LSB first, then MSB (opposite of HMC5883L)
    int16_t x = Wire.read() | (Wire.read() << 8);
    int16_t y = Wire.read() | (Wire.read() << 8);
    int16_t z = Wire.read() | (Wire.read() << 8);
    
    telemetry.magX = x;
    telemetry.magY = y;
    telemetry.magZ = z;
    
    // Calculate heading (compass bearing)
    telemetry.heading = atan2(y, x) * 180.0 / PI;
    if (telemetry.heading < 0) telemetry.heading += 360;
  }
}



// ============== BATTERY VOLTAGE ==============
float readBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  // Convert to voltage (with voltage divider if used)
  // Direct connection: 0-1V range on A0
  float voltage = raw * (4.2 / 1024.0); // Adjust based on your divider
  return voltage;
}

// ============== GPS MODULE ==============
void initGPS() {
  gpsSerial.begin(9600);
  telemetry.gpsValid = false;
  telemetry.gpsSatellites = 0;
  Serial.println("[OK] GPS initialized (9600 baud)");
}

// Parse NMEA sentence - extract data from $GPGGA and $GPRMC
void parseGPS(char* sentence) {
  if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
    // $GPGGA,HHMMSS.ss,LLLL.LL,N,LLLLL.LL,E,Q,SS,HDOP,ALT,M,...
    char* token = strtok(sentence, ",");
    int fieldIndex = 0;
    float lat = 0, lon = 0;
    char latDir = 'N', lonDir = 'E';
    
    while (token != NULL) {
      switch (fieldIndex) {
        case 2: // Latitude
          lat = atof(token);
          break;
        case 3: // N/S
          latDir = token[0];
          break;
        case 4: // Longitude
          lon = atof(token);
          break;
        case 5: // E/W
          lonDir = token[0];
          break;
        case 6: // Fix quality (0=invalid, 1=GPS, 2=DGPS)
          telemetry.gpsValid = (atoi(token) > 0);
          break;
        case 7: // Satellites
          telemetry.gpsSatellites = atoi(token);
          break;
        case 9: // Altitude
          telemetry.gpsAltitude = atof(token);
          break;
      }
      token = strtok(NULL, ",");
      fieldIndex++;
    }
    
    // Convert NMEA format (DDMM.MMMM) to decimal degrees
    if (telemetry.gpsValid && lat != 0) {
      int latDeg = (int)(lat / 100);
      float latMin = lat - (latDeg * 100);
      telemetry.latitude = latDeg + (latMin / 60.0);
      if (latDir == 'S') telemetry.latitude = -telemetry.latitude;
      
      int lonDeg = (int)(lon / 100);
      float lonMin = lon - (lonDeg * 100);
      telemetry.longitude = lonDeg + (lonMin / 60.0);
      if (lonDir == 'W') telemetry.longitude = -telemetry.longitude;
    }
  }
  else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
    // $GPRMC - get speed
    char* token = strtok(sentence, ",");
    int fieldIndex = 0;
    
    while (token != NULL) {
      if (fieldIndex == 7) { // Speed in knots
        telemetry.gpsSpeed = atof(token) * 1.852; // Convert to km/h
        break;
      }
      token = strtok(NULL, ",");
      fieldIndex++;
    }
  }
}

void readGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    
    if (c == '\n' || c == '\r') {
      if (gpsBufferIndex > 0) {
        gpsBuffer[gpsBufferIndex] = '\0';
        parseGPS(gpsBuffer);
        gpsBufferIndex = 0;
      }
    } else if (gpsBufferIndex < sizeof(gpsBuffer) - 1) {
      gpsBuffer[gpsBufferIndex++] = c;
    }
  }
}

// ============== PARACHUTE CONTROL ==============
void deployParachute() {
  if (!telemetry.parachuteDeployed) {
    // parachuteServo.write(SERVO_OPEN);  // DISABLED - uncomment when servo is connected
    telemetry.parachuteDeployed = true;
    telemetry.status = "PARACHUTE DEPLOYED";
    Serial.println("[!] PARACHUTE DEPLOYED!");
    
    // Flash LED rapidly
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(100);
    }
  }
}

void checkAutoDeployment() {
  // Auto-deploy based on GPS altitude during descent
  if (telemetry.gpsValid && 
      telemetry.gpsAltitude > 0 && 
      telemetry.gpsAltitude < 50 && 
      !telemetry.parachuteDeployed) {
    deployParachute();
  }
}

// ============== WEB SERVER HANDLERS ==============
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>CanSat Ground Station</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background: linear-gradient(135deg, #0c0c1e 0%, #1a1a3e 100%);
      color: #fff; 
      min-height: 100vh;
      padding: 20px;
    }
    h1 { 
      text-align: center; 
      font-size: 2em;
      background: linear-gradient(90deg, #00d4ff, #7c3aed);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-bottom: 20px;
    }
    .grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); 
      gap: 15px; 
    }
    .card { 
      background: rgba(255,255,255,0.05);
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 15px; 
      padding: 20px;
    }
    .card h3 { 
      color: #00d4ff; 
      margin-bottom: 15px;
      font-size: 0.9em;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    .value { 
      font-size: 2.5em; 
      font-weight: bold;
      background: linear-gradient(90deg, #fff, #a0a0ff);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    .unit { color: #888; font-size: 0.9em; }
    .status { 
      text-align: center; 
      padding: 15px; 
      border-radius: 10px;
      font-weight: bold;
      margin-bottom: 15px;
    }
    .status.active { background: linear-gradient(90deg, #00c853, #00e676); color: #000; }
    .status.deployed { background: linear-gradient(90deg, #ff6d00, #ff9100); color: #000; }
    .btn {
      width: 100%;
      padding: 15px;
      border: none;
      border-radius: 10px;
      font-size: 1em;
      font-weight: bold;
      cursor: pointer;
      margin-top: 10px;
      transition: transform 0.2s;
    }
    .btn:hover { transform: scale(1.02); }
    .btn-deploy { background: linear-gradient(90deg, #ff1744, #ff5252); color: #fff; }
    .btn-reset { background: linear-gradient(90deg, #00bcd4, #00e5ff); color: #000; }
    .compass {
      width: 120px; height: 120px;
      border-radius: 50%;
      border: 3px solid #00d4ff;
      margin: 0 auto;
      position: relative;
    }
    .compass-needle {
      position: absolute;
      width: 4px; height: 50px;
      background: linear-gradient(to top, #ff1744 50%, #fff 50%);
      left: 50%; top: 10px;
      transform-origin: bottom center;
      border-radius: 2px;
    }
  </style>
</head>
<body>
  <h1>🛰️ CanSat Ground Station</h1>
  
  <div id="status" class="status active">MISSION ACTIVE - TELEMETRY OK</div>
  
  <div class="grid">
    <div class="card">
      <h3>🌡️ Temperature</h3>
      <div class="value" id="temp">--</div>
      <div class="unit">°C</div>
    </div>
    <div class="card">
      <h3>💧 Humidity</h3>
      <div class="value" id="humidity">--</div>
      <div class="unit">%</div>
    </div>
    <div class="card">
      <h3>📏 Altitude (Ultrasonic)</h3>
      <div class="value" id="altitude">--</div>
      <div class="unit">cm</div>
    </div>
    <div class="card">
      <h3>🧭 Heading</h3>
      <div class="compass">
        <div class="compass-needle" id="needle"></div>
      </div>
      <div style="text-align:center; margin-top:10px;">
        <span class="value" id="heading">--</span>
        <span class="unit">°</span>
      </div>
    </div>
    <div class="card">
      <h3>📐 Roll / Pitch</h3>
      <div class="value" id="roll">--</div>
      <div class="unit">° roll</div>
      <div class="value" id="pitch">--</div>
      <div class="unit">° pitch</div>
    </div>
    <div class="card">
      <h3>⚡ Acceleration</h3>
      <div>X: <span id="ax">--</span>g</div>
      <div>Y: <span id="ay">--</span>g</div>
      <div>Z: <span id="az">--</span>g</div>
    </div>
    <div class="card">
      <h3>⏱️ Mission Time</h3>
      <div class="value" id="time">00:00</div>
    </div>
    <div class="card">
      <h3>🪂 Parachute Control</h3>
      <button class="btn btn-deploy" onclick="deploy()">DEPLOY NOW</button>
      <button class="btn btn-reset" onclick="reset()">RESET</button>
    </div>
    <div class="card">
      <h3>📍 GPS Position</h3>
      <div id="gpsStatus" style="color:#888;margin-bottom:10px;">Acquiring...</div>
      <div>Lat: <span id="lat">--</span>°</div>
      <div>Lon: <span id="lon">--</span>°</div>
      <div style="margin-top:10px;"><a id="mapLink" href="#" target="_blank" style="color:#00d4ff;">Open in Maps</a></div>
    </div>
    <div class="card">
      <h3>🛰️ GPS Altitude</h3>
      <div class="value" id="gpsAlt">--</div>
      <div class="unit">meters</div>
      <div style="margin-top:10px;">Speed: <span id="gpsSpd">--</span> km/h</div>
      <div>Satellites: <span id="gpsSat">--</span></div>
    </div>
  </div>
  
  <script>
    function update() {
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById('temp').innerText = d.temp.toFixed(1);
        document.getElementById('humidity').innerText = d.hum.toFixed(1);
        document.getElementById('altitude').innerText = d.dist.toFixed(0);
        document.getElementById('heading').innerText = d.hdg.toFixed(0);
        document.getElementById('roll').innerText = d.roll.toFixed(1);
        document.getElementById('pitch').innerText = d.pitch.toFixed(1);
        document.getElementById('ax').innerText = d.ax.toFixed(2);
        document.getElementById('ay').innerText = d.ay.toFixed(2);
        document.getElementById('az').innerText = d.az.toFixed(2);
        document.getElementById('needle').style.transform = 'rotate(' + d.hdg + 'deg)';
        
        let mins = Math.floor(d.time / 60000);
        let secs = Math.floor((d.time % 60000) / 1000);
        document.getElementById('time').innerText = 
          (mins < 10 ? '0' : '') + mins + ':' + (secs < 10 ? '0' : '') + secs;
        
        if (d.chute) {
          document.getElementById('status').className = 'status deployed';
          document.getElementById('status').innerText = '🪂 PARACHUTE DEPLOYED';
        }
        
        // GPS data
        if (d.gpsValid) {
          document.getElementById('gpsStatus').innerText = '✓ GPS Fix';
          document.getElementById('gpsStatus').style.color = '#00e676';
          document.getElementById('lat').innerText = d.lat.toFixed(6);
          document.getElementById('lon').innerText = d.lon.toFixed(6);
          document.getElementById('gpsAlt').innerText = d.gpsAlt.toFixed(1);
          document.getElementById('gpsSpd').innerText = d.gpsSpd.toFixed(1);
          document.getElementById('gpsSat').innerText = d.gpsSat;
          document.getElementById('mapLink').href = 'https://maps.google.com/?q=' + d.lat + ',' + d.lon;
        } else {
          document.getElementById('gpsStatus').innerText = 'Searching (' + d.gpsSat + ' sats)...';
          document.getElementById('gpsStatus').style.color = '#ff9100';
        }
      });
    }
    
    function deploy() { 
      if(confirm('Deploy parachute?')) fetch('/deploy'); 
    }
    function reset() { 
      fetch('/reset'); 
      location.reload(); 
    }
    
    setInterval(update, 200);
    update();
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temp\":" + String(telemetry.temperature, 2) + ",";
  json += "\"hum\":" + String(telemetry.humidity, 2) + ",";
  json += "\"hdg\":" + String(telemetry.heading, 2) + ",";
  json += "\"roll\":" + String(telemetry.roll, 2) + ",";
  json += "\"pitch\":" + String(telemetry.pitch, 2) + ",";
  json += "\"ax\":" + String(telemetry.accelX, 3) + ",";
  json += "\"ay\":" + String(telemetry.accelY, 3) + ",";
  json += "\"az\":" + String(telemetry.accelZ, 3) + ",";
  json += "\"time\":" + String(telemetry.missionTime) + ",";
  json += "\"chute\":" + String(telemetry.parachuteDeployed ? "true" : "false") + ",";
  json += "\"lat\":" + String(telemetry.latitude, 6) + ",";
  json += "\"lon\":" + String(telemetry.longitude, 6) + ",";
  json += "\"gpsAlt\":" + String(telemetry.gpsAltitude, 1) + ",";
  json += "\"gpsSpd\":" + String(telemetry.gpsSpeed, 1) + ",";
  json += "\"gpsSat\":" + String(telemetry.gpsSatellites) + ",";
  json += "\"gpsValid\":" + String(telemetry.gpsValid ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleDeploy() {
  deployParachute();
  server.send(200, "text/plain", "OK");
}

void handleReset() {
  telemetry.parachuteDeployed = false;
  // parachuteServo.write(SERVO_CLOSED);  // DISABLED - uncomment when servo is connected
  telemetry.status = "READY";
  server.send(200, "text/plain", "OK");
}

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n================================");
  Serial.println("   CanSat v1.1 - Initializing   ");
  Serial.println("================================\n");
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin(D2, D1); // SDA, SCL
  
  // Initialize sensors
  initMPU6050();
  initQMC5883L();
  initDHT();
  initGPS();
  
  // Initialize servo - DISABLED, uncomment when servo is connected
  // parachuteServo.attach(SERVO_PIN);
  // parachuteServo.write(SERVO_CLOSED);
  telemetry.parachuteDeployed = false;
  
  // Create WiFi Access Point
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[OK] WiFi AP created: ");
  Serial.println(AP_SSID);
  Serial.print("[OK] IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Setup web server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/deploy", handleDeploy);
  server.on("/reset", handleReset);
  server.begin();
  
  Serial.println("\n[OK] Ground Station ready!");
  Serial.println("Connect to WiFi: " + String(AP_SSID));
  Serial.println("Open browser: http://192.168.4.1");
  Serial.println("\n================================");
  Serial.println("       MISSION STARTED!         ");
  Serial.println("================================\n");
  
  // Blink LED to indicate ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  telemetry.status = "ACTIVE";
}

// ============== MAIN LOOP ==============
void loop() {
  static unsigned long lastUpdate = 0;
  
  // Update mission time
  telemetry.missionTime = millis();
  
  // Read sensors at interval
  if (millis() - lastUpdate >= TELEMETRY_INTERVAL) {
    lastUpdate = millis();
    
    // Read all sensors
    readMPU6050();
    readQMC5883L();
    telemetry.temperature = readTemperature();
    telemetry.humidity = readHumidity();
    telemetry.batteryVoltage = readBatteryVoltage();
    
    // Read GPS (runs every loop, but only parses when data available)
    readGPS();
    
    // Check for auto deployment
    checkAutoDeployment();
    
    // Heartbeat LED
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    
    // Serial telemetry
    Serial.print("T:");
    Serial.print(telemetry.missionTime / 1000.0, 1);
    Serial.print("s | Temp:");
    Serial.print(telemetry.temperature);
    Serial.print("C | Hum:");
    Serial.print(telemetry.humidity);
    Serial.print("% | Hdg:");
    Serial.print(telemetry.heading);
    Serial.println("°");
  }
  
  // Handle web requests
  server.handleClient();
}
