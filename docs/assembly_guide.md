# CanSat Assembly Instructions

## Overview

This guide walks you through building a complete CanSat that fits in a standard soda can (66mm diameter × 115mm height).

---

## Step 1: Prepare the Electronics

### 1.1 Test Each Component

Before soldering, test each component on a breadboard:

```cpp
// Quick I2C Scanner - Upload to check sensors
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);
  Serial.println("I2C Scanner");
}

void loop() {
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found: 0x");
      Serial.println(addr, HEX);
    }
  }
  delay(5000);
}
```

Expected addresses:
- 0x68 = MPU6050
- 0x1E = HMC5883L
- 0x5A = MPR121

### 1.2 Solder Header Pins

If your ESP8266 doesn't have headers:
1. Place headers in breadboard for alignment
2. Position ESP8266 on top
3. Solder each pin carefully
4. Check for cold joints

---

## Step 2: Build the Stack

### Layout (Bottom to Top)

```
    ┌─────────────────┐
    │   Parachute     │  ◄── Top cap
    │═══════════════════│
    │   Servo Mount   │
    ├─────────────────┤
    │                 │
    │   ESP8266       │  ◄── Main board
    │   + Sensors     │
    │                 │
    ├─────────────────┤
    │   Battery       │  ◄── 18650 or LiPo
    │   + Charging    │
    ├─────────────────┤
    │   Ultrasonic    │  ◄── Facing down
    └─────────────────┘
```

### 2.1 Sensor Board Assembly

On a 4cm × 4cm perfboard:

```
    ┌──────────────────────────┐
    │  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○  │
    │  ┌──────┐    ┌──────┐   │
    │  │ESP826│    │MPU   │   │
    │  │6 Mini│    │6050  │   │
    │  └──────┘    └──────┘   │
    │                         │
    │  ┌──────┐    ┌──────┐   │
    │  │HMC   │    │DHT11 │   │
    │  │5883L │    │      │   │
    │  └──────┘    └──────┘   │
    │  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○  │
    └──────────────────────────┘
         Power + I2C traces
```

---

## Step 3: Wire the Connections

### 3.1 Power Bus

Create a power distribution:

```
Battery (+) ────┬──► ESP8266 VIN
                │
                ├──► MT3608 IN+ ──► 5V OUT ──┬──► Servo VCC
                │                            └──► Ultrasonic VCC
                │
                └──► TP4056 B+ (charging)

Battery (-) ────┬──► ESP8266 GND
                ├──► All sensor GND
                └──► Common Ground
```

### 3.2 I2C Bus

Connect all I2C devices in parallel:

```
ESP8266 D1 (SCL) ──────┬── MPU6050 SCL
                       ├── HMC5883L SCL
                       └── (optional) BMP180 SCL

ESP8266 D2 (SDA) ──────┬── MPU6050 SDA
                       ├── HMC5883L SDA
                       └── (optional) BMP180 SDA

Add 4.7kΩ pull-up resistors from SCL→3.3V and SDA→3.3V
```

### 3.3 Other Connections

```
ESP8266 D4 ──────────► Servo Signal (orange wire)
ESP8266 D5 ──────────► Ultrasonic TRIG
ESP8266 D6 ◄──[divider]── Ultrasonic ECHO (see wiring_diagram.md)
ESP8266 D8 ──────────► DHT11 Data (with 10kΩ pull-up)
ESP8266 D7 ──[330Ω]──► LED (+)
```

---

## Step 4: Build the Chassis

### Option A: PVC Pipe (Easiest)

1. Get 65mm PVC pipe
2. Cut to 110mm length
3. 3D print or carve end caps
4. Drill holes for ultrasonic sensor
5. Add slots for parachute release

### Option B: 3D Printed

Print in sections:
- Bottom cap (with ultrasonic mount)
- Main body (cylinder with internal rails)  
- Top cap (with parachute attachment)
- Servo mount bracket

### Mounting Components

```
Use standoffs or hot glue:

    ┌──────────────────┐
    │    ╔═══════╗     │
    │    ║ Board ║ ◄───┼── Nylon standoffs (M3)
    │    ╚═══════╝     │
    │   ┌───────────┐  │
    │   │  Battery  │ ◄┼── Foam padding + velcro
    │   └───────────┘  │
    └──────────────────┘
```

---

## Step 5: Parachute Mechanism

### Simple Servo Release

```
         Parachute strings
              ║
        ┌─────╨─────┐
        │   Ring    │ ◄── Attached to parachute
        └─────┬─────┘
              │
    ┌─────────┴─────────┐
    │   Release Hook    │ ◄── Attached to servo horn
    │        ╱          │
    │    ___╱___        │
    │   │ SERVO │       │
    │   └───────┘       │
    └───────────────────┘
```

**How it works:**
1. Servo at 0° = Hook holds ring closed
2. Servo at 90° = Hook rotates, releases ring
3. Parachute deploys from spring or gravity

### Build the Release

1. Bend a paper clip into a hook shape
2. Attach to servo horn with hot glue
3. Use a keyring as the release ring
4. Test manually before coding

---

## Step 6: Upload Firmware

### Arduino IDE Setup

1. **Install ESP8266 Board**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to Tools → Board → Board Manager
   - Search "ESP8266" and install

2. **Install Libraries**
   - Sketch → Include Library → Manage Libraries
   - Install: `ESP8266WiFi`, `ESP8266WebServer`, `Wire`, `Servo`

3. **Upload Code**
   - Open `cansat_firmware.ino`
   - Select board: NodeMCU 1.0 or Wemos D1 Mini
   - Select port
   - Click Upload

---

## Step 7: Test the System

### Ground Test Checklist

- [ ] Power on ESP8266
- [ ] Check serial monitor (115200 baud)
- [ ] See I2C devices detected
- [ ] Connect phone to "CanSat-Telemetry" WiFi
- [ ] Open http://192.168.4.1 in browser
- [ ] See live sensor data
- [ ] Test "Deploy" button
- [ ] Verify servo moves
- [ ] Test ultrasonic readings (hold at various heights)

### Drop Test

1. Connect to WiFi ground station
2. Start data recording
3. Drop from 2-3 meters onto soft surface
4. Check data shows proper readings
5. Verify parachute would deploy at set altitude

---

## Safety Notes

⚠️ **Battery Safety**
- Never short circuit Li-ion batteries
- Use protection circuit (TP4056 has it)
- Don't puncture or crush cells

⚠️ **Launch Safety**
- Check local regulations
- Use an open area
- Have recovery plan
- Never launch near airports

⚠️ **Electronics**
- Double-check polarities before powering
- Use fuses for high-current connections
- Secure all connections with hot glue
