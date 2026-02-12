# 🔧 Program ESP8266 Using Arduino Uno (Step by Step)

Your NodeMCU's USB chip is faulty, but we can use your Arduino Uno
as a USB-to-Serial bridge to upload the firmware!

---

## ⚠️ Important: ESP8266 is 3.3V!

Arduino Uno is **5V**, ESP8266 is **3.3V**. We need a voltage divider
on the TX line so we don't damage the ESP8266.

**You'll need:** 1K resistor + 2K2 resistor (from your kit)

---

## 🔌 STEP 1: Disable the Arduino's Brain

Connect Arduino **RESET → GND** with a jumper wire. This turns off
the ATmega328P chip and makes the Arduino act as a simple USB-to-Serial adapter.

```
    ARDUINO UNO
    ┌──────────────────────┐
    │                      │
    │   RESET ●──┐         │
    │            │ jumper   │
    │     GND ●──┘ wire    │
    │                      │
    └──────────────────────┘
```

---

## 🔌 STEP 2: Wire Arduino to ESP8266

### Connection Table:

| # | Arduino Uno Pin | → | ESP8266 NodeMCU Pin | Notes |
|---|----------------|---|---------------------|-------|
| 1 | **RESET** | → | **GND** (Arduino's own GND) | Disables ATmega |
| 2 | **GND** | → | **GND** | Common ground |
| 3 | **3.3V** | → | **3.3V** | Powers the ESP8266 |
| 4 | **Pin 0 (RX)** | → | **TX** | ESP sends data to PC |
| 5 | **Pin 1 (TX)** | → | **RX** (via voltage divider!) | PC sends data to ESP |

### Wire 5: Voltage Divider (IMPORTANT!)

Arduino TX is **5V** but ESP8266 RX can only handle **3.3V**.
Use your 1K + 2K2 resistors:

```
    Arduino Pin 1 (TX)
         │
       [1K Ω]    ← 1K resistor (Brown-Black-Red)
         │
         ├──────── ESP8266 RX pin
         │
       [2K2 Ω]   ← 2.2K resistor (Red-Red-Red)
         │
        GND

    This converts 5V → ~3.4V (safe for ESP8266!)
```

### How to build on breadboard:

```
    Breadboard Row 1: Arduino TX wire → 1K resistor (one leg)
    Breadboard Row 2: 1K resistor (other leg) + 2K2 resistor (one leg) + wire to ESP8266 RX
    Breadboard Row 3: 2K2 resistor (other leg) → [−] GND rail
```

---

## 🔌 STEP 3: Complete Wiring Diagram

```
    ARDUINO UNO                          ESP8266 NodeMCU V3
    ┌─────────────┐                      ┌─────────────┐
    │             │                      │             │
    │ RESET ●─┐   │                      │             │
    │ GND   ●─┘   │                      │             │
    │             │                      │             │
    │ GND   ●─────┼──── ⚫ ──────────────┼── GND      │
    │             │                      │             │
    │ 3.3V  ●─────┼──── 🔴 ──────────────┼── 3.3V     │
    │             │                      │             │
    │ Pin 0 ●─────┼──── 🟢 ──────────────┼── TX       │
    │ (RX)        │                      │             │
    │             │        ┌─[1KΩ]─┐     │             │
    │ Pin 1 ●─────┼── 🟡 ──┤       ├─────┼── RX       │
    │ (TX)        │        └─[2K2]─┘     │             │
    │             │            │         │             │
    │             │           GND        │             │
    └──────┬──────┘                      └─────────────┘
           │
      [USB-B Cable to PC]
```

> ⚠️ Do NOT plug in the NodeMCU's micro-USB at the same time!
> Only the Arduino's USB-B cable should be connected to your PC.

---

## 🔌 STEP 4: Put ESP8266 in Flash Mode

Before uploading, the ESP8266 needs to be in "flash mode":

1. **Hold the FLASH button** on the NodeMCU
2. **Press and release RST button** (while still holding FLASH)
3. **Release FLASH button** after 1 second

```
    On the NodeMCU board:

    ┌──────────────────────────┐
    │  [RST]          [FLASH]  │
    │    ↑               ↑     │
    │  Step 2          Step 1  │
    │  (tap)          (hold)   │
    └──────────────────────────┘

    Order: Hold FLASH → Tap RST → Release FLASH
```

---

## 🚀 STEP 5: Upload from Arduino IDE

1. Open **Arduino IDE**
2. Open file: `cansat_firmware/cansat_firmware.ino`
3. Go to **Tools → Board → NodeMCU 1.0 (ESP-12E Module)**
4. Go to **Tools → Port → COM?** (the Arduino's COM port)
5. Go to **Tools → Upload Speed → 115200**
6. **Put ESP8266 in flash mode** (Step 4 above)
7. Click **Upload (→ button)**

### If it says "Connecting........_____":
- Repeat the FLASH + RST combo (Step 4)
- Try again

### Success looks like:
```
Writing at 0x00000000... (3%)
Writing at 0x00004000... (6%)
...
Writing at 0x000fc000... (100%)

Done uploading.  ✅
```

---

## ✅ After Upload is Done

1. **Remove the RESET-to-GND jumper** on the Arduino
2. **Disconnect** all wires between Arduino and ESP8266
3. The firmware is now saved on the ESP8266's flash memory!
4. Power the ESP8266 from any USB cable (even charge-only works for power)
5. Connect to WiFi: **CanSat-Telemetry** (password: cansat2024)
6. Open: **http://192.168.4.1**

---

## ❌ Troubleshooting

| Problem | Solution |
|---------|----------|
| "Connecting..." forever | Redo FLASH+RST combo, make sure RESET→GND on Arduino |
| "Invalid head" error | Check TX/RX wires (not swapped?) |
| No COM port | Make sure Arduino's USB cable is plugged in |
| ESP8266 gets hot | STOP! Check 3.3V/GND not swapped |
| Upload works but ESP won't boot | Remove RESET jumper from Arduino first |
