/*
 * CanSat Firmware v2.0 - Arduino Mega/Uno
 * Satellite in a Coke Can + Terrain Mapping
 * 
 * Components:
 * - Arduino Mega 2560 (or Uno)
 * - MPU6050 Accelerometer/Gyro (I2C)
 * - HW-246 / QMC5883L Magnetometer (I2C)
 * - DHT11/22 Temperature & Humidity
 * - NEO-6M GPS Module (Serial1 on Mega, SoftwareSerial on Uno)
 * - 6x HC-SR04 Ultrasonic Sensors (terrain mapping)
 * - LED Status Indicator
 * 
 * Output: Serial Monitor (115200 baud)
 * 
 * Board: Arduino Mega 2560 (or Uno)
 */

#include <Wire.h>
#include <DHT.h>

// ============== BOARD SELECTION ==============
// Uncomment ONE of these:
#define USE_MEGA   // Arduino Mega (uses Serial1 for GPS)
// #define USE_UNO    // Arduino Uno (uses SoftwareSerial for GPS)

#ifdef USE_UNO
  #include <SoftwareSerial.h>
  #define GPS_RX_PIN  4  // GPS TX → Arduino pin 4
  #define GPS_TX_PIN  5  // Not used but needed for SoftwareSerial
  SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
#endif

// ============== PIN DEFINITIONS ==============
#define DHT_PIN       7    // DHT11 data pin
#define DHT_TYPE      DHT11
#define LED_PIN       13   // Built-in LED (or external)

// I2C Pins: Mega = SDA(20), SCL(21) | Uno = SDA(A4), SCL(A5)
// These are automatic, no need to define

// ============== ULTRASONIC TERRAIN MAPPING ==============
#define NUM_SENSORS   6
#define TRIG_PIN      22   // Shared trigger (all sensors)
#define ECHO_PIN_0    23   // S0 - Center (straight down)
#define ECHO_PIN_1    25   // S1 - Left (25° tilt)
#define ECHO_PIN_2    27   // S2 - Front-Left (25° tilt)
#define ECHO_PIN_3    29   // S3 - Front-Right (25° tilt)
#define ECHO_PIN_4    31   // S4 - Right (25° tilt)
#define ECHO_PIN_5    33   // S5 - Back (25° tilt)

const int echoPins[NUM_SENSORS] = {ECHO_PIN_0, ECHO_PIN_1, ECHO_PIN_2, ECHO_PIN_3, ECHO_PIN_4, ECHO_PIN_5};

// Sensor positions (X, Y offsets in cm at ground level, for 25° tilt at 100cm height)
// These represent the relative scan positions on the ground
const float sensorX[NUM_SENSORS] = { 0.0, -46.6,  -23.3,  23.3,  46.6,   0.0};
const float sensorY[NUM_SENSORS] = { 0.0,   0.0,   40.4,  40.4,   0.0, -46.6};

// ============== I2C ADDRESSES ==============
#define MPU6050_ADDR     0x68
#define QMC5883L_ADDR    0x0D

// ============== SETTINGS ==============
#define TELEMETRY_INTERVAL   500  // Print every 500ms (readable in Serial Monitor)
#define TERRAIN_INTERVAL    1000  // Terrain scan every 1 second

// ============== SENSOR DATA ==============
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
  
  // GPS
  float latitude;
  float longitude;
  float gpsAltitude;
  float gpsSpeed;
  int gpsSatellites;
  bool gpsValid;
  
  // Terrain Mapping (6 ultrasonic sensors)
  float terrain[NUM_SENSORS];     // Distance in cm for each sensor
  float terrainMin;               // Closest point
  float terrainMax;               // Farthest point
  float terrainVariance;          // How uneven the surface is
  String terrainStatus;           // FLAT, UNEVEN, HAZARD
  
  // System
  unsigned long missionTime;
  String status;
};

SensorData telemetry;

// ============== DHT SENSOR ==============
DHT dht(DHT_PIN, DHT_TYPE);


// ============== MPU6050 ==============
void initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);  // Power management register
  Wire.write(0x00);  // Wake up
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println(F("[OK] MPU6050 initialized"));
  } else {
    Serial.println(F("[ERR] MPU6050 not found! Check wiring."));
  }
}

void readMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)6, (uint8_t)true);
  
  int16_t rawX = Wire.read() << 8 | Wire.read();
  int16_t rawY = Wire.read() << 8 | Wire.read();
  int16_t rawZ = Wire.read() << 8 | Wire.read();
  
  telemetry.accelX = rawX / 16384.0;
  telemetry.accelY = rawY / 16384.0;
  telemetry.accelZ = rawZ / 16384.0;
  
  telemetry.roll = atan2(telemetry.accelY, telemetry.accelZ) * 180.0 / PI;
  telemetry.pitch = atan2(-telemetry.accelX, 
    sqrt(telemetry.accelY * telemetry.accelY + telemetry.accelZ * telemetry.accelZ)) * 180.0 / PI;
}


// ============== QMC5883L MAGNETOMETER (HW-246) ==============
void initQMC5883L() {
  // Set/Reset period
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x0B);
  Wire.write(0x01);
  byte error = Wire.endTransmission();
  
  if (error != 0) {
    Serial.println(F("[ERR] QMC5883L not found! Check wiring."));
    return;
  }
  
  // Control register: continuous mode, 200Hz, 8G range, OSR 512
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x09);
  Wire.write(0x1D);
  Wire.endTransmission();
  
  Serial.println(F("[OK] QMC5883L initialized"));
}

void readQMC5883L() {
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)QMC5883L_ADDR, (uint8_t)6, (uint8_t)true);
  
  // QMC5883L is little-endian
  int16_t rawX = Wire.read() | (Wire.read() << 8);
  int16_t rawY = Wire.read() | (Wire.read() << 8);
  int16_t rawZ = Wire.read() | (Wire.read() << 8);
  
  telemetry.magX = rawX;
  telemetry.magY = rawY;
  telemetry.magZ = rawZ;
  
  float heading = atan2(rawY, rawX) * 180.0 / PI;
  if (heading < 0) heading += 360;
  telemetry.heading = heading;
}


// ============== DHT SENSOR ==============
void readDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t)) telemetry.temperature = t;
  if (!isnan(h)) telemetry.humidity = h;
}


// ============== GPS (NEO-6M) ==============
void initGPS() {
  #ifdef USE_MEGA
    Serial1.begin(9600);  // Mega: Hardware Serial1 (pins 18/19)
    Serial.println(F("[OK] GPS on Serial1 (Mega pin 19=RX)"));
  #else
    gpsSerial.begin(9600);
    Serial.println(F("[OK] GPS on SoftwareSerial (pin 4)"));
  #endif
}

// Simple NMEA parser for $GPGGA sentences
void readGPS() {
  Stream* gps;
  
  #ifdef USE_MEGA
    gps = &Serial1;
  #else
    gps = &gpsSerial;
  #endif
  
  while (gps->available()) {
    static char buffer[128];
    static int idx = 0;
    
    char c = gps->read();
    
    if (c == '\n' || c == '\r') {
      buffer[idx] = '\0';
      if (idx > 6 && strncmp(buffer, "$GPGGA", 6) == 0) {
        parseGPGGA(buffer);
      }
      idx = 0;
    } else if (idx < 127) {
      buffer[idx++] = c;
    }
  }
}

void parseGPGGA(char* sentence) {
  // $GPGGA,time,lat,N/S,lon,E/W,fix,sats,hdop,alt,M,...
  char* token;
  int field = 0;
  
  token = strtok(sentence, ",");
  while (token != NULL) {
    field++;
    
    switch (field) {
      case 3: { // Latitude
        float raw = atof(token);
        int deg = (int)(raw / 100);
        float min = raw - deg * 100;
        telemetry.latitude = deg + min / 60.0;
        break;
      }
      case 4: // N/S
        if (token[0] == 'S') telemetry.latitude = -telemetry.latitude;
        break;
      case 5: { // Longitude
        float raw = atof(token);
        int deg = (int)(raw / 100);
        float min = raw - deg * 100;
        telemetry.longitude = deg + min / 60.0;
        break;
      }
      case 6: // E/W
        if (token[0] == 'W') telemetry.longitude = -telemetry.longitude;
        break;
      case 7: // Fix quality
        telemetry.gpsValid = (atoi(token) > 0);
        break;
      case 8: // Satellites
        telemetry.gpsSatellites = atoi(token);
        break;
      case 10: // Altitude
        telemetry.gpsAltitude = atof(token);
        break;
    }
    
    token = strtok(NULL, ",");
  }
}


// ============== ULTRASONIC TERRAIN MAPPING ==============
void initUltrasonicArray() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(echoPins[i], INPUT);
    telemetry.terrain[i] = 0;
  }
  
  telemetry.terrainStatus = "INIT";
  Serial.print(F("[OK] Ultrasonic array: "));
  Serial.print(NUM_SENSORS);
  Serial.println(F(" sensors initialized"));
}

// Read a single ultrasonic sensor
float readSingleUltrasonic(int echoPin) {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo (timeout 30ms = ~5m max)
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) return -1;  // No echo (out of range)
  
  float distance = duration * 0.0343 / 2.0;  // cm
  return distance;
}

// Read all 6 sensors sequentially
void readTerrainArray() {
  float sum = 0;
  float sumSq = 0;
  int validCount = 0;
  float minDist = 9999;
  float maxDist = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    float dist = readSingleUltrasonic(echoPins[i]);
    telemetry.terrain[i] = dist;
    
    if (dist > 0) {  // Valid reading
      sum += dist;
      sumSq += dist * dist;
      validCount++;
      if (dist < minDist) minDist = dist;
      if (dist > maxDist) maxDist = dist;
    }
    
    delay(30);  // Wait between readings to avoid cross-talk
  }
  
  telemetry.terrainMin = minDist;
  telemetry.terrainMax = maxDist;
  
  // Calculate variance (how uneven the surface is)
  if (validCount > 1) {
    float mean = sum / validCount;
    telemetry.terrainVariance = (sumSq / validCount) - (mean * mean);
  } else {
    telemetry.terrainVariance = 0;
  }
  
  // Classify terrain
  analyzeTerrain();
}

void analyzeTerrain() {
  float range = telemetry.terrainMax - telemetry.terrainMin;
  int outOfRange = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (telemetry.terrain[i] < 0) outOfRange++;
  }
  
  if (outOfRange >= 3) {
    telemetry.terrainStatus = "NO GROUND";
  } else if (range > 50) {
    telemetry.terrainStatus = "HAZARD!";
  } else if (range > 20) {
    telemetry.terrainStatus = "UNEVEN";
  } else {
    telemetry.terrainStatus = "FLAT-SAFE";
  }
}

void printTerrainJSON() {
  Serial.print(F("TERRAIN:"));
  Serial.print(F("{\"s0\":"));
  Serial.print(telemetry.terrain[0], 1);
  Serial.print(F(",\"s1\":"));
  Serial.print(telemetry.terrain[1], 1);
  Serial.print(F(",\"s2\":"));
  Serial.print(telemetry.terrain[2], 1);
  Serial.print(F(",\"s3\":"));
  Serial.print(telemetry.terrain[3], 1);
  Serial.print(F(",\"s4\":"));
  Serial.print(telemetry.terrain[4], 1);
  Serial.print(F(",\"s5\":"));
  Serial.print(telemetry.terrain[5], 1);
  Serial.print(F(",\"min\":"));
  Serial.print(telemetry.terrainMin, 1);
  Serial.print(F(",\"max\":"));
  Serial.print(telemetry.terrainMax, 1);
  Serial.print(F(",\"var\":"));
  Serial.print(telemetry.terrainVariance, 1);
  Serial.print(F(",\"status\":\""));
  Serial.print(telemetry.terrainStatus);
  Serial.println(F("\"}"));
}

// ============== SERIAL OUTPUT ==============
void printTelemetry() {
  Serial.println(F("════════════════════════════════════════════════"));
  
  // Mission time
  unsigned long secs = telemetry.missionTime / 1000;
  unsigned long mins = secs / 60;
  secs = secs % 60;
  Serial.print(F("⏱️  Mission Time: "));
  Serial.print(mins);
  Serial.print(F("m "));
  Serial.print(secs);
  Serial.println(F("s"));
  
  Serial.println(F("────────────────────────────────────────────────"));
  
  // Temperature & Humidity
  Serial.print(F("🌡️  Temp: "));
  Serial.print(telemetry.temperature, 1);
  Serial.print(F("°C  |  Humidity: "));
  Serial.print(telemetry.humidity, 1);
  Serial.println(F("%"));
  
  // Heading
  Serial.print(F("🧭  Heading: "));
  Serial.print(telemetry.heading, 1);
  Serial.println(F("°"));
  
  // Orientation
  Serial.print(F("📐  Roll: "));
  Serial.print(telemetry.roll, 1);
  Serial.print(F("°  |  Pitch: "));
  Serial.print(telemetry.pitch, 1);
  Serial.println(F("°"));
  
  // Acceleration
  Serial.print(F("📊  Accel X:"));
  Serial.print(telemetry.accelX, 3);
  Serial.print(F("g  Y:"));
  Serial.print(telemetry.accelY, 3);
  Serial.print(F("g  Z:"));
  Serial.print(telemetry.accelZ, 3);
  Serial.println(F("g"));
  
  // GPS
  Serial.print(F("📡  GPS: "));
  if (telemetry.gpsValid) {
    Serial.print(telemetry.latitude, 6);
    Serial.print(F(", "));
    Serial.print(telemetry.longitude, 6);
    Serial.print(F("  Alt: "));
    Serial.print(telemetry.gpsAltitude, 1);
    Serial.print(F("m  Sats: "));
    Serial.println(telemetry.gpsSatellites);
  } else {
    Serial.println(F("Searching for satellites..."));
  }
  
  // Terrain
  Serial.print(F("🗺️  Terrain: "));
  Serial.print(telemetry.terrainStatus);
  Serial.print(F("  [S0:"));
  Serial.print(telemetry.terrain[0], 0);
  Serial.print(F(" S1:"));
  Serial.print(telemetry.terrain[1], 0);
  Serial.print(F(" S2:"));
  Serial.print(telemetry.terrain[2], 0);
  Serial.print(F(" S3:"));
  Serial.print(telemetry.terrain[3], 0);
  Serial.print(F(" S4:"));
  Serial.print(telemetry.terrain[4], 0);
  Serial.print(F(" S5:"));
  Serial.print(telemetry.terrain[5], 0);
  Serial.println(F("]cm"));
  
  Serial.println(F("════════════════════════════════════════════════\n"));
}

// JSON output (for Serial Plotter or data logging)
void printJSON() {
  Serial.print(F("{\"temp\":"));
  Serial.print(telemetry.temperature, 2);
  Serial.print(F(",\"hum\":"));
  Serial.print(telemetry.humidity, 2);
  Serial.print(F(",\"hdg\":"));
  Serial.print(telemetry.heading, 2);
  Serial.print(F(",\"roll\":"));
  Serial.print(telemetry.roll, 2);
  Serial.print(F(",\"pitch\":"));
  Serial.print(telemetry.pitch, 2);
  Serial.print(F(",\"ax\":"));
  Serial.print(telemetry.accelX, 3);
  Serial.print(F(",\"ay\":"));
  Serial.print(telemetry.accelY, 3);
  Serial.print(F(",\"az\":"));
  Serial.print(telemetry.accelZ, 3);
  Serial.print(F(",\"lat\":"));
  Serial.print(telemetry.latitude, 6);
  Serial.print(F(",\"lon\":"));
  Serial.print(telemetry.longitude, 6);
  Serial.print(F(",\"gpsAlt\":"));
  Serial.print(telemetry.gpsAltitude, 1);
  Serial.print(F(",\"gpsSat\":"));
  Serial.print(telemetry.gpsSatellites);
  Serial.print(F(",\"time\":"));
  Serial.print(telemetry.missionTime);
  Serial.println(F("}"));
}


// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  
  Serial.println(F("\n\n================================"));
  Serial.println(F("   CanSat v1.0 - Arduino Mega   "));
  Serial.println(F("================================\n"));
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize sensors
  initMPU6050();
  initQMC5883L();
  dht.begin();
  Serial.println(F("[OK] DHT11 initialized"));
  initGPS();
  initUltrasonicArray();
  
  telemetry.status = "READY";
  
  Serial.println(F("\n[OK] All sensors initialized!"));
  Serial.println(F("[OK] Telemetry output: Serial Monitor (115200 baud)"));
  Serial.println(F("\n================================"));
  Serial.println(F("       MISSION STARTED!         "));
  Serial.println(F("================================\n"));
  
  // Blink LED 3 times
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}


// ============== MAIN LOOP ==============
void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastTerrain = 0;
  
  // Update mission time
  telemetry.missionTime = millis();
  
  // Read GPS continuously
  readGPS();
  
  // Read terrain array at its own interval
  if (millis() - lastTerrain >= TERRAIN_INTERVAL) {
    lastTerrain = millis();
    readTerrainArray();
    printTerrainJSON();  // Sends TERRAIN:{json} for Python viewer
  }
  
  // Read sensors and print at interval
  if (millis() - lastUpdate >= TELEMETRY_INTERVAL) {
    lastUpdate = millis();
    
    // Read sensors
    readMPU6050();
    readQMC5883L();
    readDHT();
    
    // Heartbeat LED
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    
    // Print telemetry
    printTelemetry();
  }
}
