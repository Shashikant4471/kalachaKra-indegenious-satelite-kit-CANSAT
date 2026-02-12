/*
 * Terrain Scanner v1.0 - Arduino Uno
 * 5x HC-SR04 Ultrasonic Sensors for 3D Terrain Mapping
 * 
 * Layout (bottom view, looking down):
 * 
 *    S1 (Top-L) ────── S2 (Top-R)
 *         \              /
 *          \    S0      /
 *           \  (Center)/
 *         /              \
 *    S3 (Bot-L) ────── S4 (Bot-R)
 * 
 * Board: Arduino Uno
 * Pins Used: 2-7 (TRIG + 5 ECHO) + 13 (LED)
 */

// ============== PIN DEFINITIONS ==============
#define NUM_SENSORS   5
#define TRIG_PIN      2    // Shared trigger (all sensors)
#define ECHO_PIN_0    3    // S0 - Center (straight down)
#define ECHO_PIN_1    4    // S1 - Top-Left corner
#define ECHO_PIN_2    5    // S2 - Top-Right corner
#define ECHO_PIN_3    6    // S3 - Bottom-Left corner
#define ECHO_PIN_4    7    // S4 - Bottom-Right corner
#define LED_PIN       13   // Built-in LED (heartbeat)

const int echoPins[NUM_SENSORS] = {
  ECHO_PIN_0, ECHO_PIN_1, ECHO_PIN_2,
  ECHO_PIN_3, ECHO_PIN_4
};

const char* sensorNames[] = {"Center", "Top-L", "Top-R", "Bot-L", "Bot-R"};

// Sensor positions (X, Y) for 3D visualization
// Center = (0,0), corners at ~45° angles
const float sensorX[NUM_SENSORS] = { 0.0, -1.0,  1.0, -1.0,  1.0};
const float sensorY[NUM_SENSORS] = { 0.0,  1.0,  1.0, -1.0, -1.0};

// ============== SETTINGS ==============
#define SCAN_INTERVAL  500   // Scan every 500ms

// ============== DATA ==============
float terrain[NUM_SENSORS];
float terrainMin, terrainMax, terrainVariance;
String terrainStatus = "INIT";
unsigned long scanCount = 0;


// ============== READ SINGLE SENSOR ==============
float readSingleUltrasonic(int echoPin) {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo (timeout 30ms = ~5m max range)
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) return -1;  // No echo (out of range)
  
  float distance = duration * 0.0343 / 2.0;  // Convert to cm
  return distance;
}


// ============== READ ALL 5 SENSORS ==============
void readTerrainArray() {
  float sum = 0;
  float sumSq = 0;
  int validCount = 0;
  float minDist = 9999;
  float maxDist = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    float dist = readSingleUltrasonic(echoPins[i]);
    terrain[i] = dist;
    
    if (dist > 0) {  // Valid reading
      sum += dist;
      sumSq += dist * dist;
      validCount++;
      if (dist < minDist) minDist = dist;
      if (dist > maxDist) maxDist = dist;
    }
    
    delay(30);  // Avoid cross-talk between sensors
  }
  
  terrainMin = (validCount > 0) ? minDist : 0;
  terrainMax = (validCount > 0) ? maxDist : 0;
  
  // Calculate variance (measure of unevenness)
  if (validCount > 1) {
    float mean = sum / validCount;
    terrainVariance = (sumSq / validCount) - (mean * mean);
  } else {
    terrainVariance = 0;
  }
  
  // Classify terrain
  analyzeTerrain(validCount);
}


// ============== TERRAIN ANALYSIS ==============
void analyzeTerrain(int validCount) {
  float range = terrainMax - terrainMin;
  int outOfRange = NUM_SENSORS - validCount;
  
  if (outOfRange >= 3) {
    terrainStatus = "NO_GROUND";
  } else if (range > 50) {
    terrainStatus = "HAZARD!";
  } else if (range > 20) {
    terrainStatus = "UNEVEN";
  } else if (validCount > 0) {
    terrainStatus = "FLAT-SAFE";
  } else {
    terrainStatus = "NO_DATA";
  }
}


// ============== OUTPUT ==============
void printTerrainJSON() {
  // JSON format for Python terrain viewer
  Serial.print(F("TERRAIN:"));
  Serial.print(F("{\"s0\":"));
  Serial.print(terrain[0], 1);
  Serial.print(F(",\"s1\":"));
  Serial.print(terrain[1], 1);
  Serial.print(F(",\"s2\":"));
  Serial.print(terrain[2], 1);
  Serial.print(F(",\"s3\":"));
  Serial.print(terrain[3], 1);
  Serial.print(F(",\"s4\":"));
  Serial.print(terrain[4], 1);
  Serial.print(F(",\"min\":"));
  Serial.print(terrainMin, 1);
  Serial.print(F(",\"max\":"));
  Serial.print(terrainMax, 1);
  Serial.print(F(",\"var\":"));
  Serial.print(terrainVariance, 1);
  Serial.print(F(",\"status\":\""));
  Serial.print(terrainStatus);
  Serial.println(F("\"}"));
}

void printHumanReadable() {
  Serial.println(F("════════════════════════════════════════════════"));
  Serial.print(F("🗺️  TERRAIN SCAN #"));
  Serial.println(scanCount);
  Serial.println(F("────────────────────────────────────────────────"));
  
  // Visual layout matching physical arrangement
  Serial.print(F("    "));
  Serial.print(terrain[1] >= 0 ? String(terrain[1], 0) : "---");
  Serial.print(F("cm ──────── "));
  Serial.print(terrain[2] >= 0 ? String(terrain[2], 0) : "---");
  Serial.println(F("cm"));
  
  Serial.print(F("        \\    "));
  Serial.print(terrain[0] >= 0 ? String(terrain[0], 0) : "---");
  Serial.println(F("cm   /"));
  
  Serial.println(F("         \\  (Center) /"));
  
  Serial.print(F("    "));
  Serial.print(terrain[3] >= 0 ? String(terrain[3], 0) : "---");
  Serial.print(F("cm ──────── "));
  Serial.print(terrain[4] >= 0 ? String(terrain[4], 0) : "---");
  Serial.println(F("cm"));
  
  Serial.println(F("────────────────────────────────────────────────"));
  Serial.print(F("  Min: "));
  Serial.print(terrainMin, 1);
  Serial.print(F("cm | Max: "));
  Serial.print(terrainMax, 1);
  Serial.print(F("cm | Range: "));
  Serial.print(terrainMax - terrainMin, 1);
  Serial.println(F("cm"));
  
  Serial.print(F("  Status: "));
  Serial.println(terrainStatus);
  Serial.println(F("════════════════════════════════════════════════\n"));
}


// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  
  Serial.println(F("\n\n================================"));
  Serial.println(F("  Terrain Scanner v1.0 - Uno   "));
  Serial.println(F("  5x HC-SR04 Ultrasonic Array  "));
  Serial.println(F("================================\n"));
  
  // Setup pins
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(echoPins[i], INPUT);
    terrain[i] = 0;
  }
  
  Serial.println(F("[OK] TRIG pin: 2 (shared)"));
  Serial.println(F("[OK] ECHO pins: 3, 4, 5, 6, 7"));
  Serial.println(F("[OK] 5 sensors initialized"));
  Serial.println(F("[OK] Layout: Center + 4 corners"));
  
  Serial.println(F("\n================================"));
  Serial.println(F("     SCANNING STARTED!         "));
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
  static unsigned long lastScan = 0;
  
  if (millis() - lastScan >= SCAN_INTERVAL) {
    lastScan = millis();
    scanCount++;
    
    // Read all 5 sensors
    readTerrainArray();
    
    // Output data
    printTerrainJSON();       // For Python viewer
    printHumanReadable();     // For Serial Monitor
    
    // Heartbeat LED
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}
