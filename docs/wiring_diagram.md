# CanSat Wiring Guide

## Quick Reference Pinout

```
ESP8266 NodeMCU/Wemos D1 Mini
═══════════════════════════════════════════════════════════

                    ┌─────────────┐
                    │   USB PORT  │
                    └──────┬──────┘
              ┌────────────┴────────────┐
        3.3V ─┤ 3V3              VIN    ├─ 5V (Battery in)
         GND ─┤ GND              GND    ├─ GND
   [DHT11]   ─┤ D3 (GPIO0)       D1     ├─ SCL [I2C Clock]
   [SERVO]   ─┤ D4 (GPIO2)       D2     ├─ SDA [I2C Data]
   [US-TRIG] ─┤ D5 (GPIO14)      D5     ├─
   [US-ECHO] ─┤ D6 (GPIO12)      D6     ├─
   [LED]     ─┤ D7 (GPIO13)      D7     ├─
   [GPS-TX]  ─┤ D8 (GPIO15)      D8     ├─ [GPS RX from module]
   [NC]      ─┤ RX               TX     ├─ [NC]
   [BATTERY] ─┤ A0               RST    ├─ Reset
              └─────────────────────────┘
```

---

## Component Wiring

### 1. MPU6050 (Accelerometer + Gyroscope)

| MPU6050 | ESP8266 | Wire Color (Suggested) |
|---------|---------|------------------------|
| VCC     | 3.3V    | 🔴 Red                 |
| GND     | GND     | ⚫ Black               |
| SCL     | D1      | 🟡 Yellow              |
| SDA     | D2      | 🟢 Green               |
| INT     | (not connected) | Optional - not used |

### 2. HW-246 (QMC5883L Magnetometer/Compass)

| HW-246 | ESP8266 | Note |
|--------|---------|------|
| VCC    | 3.3V    | Share with MPU6050 |
| GND    | GND     | Share with MPU6050 |
| SCL    | D1      | Same I2C bus |
| SDA    | D2      | Same I2C bus |

### 3. DHT11/DHT22 (Temperature & Humidity)

```
DHT11/22 (front view - grid facing you)
    ┌─────┐
    │ ▓▓▓ │
    │ ▓▓▓ │
    └┬─┬─┬┘
     │ │ │
     1 2 3
     │ │ └── NC (not connected) or GND
     │ └──── DATA → D3 (GPIO0)
     └────── VCC → 3.3V

Add 10K resistor as pull-up between VCC and DATA
```

### 4. HC-SR04 (Ultrasonic Sensor)

| HC-SR04 | ESP8266 | Note |
|---------|---------|------|
| VCC     | VIN (5V)| Needs 5V! |
| GND     | GND     | |
| TRIG    | D5      | |
| ECHO    | D6      | ⚠️ Use voltage divider! |

**⚠️ IMPORTANT**: HC-SR04 ECHO outputs 5V, but ESP8266 GPIO is 3.3V!

```
Voltage Divider for ECHO pin (using your resistors):
                      
HC-SR04 ECHO ──┬── 1K ───┬── 2K2 ──┬── GND
               │         │         
               │      ESP8266 D6   
               │         
        (5V signal → 3.44V, safe for ESP8266)
```

### 5. SG90 Micro Servo (Parachute)

| Servo | ESP8266 | Wire Color |
|-------|---------|------------|
| VCC   | VIN (5V)| 🔴 Red (center) |
| GND   | GND     | 🟤 Brown/Black |
| Signal| D4      | 🟠 Orange/Yellow |

### 6. GPS Module (NEO-6M or similar)

| GPS Module | ESP8266 | Note |
|------------|---------|------|
| VCC        | 3.3V    | Some modules need 5V - check yours! |
| GND        | GND     | |
| TX         | D8      | GPS TX → ESP8266 RX (SoftwareSerial) |
| RX         | (not connected) | Optional, not needed for reading |

```
GPS Module (NEO-6M)
    ┌─────────────────┐
    │    ○ Antenna    │
    │   ┌─────────┐   │
    │   │ NEO-6M  │   │
    │   │  GPS    │   │
    │   └─────────┘   │
    └─┬───┬───┬───┬───┘
      │   │   │   │
     VCC GND  TX  RX
      │   │   │   │
     3.3V GND D8  NC
      (to ESP8266)
```

> ⚠️ **Note**: GPS needs clear sky view to get a fix. Test outdoors!

### 7. Status LED

```
ESP8266 D7 ──── 330E ──── LED (+) ──── LED (-) ──── GND
```

### 8. Resistors Summary (From Your Kit)

| Purpose | Resistor | Color Code |
|---------|----------|------------|
| Voltage Divider R1 | **1K** | Brown-Black-Red |
| Voltage Divider R2 | **2K2** | Red-Red-Red |
| LED Current Limit | **330E** | Orange-Orange-Brown |
| DHT11 Pull-up | **10K** | Brown-Black-Orange |

---

## Power Wiring (USB from PC - Testing Phase)

```
        PC/Laptop USB
             │
             ▼
    ┌─────────────────┐
    │   ESP8266       │
    │   (USB Port)    │
    └────────┬────────┘
             │
    ┌────────┴────────┐
    │                 │
    ▼                 ▼
   5V (VIN)         3.3V (from onboard regulator)
    │                 │
    ├── Servo VCC     ├── MPU6050 VCC
    │                 ├── HW-246 VCC
    └── Ultrasonic    ├── NEO-6M GPS VCC
        VCC           ├── DHT11 VCC
                      └── LED (via 330Ω)

GND ──────────────────────────────────────────
    └── All component GND pins (common ground)
```

> ⚠️ **Note**: USB provides ~500mA. If servo draws too much current, 
> power it separately from a 5V adapter or battery later.

### For Later: Battery Power Upgrade
When ready for standalone operation, add:
- Li-ion battery (3.7V)
- TP4056 charging module
- MT3608 boost converter (for 5V to servo/ultrasonic)

---

## I2C Bus Wiring Diagram

```
                    3.3V
                     │
                    4.7kΩ (pull-up)
                     │
ESP8266 D1 (SCL) ────┼────┬────────┬────────┐
                     │    │        │        │
                     │  MPU6050   HW-246   MPR121
                     │   SCL      SCL       SCL
                     │
                    4.7kΩ (pull-up)
                     │
ESP8266 D2 (SDA) ────┼────┬────────┬────────┐
                          │        │        │
                        MPU6050   HW-246   MPR121
                         SDA      SDA       SDA

All I2C device GND pins connected to common GND
All I2C device VCC pins connected to 3.3V
```

---

## Complete Assembly Checklist

- [ ] Solder header pins to ESP8266 (if needed)
- [ ] Wire I2C bus (SCL/SDA) to MPU6050 + HW-246
- [ ] Connect DHT11/22 with pull-up resistor
- [ ] Wire ultrasonic with voltage divider on ECHO
- [ ] Connect GPS module TX to D8
- [ ] Connect servo to D4
- [ ] Wire status LED with resistor
- [ ] Connect USB cable to PC for power
- [ ] Test each connection with multimeter
- [ ] Upload firmware and verify serial output
- [ ] Test GPS outdoors for satellite fix
