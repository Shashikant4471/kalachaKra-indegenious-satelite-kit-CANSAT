# 🔌 Ultrasonic Terrain Scanner — 5-Sensor Wiring Guide
## Arduino Uno + 5x HC-SR04 Sensors (Center + 4 Corners)

---

## 📋 Layout Configuration

Your arrangement: **1 Center Sensor + 4 Corner Sensors**

```
      (Top-Left)        (Top-Right)
         S1 ────────────── S2
          \                /
           \      S0      /
            \  (Center)  /
           /              \
         S3 ────────────── S4
    (Bottom-Left)     (Bottom-Right)
```

---

## 📌 Arduino Uno Pin Assignment

All sensors share **Pin 2** for Trigger. Each has its own Echo pin.

```
Uno Pin  │ Sensor                 │ Wire Color
─────────┼────────────────────────┼──────────
Pin 2    │ ALL TRIG (shared)      │ 🟤 Brown
Pin 3    │ S0 ECHO (Center)       │ 🟠 Orange
Pin 4    │ S1 ECHO (Top-Left)     │ 🟡 Yellow
Pin 5    │ S2 ECHO (Top-Right)    │ 🟢 Green
Pin 6    │ S3 ECHO (Bot-Left)     │ 🔵 Blue
Pin 7    │ S4 ECHO (Bot-Right)    │ 🟣 Purple
5V       │ ALL VCC                │ 🔴 Red
GND      │ ALL GND                │ ⚫ Black
```

---

## 🛠️ Step-by-Step Wiring

### 1. Power Rails (The Foundation)
Connect Arduino **5V** and **GND** to the breadboard power rails.
- `Uno 5V` → `Red Rail (+)`
- `Uno GND` → `Blue Rail (-)`

### 2. The Shared Trigger (Pin 2)
1. Connect a wire from **Uno Pin 2** to an empty row on the breadboard.
2. From that row, run **5 wires**, one to the **TRIG** pin of *each* sensor.
   *(Alternatively, daisy-chain the TRIG pins if sensors are close, but star-wiring is better for signal)*.

### 3. Sensor S0 (Center)
- **VCC** → `Red Rail (+)`
- **GND** → `Blue Rail (-)`
- **TRIG** → To the shared Pin 2 connnection
- **ECHO** → **Uno Pin 3**

### 4. Sensor S1 (Top-Left)
- **VCC** → `Red Rail (+)`
- **GND** → `Blue Rail (-)`
- **TRIG** → To the shared Pin 2 connnection
- **ECHO** → **Uno Pin 4**

### 5. Sensor S2 (Top-Right)
- **VCC** → `Red Rail (+)`
- **GND** → `Blue Rail (-)`
- **TRIG** → To the shared Pin 2 connnection
- **ECHO** → **Uno Pin 5**

### 6. Sensor S3 (Bottom-Left)
- **VCC** → `Red Rail (+)`
- **GND** → `Blue Rail (-)`
- **TRIG** → To the shared Pin 2 connnection
- **ECHO** → **Uno Pin 6**

### 7. Sensor S4 (Bottom-Right)
- **VCC** → `Red Rail (+)`
- **GND** → `Blue Rail (-)`
- **TRIG** → To the shared Pin 2 connnection
- **ECHO** → **Uno Pin 7**

---

## 🧪 Testing

1. **Upload Code**: Open `terrain_scanner.ino` and upload to your Uno.
2. **Serial Monitor**: Open at **115200 baud**.
3. **Verify**: You should see readings for "Center", "Top-L", "Top-R", etc.
   - If a sensor reads `0` or `---`, check its VCC/GND and Echo wire.
   - If *all* read `0`, check the detailed Trig wire connection at Pin 2.

## 🚀 Visualization
Once verified in Serial Monitor, close it and run the Python viewer:
```bash
python terrain_viewer.py COMx  # Replace COMx with your port (e.g., COM3)
```
