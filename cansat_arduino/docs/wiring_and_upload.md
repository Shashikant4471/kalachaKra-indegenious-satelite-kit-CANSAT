# 🔌 Arduino Uno CanSat - Beginner Wiring Guide (Step by Step)

## What You'll Need

| # | Item | Qty |
|---|------|-----|
| 1 | Arduino Uno | 1 |
| 2 | Breadboard | 1 |
| 3 | MPU6050 (accelerometer) | 1 |
| 4 | HW-246 / QMC5883L (magnetometer) | 1 |
| 5 | DHT11 (temp & humidity) | 1 |
| 6 | NEO-6M GPS | 1 |
| 7 | 10K Resistor (Brown-Black-Orange) | 1 |
| 8 | Jumper wires (M-M and M-F) | ~12 |

> 💡 The Arduino Uno has a **built-in LED** on pin 13 — no extra LED or resistor needed!

---

## 📍 Arduino Uno Pin Map (What We'll Use)

```
                    ARDUINO UNO (Top View)
    ┌─────────────────────────────────────────────┐
    │                                             │
    │  DIGITAL PINS                               │
    │  ┌─────────────────────────────────────────┐│
    │  │ 13 12 11 10  9  8  7  6  5  4  3  2  1 ││
    │  │ LED              DHT     GPS             ││
    │  └─────────────────────────────────────────┘│
    │                                             │
    │                    [USB PORT]                │
    │                                             │
    │  ANALOG/POWER PINS                          │
    │  ┌─────────────────────────────────────────┐│
    │  │ A0 A1 A2 A3 A4  A5     5V 3.3V GND GND ││
    │  │              SDA SCL                     ││
    │  └─────────────────────────────────────────┘│
    └─────────────────────────────────────────────┘

    Pins We Use:
    ─────────────────────────────────
    3.3V     →  Power for all sensors
    GND      →  Ground for all sensors
    A4 (SDA) →  I2C Data (MPU6050 + HW-246)
    A5 (SCL) →  I2C Clock (MPU6050 + HW-246)
    Pin 7    →  DHT11 data
    Pin 4    →  GPS TX
    Pin 13   →  Built-in LED ✨
```

---

## 🟢 STEP 1: Set Up Power Rails

Get **2 jumper wires** (red + black).

### What To Do:
1. Connect Arduino **3.3V** → Breadboard **[+] rail** (red line)
2. Connect Arduino **GND** → Breadboard **[−] rail** (blue line)

```
    ARDUINO UNO                    BREADBOARD
    ┌──────────┐         ┌──────────────────────────┐
    │          │         │  [+] ● ● ● ● ● ● ● ●   │ ← Red line
    │   3.3V ●─┼── 🔴 ──│──►                        │
    │          │         │                           │
    │    GND ●─┼── ⚫ ──│──►                        │
    │          │         │  [−] ● ● ● ● ● ● ● ●   │ ← Blue line
    └──────────┘         └──────────────────────────┘
```

> ⚠️ **Use 3.3V, NOT 5V!** The sensors are 3.3V devices.

### ✅ Checkpoint: You should have 2 wires connected.

---

## 🟢 STEP 2: Wire MPU6050 (Accelerometer)

Get **4 jumper wires**.

### MPU6050 has these pins:
```
    MPU6050
    ┌────────┐
    │ VCC    │ → Power
    │ GND    │ → Ground
    │ SCL    │ → Clock
    │ SDA    │ → Data
    │ XDA    │ → (ignore)
    │ XCL    │ → (ignore)
    │ AD0    │ → (ignore)
    │ INT    │ → (ignore)
    └────────┘
    Only use the first 4 pins!
```

### What To Do:

| # | MPU6050 Pin | Connect To | Wire |
|---|-------------|------------|------|
| 1 | **VCC** | Breadboard [+] rail | 🔴 Red |
| 2 | **GND** | Breadboard [−] rail | ⚫ Black |
| 3 | **SCL** | Arduino **A5** | 🟡 Yellow |
| 4 | **SDA** | Arduino **A4** | 🟢 Green |

```
    MPU6050                         ARDUINO UNO
    ┌────────┐
    │ VCC ●──┼── 🔴 ──► [+] Rail (3.3V)
    │ GND ●──┼── ⚫ ──► [−] Rail
    │ SCL ●──┼── 🟡 ──────────────► A5
    │ SDA ●──┼── 🟢 ──────────────► A4
    └────────┘
```

### ✅ Checkpoint: You should have 6 wires total (2 power + 4 MPU6050).

---

## 🟢 STEP 3: Wire HW-246 / QMC5883L (Magnetometer)

Get **4 more jumper wires**.

### What To Do:

| # | HW-246 Pin | Connect To | Wire |
|---|------------|------------|------|
| 1 | **VCC** | Breadboard [+] rail | 🔴 Red |
| 2 | **GND** | Breadboard [−] rail | ⚫ Black |
| 3 | **SCL** | Arduino **A5** (SAME as MPU6050!) | 🟡 Yellow |
| 4 | **SDA** | Arduino **A4** (SAME as MPU6050!) | 🟢 Green |

```
    HW-246                          ARDUINO UNO
    ┌────────┐
    │ VCC ●──┼── 🔴 ──► [+] Rail (3.3V)
    │ GND ●──┼── ⚫ ──► [−] Rail
    │ SCL ●──┼── 🟡 ──────────────► A5  (shared with MPU6050)
    │ SDA ●──┼── 🟢 ──────────────► A4  (shared with MPU6050)
    └────────┘
```

> 💡 **Both sensors share A4 and A5!** This is called I2C — multiple devices on the same 2 wires. They have different addresses so they don't conflict.

### How to share pins on the breadboard:
```
    Arduino A4 ── 🟢 ── breadboard row 10 ──┬── MPU6050 SDA
                                             └── HW-246 SDA

    Arduino A5 ── 🟡 ── breadboard row 11 ──┬── MPU6050 SCL
                                             └── HW-246 SCL
```

### ✅ Checkpoint: You should have 10 wires total.

---

## 🟢 STEP 4: Wire DHT11 (Temperature & Humidity)

Get **3 jumper wires** + your **10K resistor** (Brown-Black-Orange).

### DHT11 Pins (looking at the front, left to right):
```
    DHT11 (front view)
    ┌──────────┐
    │  ┌────┐  │
    │  │    │  │
    │  │    │  │
    │  └────┘  │
    │ 1  2  3  │
    │VCC DAT GND│
    └──────────┘
      │   │   │
     3.3V │  GND
          │
        Pin 7
```

> ⚠️ Some DHT11 modules have **4 pins** — the 3rd pin is unused (NC). Use pins 1, 2, and 4.

### What To Do:

| # | DHT11 Pin | Connect To | Wire |
|---|-----------|------------|------|
| 1 | **VCC** (pin 1) | Breadboard [+] rail | 🔴 Red |
| 2 | **DATA** (pin 2) | Arduino **Pin 7** | 🔵 Blue |
| 3 | **GND** (pin 3 or 4) | Breadboard [−] rail | ⚫ Black |
| 4 | **10K Resistor** | Between VCC row and DATA row | (component) |

```
    [+] Rail (3.3V) ──────┬──── DHT11 VCC
                          │
                        [10K]  ← Pull-up resistor (Brown-Black-Orange)
                          │
    Arduino Pin 7 ────────┴──── DHT11 DATA

    [−] Rail (GND) ──────────── DHT11 GND
```

> 💡 **Why the resistor?** The DHT11 needs a "pull-up" resistor to send clean signals. Without it, you get wrong readings.

### ✅ Checkpoint: You should have 13 wires + 1 resistor.

---

## 🟢 STEP 5: Wire GPS Module (NEO-6M)

Get **3 jumper wires**. The GPS stays external (not on breadboard).

### What To Do:

| # | GPS Pin | Connect To | Wire |
|---|---------|------------|------|
| 1 | **VCC** | Breadboard [+] rail (3.3V) | 🔴 Red |
| 2 | **GND** | Breadboard [−] rail | ⚫ Black |
| 3 | **TX** | Arduino **Pin 4** | ⚪ White |
| 4 | **RX** | ❌ NOT connected | — |

```
    NEO-6M GPS
    ┌──────────┐
    │ VCC ● ───│── 🔴 ──► [+] Rail (3.3V)
    │ GND ● ───│── ⚫ ──► [−] Rail
    │ TX  ● ───│── ⚪ ──► Arduino Pin 4
    │ RX  ●    │  (leave empty)
    └──────────┘
```

> 💡 GPS antenna should face **UP** and be near a **window** for best reception.

### ✅ Checkpoint: You should have 16 wires + 1 resistor. That's ALL!

---

## 🎉 WIRING COMPLETE!

### Final Connection Summary:

```
Arduino Uno Pin │ Component            │ Step
────────────────┼──────────────────────┼──────
3.3V            │ [+] Rail → Sensors   │ 1 ✅
GND             │ [−] Rail → Sensors   │ 1 ✅
A4 (SDA)        │ MPU6050 + HW-246     │ 2+3 ✅
A5 (SCL)        │ MPU6050 + HW-246     │ 2+3 ✅
Pin 7           │ DHT11 + 10K pullup   │ 4 ✅
Pin 4           │ GPS TX               │ 5 ✅
Pin 13          │ Built-in LED         │ (automatic!) ✅
```

### ✅ Final Checklist Before Powering On:

- [ ] All VCC wires going to [+] rail (3.3V)?
- [ ] All GND wires going to [−] rail?
- [ ] 10K resistor between DHT11 VCC and DATA?
- [ ] MPU6050 and HW-246 share A4/A5 (not separate pins)?
- [ ] GPS TX goes to Pin 4 (not Pin 19)?
- [ ] No bare wires touching each other?
- [ ] Using 3.3V (NOT 5V)?

---

## 🚀 Next: Upload the Firmware!

1. Plug the USB-B cable into Arduino Uno and your PC
2. Open **Arduino IDE**
3. Open the file: `cansat_arduino/cansat_mega/cansat_mega.ino`
4. Go to **Tools → Board → Arduino Uno**
5. Go to **Tools → Port → COM?** (select the one that appears)
6. Click ▶️ **Upload**
7. Open **Serial Monitor** (Ctrl+Shift+M) → Set baud to **115200**
8. Watch the data flow! 📊
