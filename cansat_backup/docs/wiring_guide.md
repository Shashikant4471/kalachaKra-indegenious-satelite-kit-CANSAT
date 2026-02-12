# CanSat Raspberry Pi 3B+ Wiring Guide

## 📦 Components
| # | Component | Qty |
|---|-----------|-----|
| 1 | Raspberry Pi 3 Model B+ | 1 |
| 2 | MPU6050 | 1 |
| 3 | HW-246 (QMC5883L) | 1 |
| 4 | NEO-6M GPS | 1 |
| 5 | DHT11/DHT22 | 1 |
| 6 | LED | 1 |
| 7 | 10K Resistor | 1 |
| 8 | 330E Resistor | 1 |
| 9 | Jumper Wires | Many |

---

## 🔌 Raspberry Pi GPIO Pinout

```
                    Raspberry Pi 3B+ (Top View)
    ┌──────────────────────────────────────────────────┐
    │  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ● │
    │  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ●  ● ● │
    │  1 2  3 4  5 6  7 8  9 10 11 12 ...              │
    └──────────────────────────────────────────────────┘

    Pin#  │ Name     │ Our Usage
    ──────┼──────────┼──────────────────
     1    │ 3.3V     │ Sensor Power
     2    │ 5V       │ (not used)
     3    │ GPIO2    │ I2C SDA ← MPU6050 + HW-246
     4    │ 5V       │ (not used)
     5    │ GPIO3    │ I2C SCL ← MPU6050 + HW-246
     6    │ GND      │ Common Ground
     7    │ GPIO4    │ DHT11 Data
     8    │ GPIO14   │ UART TX (to GPS RX) - optional
     9    │ GND      │ Common Ground
    10    │ GPIO15   │ UART RX ← GPS TX
    11    │ GPIO17   │ LED (via 330E)
    39    │ GND      │ Common Ground
```

---

## 📍 STEP 1: Power

Your Raspberry Pi is powered via its **micro-USB port** from your PC or a 5V adapter.

All sensors connect to **Pin 1 (3.3V)** for power and **Pin 6 or 9 (GND)** for ground.

```
    Pi Pin 1 (3.3V) ────── 🔴 ────── [+] Rail on breadboard
    Pi Pin 6 (GND)  ────── ⚫ ────── [−] Rail on breadboard
```

---

## 📍 STEP 2: I2C Sensors (MPU6050 + HW-246)

Both sensors share the I2C bus (just like the ESP8266 version!).

| Sensor Pin | Connect To | Pi Pin |
|------------|------------|--------|
| MPU6050 VCC | 3.3V | Pin 1 |
| MPU6050 GND | GND | Pin 6 |
| MPU6050 SDA | I2C SDA | Pin 3 (GPIO2) |
| MPU6050 SCL | I2C SCL | Pin 5 (GPIO3) |
| HW-246 VCC | 3.3V | Pin 1 |
| HW-246 GND | GND | Pin 6 |
| HW-246 SDA | I2C SDA | Pin 3 (same as MPU6050) |
| HW-246 SCL | I2C SCL | Pin 5 (same as MPU6050) |

```
Pi Pin 3 (SDA) ──── 🟢 ────┬──── MPU6050 SDA
                            └──── HW-246 SDA

Pi Pin 5 (SCL) ──── 🟡 ────┬──── MPU6050 SCL
                            └──── HW-246 SCL
```

> 💡 The Raspberry Pi has **built-in pull-up resistors** on I2C pins, so no external pull-ups needed!

---

## 📍 STEP 3: DHT11 Sensor

| DHT11 Pin | Connect To | Pi Pin |
|-----------|------------|--------|
| VCC | 3.3V | Pin 1 |
| DATA | GPIO4 | Pin 7 |
| GND | GND | Pin 6 |
| 10K Resistor | Between VCC and DATA | — |

```
    Pi Pin 1 (3.3V) ──────┬──── DHT11 VCC
                          │
                        [10K]  ← Pull-up resistor
                          │
    Pi Pin 7 (GPIO4) ─────┴──── DHT11 DATA

    Pi Pin 6 (GND) ──────────── DHT11 GND
```

---

## 📍 STEP 4: GPS Module (NEO-6M)

| GPS Pin | Connect To | Pi Pin |
|---------|------------|--------|
| VCC | 3.3V | Pin 1 |
| GND | GND | Pin 9 |
| TX | UART RX | Pin 10 (GPIO15) |
| RX | Not connected | — |

```
    Pi Pin 1 (3.3V) ──── 🔴 ──── GPS VCC
    Pi Pin 9 (GND)  ──── ⚫ ──── GPS GND
    Pi Pin 10 (RX)  ──── ⚪ ──── GPS TX
```

> ⚠️ You may need to **enable UART** on the Pi. See setup guide.

---

## 📍 STEP 5: LED

| Connection | Pi Pin |
|------------|--------|
| GPIO17 → 330E → LED(+) → LED(−) → GND | Pin 11, Pin 6 |

```
    Pi Pin 11 (GPIO17) ──── [330E] ──── LED(+) ──── LED(−) ──── GND
```

---

## 📊 Complete Wiring Summary

```
Pi Pin  │ Name    │ Component
────────┼─────────┼────────────────────
 1      │ 3.3V    │ All sensor VCC
 3      │ GPIO2   │ I2C SDA (MPU6050 + HW-246)
 5      │ GPIO3   │ I2C SCL (MPU6050 + HW-246)
 6      │ GND     │ Common Ground
 7      │ GPIO4   │ DHT11 DATA (+ 10K pullup)
10      │ GPIO15  │ GPS TX → Pi RX
11      │ GPIO17  │ LED (via 330E)
```

**Only 5 GPIO pins used!** Much simpler than the ESP8266 version.
