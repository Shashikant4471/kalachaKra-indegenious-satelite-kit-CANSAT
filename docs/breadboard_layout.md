# CanSat Breadboard Wiring Guide - Step by Step

## 🎯 For Beginners: Read This First!

**What is a breadboard?**
- A breadboard has holes arranged in rows and columns
- Holes in the same **row** (horizontal) are connected internally
- The **power rails** (+/−) run vertically on the sides
- Components plugged into the same row are electrically connected

```
     POWER RAILS          COMPONENT AREA           POWER RAILS
     + − + −                                        + − + −
     │ │ │ │     a b c d e   f g h i j              │ │ │ │
     ● ● ● ●  1  ● ● ● ● ●   ● ● ● ● ●  1           ● ● ● ●
     ● ● ● ●  2  ● ● ● ● ●   ● ● ● ● ●  2           ● ● ● ●
     │ │ │ │  3  ● ● ● ● ●   ● ● ● ● ●  3           │ │ │ │
     ...         (rows continue)                     ...
     
     ◄─────────── These 5 holes are connected ──────────────►
                  (a-b-c-d-e in same row)
```

---

## 📦 Your Components List

| # | Component | Quantity |
|---|-----------|----------|
| 1 | ESP8266 NodeMCU | 1 |
| 2 | MPU6050 | 1 |
| 3 | HW-246 (QMC5883L) | 1 |
| 4 | NEO-6M GPS | 1 |
| 5 | DHT11/DHT22 | 1 |
| 6 | HC-SR04 Ultrasonic | 1 |
| 7 | SG90 Servo | 1 |
| 8 | LED (any color) | 1 |
| 9 | 1K Resistor | 1 |
| 10 | 2K2 Resistor | 1 |
| 11 | 330E Resistor | 1 |
| 12 | 10K Resistor | 1 |
| 13 | Jumper Wires | Many |
| 14 | Breadboard (830 points) | 1 |

---

## 🔌 Breadboard Layout Overview

```
                    BREADBOARD TOP VIEW
    ════════════════════════════════════════════════════════════════
    
    ESP8266 is EXTERNAL (connected via jumper cables)
                    │
                    │ Jumper wires
                    ▼
    LEFT POWER RAIL                                RIGHT POWER RAIL
    [+] [−]                                              [+] [−]
     │   │                                                │   │
     │   │    Row numbers →  1  2  3  4  5 ... 25 ... 30  │   │
     │   │                   │  │  │  │  │                │   │
    ─┴───┴───────────────────┴──┴──┴──┴──┴────────────────┴───┴─
    
    COMPONENT PLACEMENT (on breadboard):
    
    ┌─────────────────────────────────────────────────────────────┐
    │  ROWS 1-4:   MPU6050 Accelerometer                          │
    │  ROWS 6-9:   HW-246 Magnetometer                            │
    │  ROWS 11-14: DHT11 Sensor (with 10K resistor)               │
    │  ROWS 16-20: Voltage Divider (1K + 2K2 for ultrasonic)      │
    │  ROWS 22-25: LED + 330E Resistor                            │
    └─────────────────────────────────────────────────────────────┘
    
    EXTERNAL (connected via jumper cables):
    - ESP8266 NodeMCU
    - HC-SR04 Ultrasonic
    - SG90 Servo
    - NEO-6M GPS
```

---

## 📍 STEP 1: Set Up Power Rails

**What you're doing:** Connecting ESP8266 power to the breadboard rails.

```
LEFT POWER RAIL
    [+3.3V] [GND]
       │      │
       ●      ●  ← Connect RED wire from ESP8266 3V3 here
       ●      ●  ← Connect BLACK wire from ESP8266 GND here
       ●      ●
       ●      ●
       │      │
    (runs down the entire left side)
```

### Connections:
| From | To | Wire Color |
|------|-----|------------|
| ESP8266 **3V3** pin | Left rail **[+]** (red line) | 🔴 Red |
| ESP8266 **GND** pin | Left rail **[−]** (blue line) | ⚫ Black |

---

## 📍 STEP 2: ESP8266 Connections (External via Jumper Cables)

**What you're doing:** The ESP8266 stays OFF the breadboard. Connect it using jumper wires.

```
    ┌──────────────────────────────────────┐
    │           ESP8266 NodeMCU            │
    │              (External)              │
    │                                      │
    │  ┌──────────────────────────────┐    │
    │  │         USB PORT             │    │  ◄── Connected to PC
    │  └──────────────────────────────┘    │
    │                                      │
    │  LEFT SIDE        RIGHT SIDE         │
    │  ─────────        ──────────         │
    │  3V3  ●──────────────────► [+] Rail  │
    │  GND  ●──────────────────► [−] Rail  │
    │  D3   ●──────────────────► DHT DATA  │
    │  D4   ●──────────────────► Servo SIG │
    │  D5   ●──────────────────► Ultras TRIG│
    │  D6   ●──────────────────► Volt Div  │
    │  D7   ●──────────────────► LED       │
    │  D8   ●──────────────────► GPS TX    │
    │       ●  D1 ─────────────► I2C SCL   │
    │       ●  D2 ─────────────► I2C SDA   │
    │  VIN  ●──────────────────► 5V Rail   │
    │                                      │
    └──────────────────────────────────────┘
```

### ESP8266 Pin Reference (for your jumper cables):

| ESP8266 Pin | Goes To | Wire Color |
|-------------|---------|------------|
| **3V3** | [+] Power Rail | 🔴 Red |
| **GND** | [−] Ground Rail | ⚫ Black |
| **VIN** | 5V Rail (for servo/ultrasonic) | 🔴 Red (thick) |
| **D1** | I2C SCL (MPU6050 + HW-246) | 🟡 Yellow |
| **D2** | I2C SDA (MPU6050 + HW-246) | 🟢 Green |
| **D3** | DHT11 DATA | 🟠 Orange |
| **D4** | Servo Signal | 🟠 Orange |
| **D5** | Ultrasonic TRIG | 🟡 Yellow |
| **D6** | Voltage Divider junction | 🔵 Blue |
| **D7** | LED (via 330E resistor) | 🟣 Purple |
| **D8** | GPS TX | ⚪ White |

---

## 📍 STEP 3: Wire the I2C Sensors (MPU6050 + HW-246)

**What you're doing:** Both sensors share the same I2C bus (D1/D2).

### Place MPU6050 at rows 1-4:
```
           a  b  c  d  e
     1    [VCC]  ←── Connect to [+] rail (3.3V)
     2    [GND]  ←── Connect to [−] rail  
     3    [SCL]  ←── Jumper wire to ESP8266 D1
     4    [SDA]  ←── Jumper wire to ESP8266 D2
     (Other pins: XDA, XCL, AD0, INT - Not connected)
```

### Place HW-246 at rows 6-9:
```
           a  b  c  d  e
     6    [VCC]  ←── Connect to [+] rail (3.3V)
     7    [GND]  ←── Connect to [−] rail  
     8    [SCL]  ←── Connect to SAME row as MPU6050 SCL (row 3)
     9    [SDA]  ←── Connect to SAME row as MPU6050 SDA (row 4)
     (DRDY pin - Not connected)
```

### I2C Bus Wiring Diagram:
```
ESP8266 D1 (jumper) ────┬───── MPU6050 SCL (row 3)
                        │
                        └───── HW-246 SCL (wire from row 8 to row 3)

ESP8266 D2 (jumper) ────┬───── MPU6050 SDA (row 4)
                        │
                        └───── HW-246 SDA (wire from row 9 to row 4)
```

### Connection Table:
| From | To | Wire Color |
|------|-----|------------|
| MPU6050 VCC | [+] Power Rail | 🔴 Red |
| MPU6050 GND | [−] Power Rail | ⚫ Black |
| MPU6050 SCL (row 3) | **ESP8266 D1** (jumper cable) | 🟡 Yellow |
| MPU6050 SDA (row 4) | **ESP8266 D2** (jumper cable) | 🟢 Green |
| HW-246 VCC | [+] Power Rail | 🔴 Red |
| HW-246 GND | [−] Power Rail | ⚫ Black |
| HW-246 SCL (row 8) | Row 3 (shared with MPU6050) | 🟡 Yellow (short wire) |
| HW-246 SDA (row 9) | Row 4 (shared with MPU6050) | 🟢 Green (short wire) |

---

## 📍 STEP 4: Wire the DHT11 Sensor (with Pull-up Resistor)

**What you're doing:** DHT needs a 10K pull-up resistor between VCC and DATA.

### Place DHT11 at rows 11-14:
```
           a  b  c  d  e
    11    [VCC]  ←── Connect to [+] rail (3.3V)
                     Also connect one end of 10K resistor here

    12    [DATA] ←── Jumper wire to ESP8266 D3
                     Also connect other end of 10K resistor here

    13    [NC]   ←── Not connected (some DHT11 have only 3 pins)

    14    [GND]  ←── Connect to [−] rail
```

### Detailed DHT11 Wiring:
```
                3.3V (+) Rail
                     │
          ┌──────────┴────────────┐
          │                       │
          │                     [10K]  ← 10K Resistor (Brown-Black-Orange)
          │                       │
        ┌─┴─┐                     │
        │VCC│ ────────────────────┤
        │DAT│ ────────────────────┴──────► to ESP8266 D3 (jumper cable)
        │ NC│
        │GND│ ──────────────────────────► to [−] Rail
        └───┘
        DHT11
        
    ** The 10K resistor connects between VCC (row 11) and DATA (row 12) **
```

### Connection Table:
| From | To | Wire Color |
|------|-----|------------|
| DHT11 VCC (row 11) | [+] Power Rail | 🔴 Red |
| DHT11 GND (row 14) | [−] Power Rail | ⚫ Black |
| DHT11 DATA (row 12) | **ESP8266 D3** (jumper cable) | 🟠 Orange |
| 10K Resistor | Between row 11 (VCC) and row 12 (DATA) | (component) |

---

## 📍 STEP 5: Wire the Ultrasonic Sensor (with Voltage Divider)

**⚠️ IMPORTANT:** The ECHO pin outputs 5V, which can damage ESP8266! We MUST use a voltage divider.

### Ultrasonic placed on right side of breadboard:
```
    HC-SR04 PINS:     VCC   TRIG   ECHO   GND
                       │      │      │      │
    To Breadboard:     │      │      │      │
                       ▼      ▼      ▼      ▼
```

### Voltage Divider Wiring (VERY IMPORTANT):
```
    HC-SR04 ECHO pin
           │
           │
          [1K]  ← 1K Resistor (Brown-Black-Red)
           │
           ├─────────────────────► to ESP8266 D6 (row 6, col a-e)
           │
         [2K2]  ← 2K2 Resistor (Red-Red-Red)
           │
           │
          GND (─) Rail
```

### On Breadboard (Example rows 40-45):
```
           a  b  c  d  e  f  g  h  i  j
    40          [ECHO]────[●]           
                            │            
    41                    [1K]  ← 1K resistor vertical
                            │
    42    [wire to D6]────[●]
                            │
    43                   [2K2]  ← 2K2 resistor vertical  
                            │
    44                    [●]────[wire to GND rail]
```

### Connection Table:
| From | To | Wire Color |
|------|-----|------------|
| HC-SR04 VCC | ESP8266 **VIN** (5V!) | 🔴 Red |
| HC-SR04 GND | [−] Power Rail | ⚫ Black |
| HC-SR04 TRIG | ESP8266 D5 (row 5, col a-e) | 🟡 Yellow |
| HC-SR04 ECHO | Through voltage divider → D6 | 🔵 Blue |
| 1K Resistor | Between ECHO and middle point | (component) |
| 2K2 Resistor | Between middle point and GND | (component) |

---

## 📍 STEP 6: Wire the Servo Motor

**Note:** Servo stays OFF the breadboard. Use jumper wires to connect.

### Servo Wire Colors:
```
    SERVO CONNECTOR
    ┌─────────────────┐
    │  🟤  🔴  🟠     │
    │ GND VCC SIG     │
    └─────────────────┘
```

### Connection Table:
| Servo Wire | Color | Connect To |
|------------|-------|------------|
| GND | 🟤 Brown/Black | [−] Ground Rail |
| VCC | 🔴 Red | ESP8266 **VIN** (5V) |
| Signal | 🟠 Orange/Yellow | ESP8266 D4 (row 4, col a-e) |

---

## 📍 STEP 7: Wire the GPS Module (NEO-6M)

**Note:** GPS module stays OFF the breadboard. Use jumper wires.

### GPS Module Pins:
```
    NEO-6M GPS
    ┌─────────────────┐
    │  VCC GND TX RX  │
    │   │   │   │  │  │
    └───┼───┼───┼──┼──┘
        │   │   │  │
        │   │   │  └── Not connected
        │   │   └───── to ESP8266 D8
        │   └───────── to [−] Rail
        └───────────── to [+] 3.3V Rail
```

### Connection Table:
| GPS Pin | Connect To |
|---------|------------|
| VCC | [+] 3.3V Power Rail |
| GND | [−] Ground Rail |
| TX | ESP8266 D8 (row 8, col a-e) |
| RX | Not connected |

---

## 📍 STEP 8: Wire the Status LED

**What you're doing:** LED with current-limiting resistor.

### LED + Resistor Wiring:
```
    ESP8266 D7 (row 7)
           │
         [330E]  ← 330 ohm resistor (Orange-Orange-Brown)
           │
         [LED+]  ← Longer leg (Anode)
           │
         [LED−]  ← Shorter leg (Cathode)
           │
          GND (−) Rail
```

### On Breadboard (Example row 50):
```
           a  b  c  d  e
    50    [wire from D7]────[●]
                              │
    51                     [330E]  ← Resistor
                              │
    52                      [+]  ← LED long leg here
                              │
    53                      [−]  ← LED short leg here
                              │
    54    [wire to GND rail]──●
```

---

## 📍 COMPLETE WIRING OVERVIEW DIAGRAM

```
    ═══════════════════════════════════════════════════════════════════
                        COMPLETE BREADBOARD LAYOUT
    ═══════════════════════════════════════════════════════════════════
    
    [+3.3V]  [GND]                                         [+5V]  [GND]
       │       │                                              │      │
       ●───────●  POWER RAILS                                 ●──────●
       │       │                                              │      │
    ───┴───────┴──────────────────────────────────────────────┴──────┴───
    
                    ┌─────────────────────┐
           USB ────►│     ESP8266         │◄──── USB to PC
                    │     NodeMCU         │
                    └─────────────────────┘
                      │ │ │ │ │   │ │ │ │
                      │ │ │ │ │   │ │ │ │
         Pins:       3V G D D D   D D D VIN
                      3 N 3 4 5   1 2 8
                      V D       S S G
                        │ │ │   C S P
                        │ │ │   L A S
                        │ │ └─► TRIG (Ultrasonic)
                        │ └───► SERVO
                        └─────► DHT11
    
    ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐
    │ MPU6050 │  │ HW-246  │  │  DHT11  │  │ HC-SR04 │
    │         │  │         │  │  +10K   │  │ +1K+2K2 │
    │VCC→3.3V │  │VCC→3.3V │  │VCC→3.3V │  │VCC→5V   │
    │GND→GND  │  │GND→GND  │  │GND→GND  │  │GND→GND  │
    │SCL→D1   │  │SCL→D1   │  │DAT→D3   │  │TRG→D5   │
    │SDA→D2   │  │SDA→D2   │  │         │  │ECH→D6** │
    └─────────┘  └─────────┘  └─────────┘  └─────────┘
                                          ** via voltage divider
    
    ┌─────────┐  ┌─────────┐  ┌─────────┐
    │  SERVO  │  │ NEO-6M  │  │   LED   │
    │         │  │  GPS    │  │  +330E  │
    │VCC→5V   │  │VCC→3.3V │  │         │
    │GND→GND  │  │GND→GND  │  │ D7→LED  │
    │SIG→D4   │  │TX→D8    │  │ LED→GND │
    └─────────┘  └─────────┘  └─────────┘
    
    ═══════════════════════════════════════════════════════════════════
```

---

## ✅ FINAL CHECKLIST

Go through this checklist BEFORE powering on:

### Power Connections:
- [ ] ESP8266 3V3 → [+] Rail (3.3V)
- [ ] ESP8266 GND → [−] Rail
- [ ] ESP8266 VIN → Servo VCC & Ultrasonic VCC (5V)

### I2C Bus (D1/D2):
- [ ] MPU6050 SCL → D1
- [ ] MPU6050 SDA → D2
- [ ] HW-246 SCL → D1 (same wire/row)
- [ ] HW-246 SDA → D2 (same wire/row)

### DHT11 (D3):
- [ ] DHT11 DATA → D3
- [ ] 10K resistor between VCC and DATA ✓

### Ultrasonic (D5/D6):
- [ ] HC-SR04 TRIG → D5
- [ ] HC-SR04 ECHO → through 1K → junction → through 2K2 → GND
- [ ] Junction point → D6 ✓

### Servo (D4):
- [ ] Servo Signal → D4
- [ ] Servo VCC → 5V (VIN)

### GPS (D8):
- [ ] GPS TX → D8

### LED (D7):
- [ ] D7 → 330E resistor → LED+ → LED− → GND ✓

### Double-Check:
- [ ] No wires touching that shouldn't
- [ ] All components facing correct direction
- [ ] USB cable ready to connect

---

## 🚀 Ready to Power On?

Once wiring is complete:
1. Double-check all connections
2. Connect USB cable to PC
3. Open Arduino IDE
4. Upload the firmware
5. Open Serial Monitor (115200 baud)
6. Check for sensor initialization messages

**Good luck with your CanSat! 🛰️**
